#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <sys/epoll.h>
#include <string>
#include <memory>

#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

static bool __use_http = false;

class http_servlet : public acl::HttpServlet {
public:
    http_servlet(acl::socket_stream* conn) : HttpServlet(conn, (acl::session*) nullptr) {}
    ~http_servlet() override = default;

    // @override
    bool doGet(acl::HttpServletRequest& req, acl::HttpServletResponse& res) override {
        const char data[] = "hello world!\r\n";
        res.setContentLength(sizeof(data) - 1);
        res.setKeepAlive(req.isKeepAlive());

        return res.write(data, sizeof(data) - 1) && req.isKeepAlive();
    }
};

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

using task_fn = std::function<void(void)>;

static void handle_server(fiber_pool<task_fn>&, int epfd, int lfd) {
    int fd = accept(lfd, nullptr, nullptr);
    if (fd < 0) {
        printf("accept error %s\r\n", acl::last_serror());
        exit (1);
    }

    //printf("accept one connection, fd=%d\r\n", fd);
    set_rw_timeout(fd, 60);
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

#define MAX_CLIENT  10240

static http_servlet *__clients[MAX_CLIENT];

static void handle_client(fiber_pool<task_fn>& fibers, int epfd, int fd) {
    event_del_read(epfd, fd);

    fibers.exec([epfd, fd] {
        if (__use_http) {
            http_servlet* hs = __clients[fd];
            if (hs == nullptr) {
                auto* conn = new acl::socket_stream();
                conn->open(fd);
                hs = new http_servlet(conn);
                __clients[fd] = hs;
            }

            if (hs->doRun()) {
                event_add_read(epfd, fd);
            } else {
                __clients[fd] = nullptr;
                auto* conn = hs->getStream();
                assert(conn);
                delete conn;
            }
        } else {
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
        }
    });
}

static void usage(const char *procname) {
    printf("usage: %s -h [help]\r\n"
            " -s address[default: 127.0.0.1:8288]\r\n"
            " -L min[default: 10]\r\n"
            " -M max[default: 100]\r\n"
            " -b buf[default: 500]\r\n"
            " -H [if use http, default: false]\r\n"
            " -t fiber idle timeout in seconds[default: 10]\r\n", procname);
}

int main(int argc, char *argv[]) {
    int ch, buf = 500, timeout = 10000;
    size_t max = 20, min = 10;
    std::string addr("127.0.0.1:8288");

    while ((ch = getopt(argc, argv, "hs:L:M:Hb:t:")) > 0) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                return 0;
            case 's':
                addr = optarg;
                break;
            case 'b':
                buf = atoi(optarg);
                break;
            case 'L':
                min = (size_t) atoi(optarg);
                break;
            case 'M':
                max = (size_t) atoi(optarg);
                break;
            case 'H':
                __use_http = true;
                break;
            case 't':
                timeout = atoi(optarg) * 1000;
                break;
            default:
                usage(argv[0]);
                return 1;
        }
    }

    for (size_t i = 0; i < MAX_CLIENT; i++) {
        __clients[i] = nullptr;
    }

    acl::server_socket ss(1024, false);
    if (!ss.open(addr.c_str())) {
        printf("Listen %s error %s\r\n", addr.c_str(), acl::last_serror());
        return 1;
    }

    printf("Listen %s ok\r\n", addr.c_str());

    std::shared_ptr<acl::fiber_pool> fibers
        (new acl::fiber_pool(min, max, timeout, buf, 64000));

    go[fibers] {
        while (true) {
            acl::fiber::delay(1000);
            printf("box_min: %zd, box_max: %zd, box_count: %zd, box_idle: %zd\r\n",
                    fibers->get_box_min(), fibers->get_box_max(),
                    fibers->get_box_count(), fibers->get_box_idle());
        }
    };

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
