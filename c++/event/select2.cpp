#include <signal.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class select_server;

class processor : public acl::thread_job {
public:
	processor(int fd, select_server *handle);

	void *run();

	int handle_client_read();

private:
	int fd_;
	select_server *handle_;

};

class select_server : public acl::thread {
public:
	select_server(int lfd);

	void select_del_read(int fd);

	void select_relisten(int fd);

	void* run(void);

private:
	~select_server(void);

	void handle_accept(int lfd);

	void handle_client(int fd);

	void select_add_read(int fd);

private:
	int lfd_;
	int fd_max_;
	int fds_count_;

	int *fds_listened_;
	int *fds_listened_pos_;

	fd_set read_fdset_;
	acl::thread_mutex mutex_;
	acl::thread_pool threads_;

};

select_server::select_server(int lfd)
: lfd_(lfd)
, fd_max_(lfd)
, fds_count_(0)
{
	threads_.set_idle(120);
	threads_.set_limit(100);
	threads_.set_stacksize(256000);

	// init fdset
	FD_ZERO(&read_fdset_);

	// init fd map
	fds_listened_ = new int[FD_SETSIZE];
	fds_listened_pos_ = new int[FD_SETSIZE];
	for (int i = 0; i < FD_SETSIZE; i++) {
		fds_listened_[i] = -1;
		fds_listened_pos_[i] = -1;
	}

	// init listen fd
	acl_non_blocking(lfd_, ACL_NON_BLOCKING);
	select_add_read(lfd_);
}

select_server::~select_server(void) {
	delete []fds_listened_;
	delete []fds_listened_pos_;
}

void* select_server::run(void) {
	threads_.start();

	struct timeval timeout;

	while (true) {
		timeout.tv_sec = 0;
		timeout.tv_usec = 10;
		fd_set local_read_fdset = read_fdset_;

		int nfds = select(fd_max_ + 1, &local_read_fdset,
			NULL, NULL, &timeout);

		if (nfds == -1) {
			printf("wait error %s\r\n", acl::last_serror());
			exit (1);
		} else if (nfds == 0) {
			continue;
		}

		for (int i = 0; i < fds_count_; ++i) {
			if (!FD_ISSET(fds_listened_[i], &local_read_fdset)) {
				continue;
			}
			int fd = fds_listened_[i];
			if (fd == lfd_) {
				handle_accept(fd);
			} else {
				handle_client(fd);
			}
		}
	}
	return NULL;
}

void select_server::handle_accept(int lfd) {
	int fd = accept(lfd, NULL, NULL);
	if (fd >= FD_SETSIZE) {
		printf("too large cfd=%d\r\n", fd);
		close(fd);
	} else if (fd >= 0) {
		acl_non_blocking(fd, ACL_NON_BLOCKING);
		select_add_read(fd);
	}
}

void select_server::handle_client(int fd) {
	select_del_read(fd);
	processor *job = new processor(fd, this);
	threads_.execute(job);
}

void select_server::select_del_read(int fd) {
	// lock to delete
	acl::thread_mutex_guard guard(mutex_);

	// clear fdset
	FD_CLR(fd, &read_fdset_);

	// clear fd map
	int pos = fds_listened_pos_[fd];

	assert(pos >= 0);

	fds_listened_pos_[fd] = -1;

	if (pos == --fds_count_) {
		fds_listened_[pos] = -1;
	} else {
		fds_listened_[pos] = fds_listened_[fds_count_];
		fds_listened_pos_[fds_listened_[pos]] = pos;
	}

	// reset fd_max_
	fd_max_ = lfd_;
	for (int i = 0; i < fds_count_; ++i) {
		fd_max_ = std::max(fd_max_, fds_listened_[i]);
	}
}

void select_server::select_add_read(int fd) {
	acl::thread_mutex_guard guard(mutex_);

	fds_listened_[fds_count_] = fd;
	fds_listened_pos_[fd] = fds_count_;
	fds_count_++;
	FD_SET(fd, &read_fdset_);
	fd_max_ = std::max(fd_max_, fd);
}

void select_server::select_relisten(int fd) {
	select_add_read(fd);
}

processor::processor(int fd, select_server *handle)
: fd_(fd), handle_(handle)
{}

void* processor::run() {
	if (fd_ < 0) {
		delete this;
		return 0;
	}
	while (true) {
		int ret = handle_client_read();
		if (ret == 0) {
			handle_->select_relisten(fd_);
			break;
		} else if (ret == -1) {
			close(fd_);
			break;
		}
	}
	delete this;
	return NULL;
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
	printf("usage: %s -h [help] -s server_addr -c nthreads\r\n", procname);
}

int main(int argc, char* argv[]) {
	acl::string addr("127.0.0.1|8887");
	int  ch, nthreads = 1;

	signal(SIGPIPE, SIG_IGN);
	while ((ch = getopt(argc, argv, "hc:s:")) > 0) {
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
		acl::thread* thr = new select_server(ss.sock_handle());
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
