#include <signal.h>
#include <sys/epoll.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class epoll_server;

class processor {
public:
	processor(epoll_server *handle);

	void bind(int fd);
	void run();

private:
	int fd_;
	epoll_server *handle_;

	int handle_client_read();
};

class worker : public acl::thread {
public:
	worker(void) {}
	~worker(void) {}

	void push(processor *job) {
		box_.push(job, false);
	}

protected:
	// @override
	void* run(void) {
		bool found;
		while (true) {
			processor *job = box_.pop(-1, &found);
			if (job) {
				job->run();
			} else if (found) {
				break;
			}
		}
		return NULL;
	}

private:
	acl::fiber_tbox<processor> box_;
};

class epoll_server : public acl::thread {
public:
	epoll_server(int lfd, int nthreads);

	void *run(void);

	void epoll_relisten(int fd);
	void release(processor* job);
	processor* peek(void);

private:
	~epoll_server(void);

	void handle_accept(int lfd);
	void handle_client(int fd);
	void epoll_add_read(int fd, bool oneshot);

private:
	int  lfd_;
	int  epfd_;

	std::vector<worker*> threads_;
	size_t next_;
	std::vector<processor*> jobs_;
	acl::thread_mutex lock_;
};

epoll_server::epoll_server(int lfd, int nthreads)
: lfd_(lfd)
, next_(0)
{
	for (int i = 0; i < nthreads; i++) {
		worker *wk = new worker;
		threads_.push_back(wk);
		wk->start();
	}

	epfd_ = epoll_create(1024);

	// init listen fd
	acl_non_blocking(lfd_, ACL_NON_BLOCKING);
	epoll_add_read(lfd_, true);
}

epoll_server::~epoll_server(void) {
	for (std::vector<worker*>::iterator it = threads_.begin();
		it != threads_.end(); ++it) {
		(*it)->wait(NULL);
		delete *it;
	}
	close(epfd_);
}

void* epoll_server::run(void) {
#define	MAX 128

	struct epoll_event events[MAX];

	while (true) {
		int nfds = epoll_wait(epfd_, events, MAX, -1);

		if (nfds == -1) {
			printf("wait error %s\r\n", acl::last_serror());
			exit (1);
		} else if (nfds == 0) {
			continue;
		}

		for (int i = 0; i < nfds; ++i) {
			if (events[i].data.fd == lfd_) {
				handle_accept(lfd_);
			} else {
				handle_client(events[i].data.fd);
			}
		}
	}
	return NULL;
}

void epoll_server::handle_accept(int lfd) {
	int fd = accept(lfd, NULL, NULL);

	if (fd >= 0) {
		acl_non_blocking(fd, ACL_NON_BLOCKING);
		epoll_add_read(fd, true);
	}

	epoll_relisten(lfd);
}

processor *epoll_server::peek(void) {
	acl::thread_mutex_guard guard(lock_);
	processor* job;

	if (jobs_.empty()) {
		job = new processor(this);
		return job;
	}

	job = jobs_.back();
	jobs_.pop_back();
	return job;
}

void epoll_server::release(processor *job) {
	acl::thread_mutex_guard guard(lock_);
	jobs_.push_back(job);
}

void epoll_server::handle_client(int fd) {
	processor *job = peek();
	job->bind(fd);

	size_t i = next_++ % threads_.size();
	threads_[i]->push(job);
}

void epoll_server::epoll_add_read(int fd, bool oneshot) {
	struct epoll_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	if (oneshot) {
		ev.events |= EPOLLONESHOT | EPOLLET;
	}
	ev.data.fd = fd;

	if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
		printf("epoll_ctl error: %s", acl::last_serror());
		exit (1);
	}
}

void epoll_server::epoll_relisten(int fd) {
	struct epoll_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLONESHOT;
	ev.data.fd = fd;

	if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
		printf("epoll_ctl error: %s", acl::last_serror());
		exit (1);
	}
}

processor::processor(epoll_server *handle)
: fd_(-1), handle_(handle)
{}

void processor::bind(int fd) {
	fd_ = fd;
}

void processor::run() {
	assert(fd_ >= 0);

	while (true) {
		int ret = handle_client_read();
		if (ret == 0) {
			handle_->epoll_relisten(fd_);
			break;
		} else if (ret == -1) {
			close(fd_);
			break;
		}
	}

	handle_->release(this);
}

int processor::handle_client_read() {
	char buf[4096];
	int ret;

	if ((ret = read(fd_, buf, sizeof(buf) - 1)) == 0) {
		return -1;
	} else if (ret < 0) {
#if EAGAIN == EWOULDBLOCK
		if (errno == EAGAIN) {
#else
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
			return 0;
		}
		printf("thread-%lu gets error %s\r\n",
			acl::thread::self(), acl::last_serror());
		return -1;
	} else if (write(fd_, buf, ret) != ret) {
		// xxx: fixed me
		printf("thread-%lu write error %s\r\n",
			acl::thread::self(), acl::last_serror());
		return -1;
	} else if (ret == sizeof(buf) - 1) {
		buf[ret] = 0;
		return 1;
	} else {
		buf[ret] = 0;
		return 0;
	}
}

static void usage(const char* procname) {
	printf("usage: %s -h [help] -s server_addr -c nthreads -w workers\r\n", procname);
}

int main(int argc, char* argv[]) {
	acl::string addr("127.0.0.1|8887");
	int  ch, nthreads = 1, workers = 4;

	signal(SIGPIPE, SIG_IGN);
	while ((ch = getopt(argc, argv, "hc:s:w:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			addr = optarg;
			break;
		case 'c':
			nthreads = atoi(optarg);
			break;
		case 'w':
			workers = atoi(optarg);
			break;
		default:
			break;
		}
	}

	acl::server_socket ss(127, false);
	if (ss.open(addr) == false) {
		printf("open %s error %s\r\n", addr.c_str(), acl::last_serror());
		return 1;
	}
	printf("listen %s ok\r\n", addr.c_str());

	std::vector<acl::thread*> threads;

	for (int i = 0; i < nthreads; i++) {
		acl::thread* thr = new epoll_server(ss.sock_handle(), workers);
		thr->set_detachable(false);
		threads.push_back(thr);
		thr->start();
	}

	for (std::vector<acl::thread*>::iterator it = threads.begin();
		it != threads.end(); ++it) {

		(*it)->wait();
		delete *it;
	}

	return 0;
}
