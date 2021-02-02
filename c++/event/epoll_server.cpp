/**
 * Copyright (C) 2015-2018
 * All rights reserved.
 *
 * AUTHOR(S)
 *   Zheng Shuxin
 *   E-mail: shuxin.zheng@qq.com
 * 
 * VERSION
 *   Sun 03 Dec 2017 20:49:54 AM CST
 */

#include <signal.h>
#include <sys/epoll.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

#define	EPOLL_ET	(1 << 0)
#define	EPOLL_ONESHOT	(1 << 1)

static unsigned __epoll_flag = /* EPOLL_ET |*/ EPOLL_ONESHOT;

static void epoll_add_read(int epfd, int fd, void *ptr) {
	struct epoll_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	if (__epoll_flag & EPOLL_ET) {
		ev.events |= EPOLLET;
	}
	if (__epoll_flag & EPOLL_ONESHOT) {
		ev.events |= EPOLL_ONESHOT;
	}
	ev.data.ptr = ptr;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		printf("add error %s\r\n", acl::last_serror());
		exit (1);
	}
	printf("add read=%d, epfd=%d\r\n", fd, epfd);
}

static void epoll_read_again(int epfd, int fd, void *ptr) {
	struct epoll_event ev;

	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLONESHOT;
	if (__epoll_flag & EPOLL_ET) {
		ev.events |= EPOLLET;
	}

	ev.data.ptr = ptr;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
		printf("add error %s\r\n", acl::last_serror());
		exit (1);
	}
}

class epoll_thread : public acl::thread {
public:
	epoll_thread(int epfd, acl::server_socket& ss) : epfd_(epfd), ss_(ss) {}

private:
	~epoll_thread(void) {}

	// @override
	void* run(void) {
		printf("thread-%lu started\r\n", acl::thread::self());

#define MAX	1024
		struct epoll_event events[MAX];

		while (true) {
			int nfds = epoll_wait(epfd_, events, MAX, -1);
			if (nfds == -1) {
				printf("wait error %s\r\n", acl::last_serror());
				continue;
			}

			for (int i = 0; i < nfds; i++) {
				if (events[i].data.ptr == &ss_) {
					handle_accept(ss_);
				} else {
					handle_client(events[i].data.ptr);
				}
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
		} else {
			printf("thread-%lu: accept one fd %d\r\n",
				acl::thread::self(), conn->sock_handle());
			conn->set_tcp_non_blocking(true);
			epoll_add_read(epfd_, conn->sock_handle(), conn);
			if (__epoll_flag & EPOLL_ONESHOT) {
				epoll_read_again(epfd_, ss.sock_handle(), &ss);
			}
		}
	}

	void handle_client(void *ctx) {
		acl::socket_stream* conn = (acl::socket_stream *) ctx;
		while (true) {
			int ret = handle_client_read(conn);
			if (ret == 0) {
				break;
			} else if (ret == -1) {
				printf(">>>delete fd: %d\n", conn->sock_handle());
				delete conn;
				break;
			}
		}
	}

	int handle_client_read(acl::socket_stream* conn) {
		int fd = conn->sock_handle(), ret;
		char buf[4096];
		if ((ret = read(fd, buf, sizeof(buf) - 1)) == 0) {
			printf(">>>read over fd: %d\n", fd);
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
			if (__epoll_flag & EPOLL_ONESHOT) {
				epoll_read_again(epfd_, conn->sock_handle(), conn);
			}
			return 0;
		}
	}

	void handle_client1(void *ctx) {
		acl::socket_stream* conn = (acl::socket_stream *) ctx;
		acl::string buf;
		if (conn->read(buf, false) == false) {
			printf("thread-%lu gets error %s, conn=%p\r\n",
				acl::thread::self(), acl::last_serror(), conn);
			delete conn;
		} else if (conn->write(buf) == -1) {
			printf("thread-%lu write error %s\r\n",
				acl::thread::self(), acl::last_serror());
			delete conn;
		} else {
			if (__epoll_flag & EPOLL_ONESHOT) {
				epoll_read_again(epfd_, conn->sock_handle(), conn);
			}
		}
	}

private:
	int epfd_;
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

	int epfd = epoll_create(1024);
	assert(epfd >= 0);

	epoll_add_read(epfd, ss.sock_handle(), &ss);

	std::vector<acl::thread*> threads;

	for (int i = 0; i < nthreads; i++) {
		acl::thread* thr = new epoll_thread(epfd, ss);
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
