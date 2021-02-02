#include <signal.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

#define	EPOLL_ET	(1 << 0)
#define	EPOLL_ONESHOT	(1 << 1)

class fd_ctx {
public:
	fd_ctx(void) : ss_(NULL), conn_(NULL), pos_(-1) {}
	~fd_ctx(void) {}

	acl::server_socket* get_ss(void) {
		return ss_;
	}

	acl::socket_stream* get_conn(void) {
		return conn_;
	}

	void bind(acl::server_socket* ss, ssize_t pos) {
		ss_ = ss;
		conn_ = NULL;
		pos_ = pos;
	}

	void bind(acl::socket_stream* conn, ssize_t pos) {
		conn_ = conn;
		ss_ = NULL;
		pos_ = pos;
	}

	void set_pos(ssize_t pos) {
		pos_ = pos;
	}

	ssize_t get_pos(void) const {
		return pos_;
	}

	void unbind(void) {
		ss_ = NULL;
		conn_ = NULL;
		pos_ = -1;
	}

private:
	acl::server_socket* ss_;
	acl::socket_stream* conn_;
	ssize_t pos_;
};

#define	MAX_FD	100000

class poll_thread : public acl::thread {
public:
	poll_thread(acl::server_socket& ss)
	: ss_(ss)
	{
		fds_count_ = 0;
		fds_      = new pollfd[MAX_FD];
		fds_map_  = new fd_ctx*[MAX_FD];
		for (size_t i = 0; i < MAX_FD; i++) {
			fds_map_[i] = new fd_ctx;
		}

		poll_add_read(ss.sock_handle(), &ss);
	}

private:
	~poll_thread(void) {
		delete [] fds_;

		for (size_t i = 0; i < MAX_FD; i++) {
			delete fds_map_[i];
		}
		delete [] fds_map_;
	}

	// @override
	void* run(void) {
		printf("thread-%lu started\r\n", acl::thread::self());
		int timeout = 100;
		acl::server_socket* ss;
		acl::socket_stream* conn;

		while (true) {
			int nfds = poll(fds_, (nfds_t) fds_count_, timeout);
			if (nfds == -1) {
				printf("wait error %s\r\n", acl::last_serror());
				break;
			} else if (nfds == 0) {
				continue;
			}

#define	POLL_EVENT (POLLIN | POLLERR | POLLHUP)
			for (int i = 0; i < fds_count_; i++) {
				if ((fds_[i].revents & POLL_EVENT) == 0) {
					continue;
				}

				int fd = fds_[i].fd;
				conn = fds_map_[fd]->get_conn();
				if (conn) {
					handle_client(conn);
				} else if ((ss = fds_map_[fd]->get_ss())) {
					handle_accept(*ss);
				} else {
					printf("invalid context\r\n");
					abort();
				}

				fds_[i].revents = 0;
			}
		}

		return NULL;
	}

	void handle_accept(acl::server_socket& ss) {
		acl::socket_stream* conn = ss.accept();
		if (conn == NULL) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				return;
			}
			printf("accept error %s\r\n", acl::last_serror());
			exit (1);
		} else if (conn->sock_handle() >= MAX_FD) {
			delete conn;
		} else {
			conn->set_tcp_non_blocking(true);
			poll_add_read(conn->sock_handle(), conn);
		}
	}

	void handle_client(acl::socket_stream* conn) {
		while (true) {
			int ret = handle_client_read(conn);
			if (ret == 0) {
				break;
			} else if (ret == -1) {
				poll_del_read(conn->sock_handle());
				delete conn;
				break;
			}
		}
	}

	int handle_client_read(acl::socket_stream* conn) {
		int fd = conn->sock_handle(), ret;
		char buf[4096];
		if ((ret = read(fd, buf, sizeof(buf) - 1)) == 0) {
			return -1;
		} else if (ret < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return 0;
			}
			printf("thread-%lu gets error %s, conn=%p\r\n",
				acl::thread::self(), acl::last_serror(), conn);
			return -1;
		} else if (write(fd, buf, ret) != ret) {
			printf("thread-%lu write error %s\r\n",
				acl::thread::self(), acl::last_serror());
			return -1;
		} else if (ret == sizeof(buf) - 1) {
			return 1;
		} else {
			return 0;
		}
	}

private:
	void poll_add_read(int fd, acl::socket_stream* conn) {
		fds_[fds_count_].fd = fd;
		fds_[fds_count_].events = POLLIN | POLLERR | POLLHUP;
		fds_[fds_count_].revents = 0;
		fds_map_[fd]->bind(conn, (ssize_t) fds_count_);
		fds_count_++;
	}

	void poll_add_read(int fd, acl::server_socket* ss) {
		fds_[fds_count_].fd = fd;
		fds_[fds_count_].events = POLLIN | POLLERR | POLLHUP;
		fds_[fds_count_].revents = 0;
		fds_map_[fd]->bind(ss, (ssize_t) fds_count_);
		fds_count_++;
	}

	void poll_del_read(int fd) {
		ssize_t pos = fds_map_[fd]->get_pos();

		assert(fds_count_ > 0 && pos >= 0 && pos < fds_count_);

		fds_map_[fd]->unbind();

		if (pos == --fds_count_) {
			return;
		}

		fd = fds_[fds_count_].fd;
		ssize_t pos_tail = fds_map_[fd]->get_pos();

		assert(pos_tail >= 0);

		fds_map_[fd]->set_pos(pos);
		fds_[pos] = fds_[pos_tail];
	}

private:
	struct pollfd* fds_;
	fd_ctx** fds_map_;
	ssize_t fds_count_;

	acl::server_socket& ss_;
};

static void usage(const char* procname) {
	printf("usage: %s -h [help] -s server_addr -c nthreads\r\n", procname);
}

int main(int argc, char* argv[]) {
	acl::string addr("127.0.0.1:8887");
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
		acl::thread* thr = new poll_thread(ss);
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
