#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <signal.h>

#define	MAX_FD	100000

class poll_thread : public acl::thread {
public:
	poll_thread(int lfd)
	: lfd_(lfd)
	{
		fds_count_ = 0;
		fds_       = new pollfd[MAX_FD];
		fds_map_   = new ssize_t[MAX_FD];
		for (ssize_t i = 0; i < MAX_FD; i++) {
			fds_map_[i] = -1;
		}

		acl_non_blocking(lfd_, ACL_NON_BLOCKING);
		poll_add_read(lfd_);
	}

private:
	~poll_thread(void) {
		delete [] fds_;
		delete fds_map_;
	}

	// @override
	void* run(void) {
		int timeout = 100;

		while (true) {
			int nfds = poll(fds_, (nfds_t) fds_count_, timeout);
			if (nfds == -1) {
				printf("wait error %s\r\n", acl::last_serror());
				exit (1);
			} else if (nfds == 0) {
				continue;
			}

#define	POLL_EVENT (POLLIN | POLLERR | POLLHUP)
			for (int i = 0; i < fds_count_; i++) {
				if ((fds_[i].revents & POLL_EVENT) == 0) {
					continue;
				}
				int fd = fds_[i].fd;
				if (fd == lfd_) {
					handle_accept(fd);
				} else {
					handle_client(fd);
				}
				fds_[i].revents = 0;
			}
		}

		return NULL;
	}

	void handle_accept(int lfd) {
		int cfd = accept(lfd, NULL, NULL);
		if (cfd >= MAX_FD) {
			printf("too large cfd=%d\r\n", cfd);
			close(cfd);
		} else if (cfd >= 0) {
			acl_non_blocking(cfd, ACL_NON_BLOCKING);
			poll_add_read(cfd);
		}
	}

	void handle_client(int cfd) {
		while (true) {
			int ret = handle_client_read(cfd);
			if (ret == 0) {
				break;
			} else if (ret == -1) {
				poll_del_read(cfd);
				close(cfd);
				break;
			}
		}
	}

	int handle_client_read(int fd) {
		char buf[4096];
		int ret;

		if ((ret = read(fd, buf, sizeof(buf) - 1)) == 0) {
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
		} else if (write(fd, buf, ret) != ret) {
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

private:
	void poll_add_read(int fd) {
		fds_[fds_count_].fd = fd;
		fds_[fds_count_].events = POLLIN | POLLERR | POLLHUP;
		fds_[fds_count_].revents = 0;
		fds_map_[fd] = fds_count_;
		fds_count_++;
	}

	void poll_del_read(int fd) {
		ssize_t pos = fds_map_[fd];
		assert(pos >= 0);

		if (pos == -- fds_count_) {
			fds_map_[fd] = -1;
		} else {
			fds_[pos] = fds_[fds_count_];
			fds_map_[fds_[pos].fd] = pos;
		}
	}

private:
	struct pollfd* fds_;
	ssize_t* fds_map_;
	ssize_t fds_count_;
	int lfd_;
};

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
		acl::thread* thr = new poll_thread(ss.sock_handle());
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
