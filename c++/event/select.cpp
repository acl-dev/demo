#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

#define	MAX_FD	100000

class select_thread : public acl::thread {
public:
	select_thread(int lfd)
	: lfd_(lfd)
	{
		fd_max_    = lfd_;
		fds_count_ = 0;
		fds_       = new int[MAX_FD];
		fds_map_   = new int[MAX_FD];
		FD_ZERO(&rset_);

		for (ssize_t i = 0; i < MAX_FD; i++) {
			fds_[i] = -1;
			fds_map_[i] = -1;
		}

		acl_non_blocking(lfd_, ACL_NON_BLOCKING);
		select_add_read(lfd_);
	}

private:
	~select_thread(void) {
		delete [] fds_;
		delete [] fds_map_;
	}

	// @override
	void* run(void) {
		while (true) {
			fd_set rset = rset_;
			struct timeval tv;
			tv.tv_sec  = 1;
			tv.tv_usec = 0;

			int nfds = select(fd_max_ + 1, &rset, NULL, NULL, &tv);
			if (nfds == -1) {
				printf("wait error %s\r\n", acl::last_serror());
				exit (1);
			} else if (nfds == 0) {
				continue;
			}

			for (int i = 0; i < fds_count_; i++) {
				if (!FD_ISSET(fds_[i], &rset_)) {
					continue;
				}
				int fd = fds_[i];
				if (fd == lfd_) {
					handle_accept(fd);
				} else {
					handle_client(fd);
				}
			}
		}

		return NULL;
	}

	void handle_accept(int lfd) {
		while (true) {
			int cfd = accept(lfd, NULL, NULL);
			if (cfd >= MAX_FD) {
				printf("too large cfd=%d\r\n", cfd);
				close(cfd);
			} else if (cfd >= 0) {
				acl_non_blocking(cfd, ACL_NON_BLOCKING);
				select_add_read(cfd);
#if EAGAIN == EWOULDBLOCK
			} else if (errno == EAGAIN) {
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
				break;
			} else {
				printf("accept error: %s\r\n", acl::last_serror());
				break;
			}
		}
	}

	void handle_client(int cfd) {
		while (true) {
			int ret = handle_client_read(cfd);
			if (ret == 0) {
				break;
			} else if (ret == -1) {
				select_del_read(cfd);
				close(cfd);
				break;
			}
		}
	}

	int handle_client_read(int fd) {
		char buf[4096];
		ssize_t ret;

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
		} else if (write(fd, buf, (size_t) ret) != ret) {
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
	void select_add_read(int fd) {
		fds_[fds_count_] = fd;
		FD_SET(fd, &rset_);
		fds_map_[fd] = fds_count_;
		fds_count_++;
		if (fd > fd_max_) {
			fd_max_ = fd;
		}
	}

	void select_del_read(int fd) {
		int pos = fds_map_[fd];
		FD_CLR(fd, &rset_);
		if (pos == --fds_count_) {
			fds_map_[fd] = -1;
		} else {
			fds_[pos] = fds_[fds_count_];
			fds_map_[fds_[pos]] = pos;
		}
		if (fd == fd_max_) {
			--fd_max_;
		}
	}

private:
	fd_set rset_;
	int* fds_;
	int* fds_map_;
	int  fd_max_;
	int  fds_count_;
	int  lfd_;
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

	acl::server_socket ss(128, false);
	if (ss.open(addr) == false) {
		printf("open %s error %s\r\n", addr.c_str(), acl::last_serror());
		return 1;
	}
	printf("listen %s ok\r\n", addr.c_str());

	std::vector<acl::thread*> threads;

	for (int i = 0; i < nthreads; i++) {
		acl::thread* thr = new select_thread(ss.sock_handle());
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
