#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <getopt.h>

#include <string>
#include <memory>

#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>
#include <nio/nio_event.hpp>
#include <nio/client_socket.hpp>
#include <nio/server_socket.hpp>

static bool use_http = false;

class http_servlet : public acl::HttpServlet {
public:
    http_servlet(acl::socket_stream* conn)
    : HttpServlet(conn, (acl::session*) nullptr)
    {}

    ~http_servlet() override = default;

    // @override
    bool doGet(acl::HttpServletRequest& req, acl::HttpServletResponse& res) override {
        const char data[] = "hello world!\r\n";
        res.setContentLength(sizeof(data) - 1);
        res.setKeepAlive(req.isKeepAlive());
        return res.write(data, sizeof(data) - 1) && req.isKeepAlive();
    }
};

static void http_serv(nio::client_socket* client, http_servlet* serv) {
    if (serv->doRun()) {
        client->read_await();
    } else {
        client->close_await();
    }
}

static void echo_serv(nio::client_socket* client) {
    char buf[4096];
    auto ret = client->read(buf, sizeof(buf));
    if (ret <= 0 ||client->write(buf, ret) != ret) {
        client->close_await();
    } else {
        client->read_await();
    }
}

static void handle_client(acl::fiber_pool& fibers, nio::client_socket* client) {
    acl::socket_stream* conn = new acl::socket_stream;
    conn->open(client->sock_handle());
    http_servlet* serv = new http_servlet(conn);

    (*client).on_read([&fibers, client, serv](nio::socket_t fd, bool expired) {
        if (expired) {
            printf("Read timeout from fd %d\r\n", fd);
            client->close_await();
            return;
        }

        client->read_disable();

        if (use_http) {
            fibers.exec(http_serv, client, serv);
        } else {
            fibers.exec(echo_serv, client);
        }
    }).on_error([client](nio::socket_t) {
        client->close_await();
    }).on_close([client, conn, serv](nio::socket_t fd) {
        printf("Closing client fd %d\r\n", fd);
        conn->unbind_sock();
        delete conn;
        delete serv;

        delete client;
        return true;
    });

    client->read_await(5000);
}

static void server_run(acl::fiber_pool& fibers, nio::nio_event& ev,
        const char* ip, int port) {
    nio::server_socket server(1024);
    if (!server.open(ip, port)) {
        printf("Listen error %s, addr: %s:%d\r\n", strerror(errno), ip, port);
        return;
    }

    printf("Listen %s:%d ok\r\n", ip, port);

    server.set_on_accept([&fibers, &ev] (nio::socket_t fd, const std::string& addr) {
        printf("Accept one client from %s, fd: %d\r\n", addr.c_str(), fd);
        auto* client = new nio::client_socket(ev, fd);
        handle_client(fibers, client);
    }).set_on_error([]() {
        printf("Accept error %s\r\n", strerror(errno));
    }).set_on_close([]() {
        printf("server socket closed\r\n");
    });

    server.accept_await(ev);

    while (true) {
        ev.wait(1000);
    }
}

static void usage(const char* proc) {
    printf("usage: %s -h [help]\r\n"
        " -H [if running in http server mode]\r\n"
        " -C [if nio event using caching mode, default: NIO_EVENT_F_DIRECT\r\n"
        " -e nio_event type[kernel|poll|select, default: kernel]\r\n"
        , proc);
}

int main(int argc, char *argv[]) {
    unsigned flags = nio::NIO_EVENT_F_DIRECT;
    nio::nio_event_t etype = nio::NIO_EVENT_T_KERNEL;
    int ch;

    while ((ch = getopt(argc, argv, "hHCe:")) > 0) {
        switch (ch) {
        case 'h':
            usage(argv[0]);
            return 0;
        case 'H':
            use_http = true;
            break;
        case 'C':
            flags &= ~nio::NIO_EVENT_F_DIRECT;
            break;
        case 'e':
            if (strcmp(optarg, "poll") == 0) {
                etype = nio::NIO_EVENT_T_POLL;
            } else if (strcmp(optarg, "select") == 0) {
                etype = nio::NIO_EVENT_T_SELECT;
            }
            break;
        default:
            usage(argv[0]);
            return 1;
        }
    }

    signal(SIGPIPE, SIG_IGN);
    nio::nio_event::debug(true);

    const char* ip = "127.0.0.1";
    int port = 8288;

    std::shared_ptr<acl::fiber_pool> fibers
        (new acl::fiber_pool(10, 100, -1, 500, 32000, false));

    go[fibers] {
        while (true) {
            acl::fiber::delay(1000);
            printf("box_min: %zd, box_max: %zd, box_count: %zd, box_idle: %zd\r\n",
                    fibers->get_box_min(), fibers->get_box_max(),
                    fibers->get_box_count(), fibers->get_box_idle());
        }
    };

    go[fibers, etype, flags, ip, port] {
        nio::nio_event ev(102400, etype, flags);
        server_run(*fibers, ev, ip, port);
    };

    acl::fiber::share_epoll(true);
    acl::fiber::schedule();
    return 0;
}
