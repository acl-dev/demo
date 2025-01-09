#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <sys/epoll.h>
#include <string>
#include <memory>

#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.h>
#include <acl-lib/fiber/libfiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

#include "../../../c++1x/fiber/fiber_pool2.h"

static void event_listen(int epfd, int lfd) {
    struct epoll_event ev;

    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLONESHOT;
    ev.data.fd = lfd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev) == -1) {
        printf("epoll_ctl error: %s", acl::last_serror());
        exit (1);
    }
}

static void event_add_read(int epfd, int fd) {
	struct epoll_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLONESHOT | EPOLLET;
	ev.data.fd = fd;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		printf("epoll_ctl error: %s", acl::last_serror());
		exit (1);
	}
}

static void event_del_read(int epfd, int fd) {
	struct epoll_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	ev.data.fd = fd;

	if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev) == -1) {
		printf("epoll_ctl error: %s", acl::last_serror());
		exit (1);
	}
}

static void set_rw_timeout(int fd, int timeout) {
    struct timeval tm;
    tm.tv_sec  = timeout;
    tm.tv_usec = 0;

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tm, sizeof(tm)) < 0) {
        printf("setsockopt for read error=%s, timeout=%d, fd=%d\r\n",
            acl::last_serror(), timeout, (int) fd);
        exit (1);
    } else if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tm, sizeof(tm)) < 0) {
        printf("setsockopt for send error=%s, timeout=%d, fd=%d\r\n",
            acl::last_serror(), timeout, (int) fd);
        exit (1);
    }
}

static void handle_server(fiber_pool2&, int epfd, int lfd) {
    int fd = accept(lfd, nullptr, nullptr);
    if (fd < 0) {
        printf("accept error %s\r\n", acl::last_serror());
        exit (1);
    }

    printf("accept one connection, fd=%d\r\n", fd);
    set_rw_timeout(fd, 5);
    event_add_read(epfd, fd);
    event_listen(epfd, lfd);
}

static bool write_loop(int fd, const char* buf, size_t len) {
    const char* ptr = buf;
    while (len > 0) {
        int ret = write(fd, ptr, len);
        if (ret <= 0) {
            printf("close fd=%d for writing: %s\r\n", fd, acl::last_serror());
            return false;
        }
        len -= ret;
        ptr += ret;
    }
    return true;
}

static void handle_client(fiber_pool2& fibers, int epfd, int fd) {
    event_del_read(epfd, fd);

    fibers.exec([epfd, fd] {
        char buf[4096];
        int ret = read(fd, buf, sizeof(buf) - 1);
        if (ret <= 0) {
            printf("close fd=%d for reading: %s\r\n", fd, acl::last_serror());
            close(fd);
        } else if (!write_loop(fd, buf, (size_t) ret)) {
            close(fd);
        } else {
            event_add_read(epfd, fd);
        }
    });
}

static void usage(const char *procname) {
    printf("usage: %s -h [help]\r\n"
            " -s address\r\n"
            " -c nfiber\r\n"
            " -b buf\r\n"
            " -t timeout\r\n", procname);
}

int main(int argc, char *argv[]) {
    int ch, nfiber = 100, buf = 500, timeout = -1;
    std::string addr("127.0.0.1:8288");

    while ((ch = getopt(argc, argv, "hs:c:b:t:")) > 0) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                return 0;
            case 's':
                addr = optarg;
                break;
            case 'c':
                nfiber = atoi(optarg);
                break;
            case 'b':
                buf = atoi(optarg);
                break;
            case 't':
                timeout = atoi(optarg);
                break;
            default:
                usage(argv[0]);
                return 1;
        }
    }

    acl::server_socket ss(1024, false);
    if (!ss.open(addr.c_str())) {
        printf("Listen %s error %s\r\n", addr.c_str(), acl::last_serror());
        return 1;
    }

    printf("Listen %s ok\r\n", addr.c_str());

    std::shared_ptr<fiber_pool2> fibers
        (new fiber_pool2(buf, nfiber, timeout, 0, false));

    acl::wait_group wg;

    go[fibers, &wg] {
        printf("Wait for fiber pool finish...\r\n");
        wg.wait();
        fibers->stop();
    };

#define MAX 256

    wg.add(1);
    go[fibers, &wg, &ss] {
        int epfd = epoll_create(1024), lfd = ss.sock_handle();
        struct epoll_event events[MAX];

        event_listen(epfd, lfd);

        while (true) {
            int nfds = epoll_wait(epfd, events, MAX, -1);
            if (nfds == -1) {
                printf("epoll_wait error %s\r\n", acl::last_serror());
                break;
            }

            if (nfds == 0) {
                continue;
            }

            for (int i = 0; i < nfds; i++) {
                if (events[i].data.fd == lfd) {
                    handle_server(*fibers, epfd, lfd);
                } else {
                    handle_client(*fibers, epfd, events[i].data.fd);
                }
            }
        }

        wg.done();
    };

    acl::fiber::share_epoll(true);
    acl::fiber::schedule();
    return 0;
}
