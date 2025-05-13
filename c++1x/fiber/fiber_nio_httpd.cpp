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

// Http handle process.
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

static void handle_client(acl::fiber_pool& fibers, nio::client_socket* client) {
    auto* conn = new acl::socket_stream;
    conn->open(client->sock_handle());
    auto* serv = new http_servlet(conn);

    (*client).on_read([&fibers, client, serv](nio::socket_t fd, bool expired) {
        if (expired) {
            printf("Read timeout from fd %d\r\n", fd);
            client->close_await();
            return;
        }

        // First disable the read await event from nio event.
        client->read_disable();

        // Exec one http handling process in the fiber pool.
        fibers.exec([client, serv] {
            if (serv->doRun()) {
                // Enable the read await event again in nio event.
                client->read_await(5000);
            } else {
                // Close the connection if don't keep alive, and on_close()
                // will be called.
                client->close_await();
            }
        });
    }).on_error([client](nio::socket_t) {
        client->close_await();
    }).on_close([client, conn, serv](nio::socket_t fd) {
        printf("Closing client fd %d\r\n", fd);

        // Detach socket fd from conn to avoid fd being closed in conn.
        conn->unbind_sock();
        delete conn;
        delete serv;

        delete client;
        return true;
    });

    // Enable read await event for the first time.
    client->read_await(5000);
}

static void server_run(acl::fiber_pool& fibers, nio::nio_event& ev,
        const char* ip, int port) {
    // Create one server socket and bind the specified address.
    nio::server_socket server;
    if (!server.open(ip, port)) {
        printf("Listen error %s, addr: %s:%d\r\n", strerror(errno), ip, port);
        return;
    }

    printf("Listen %s:%d ok\r\n", ip, port);

    // Register accept callback.
    server.set_on_accept([&fibers, &ev] (nio::socket_t fd, const std::string& addr) {
        printf("Accept one client from %s, fd: %d\r\n", addr.c_str(), fd);
        auto* client = new nio::client_socket(ev, fd);
        handle_client(fibers, client);
    }).set_on_error([]() {
        printf("Accept error %s\r\n", strerror(errno));
    }).set_on_close([]() {
        printf("server socket closed\r\n");
    });

    // Set accept await event.
    server.accept_await(ev);

    // The IO event loop.
    while (true) {
        ev.wait(1000);
    }
}

static void usage(const char* proc) {
    printf("usage: %s -h [help]\r\n"
        " -C [if nio event using caching mode, default: NIO_EVENT_F_DIRECT\r\n"
        " -e nio_event type[kernel|poll|select, default: kernel]\r\n"
        , proc);
}

int main(int argc, char *argv[]) {
    unsigned flags = nio::NIO_EVENT_F_DIRECT;
    nio::nio_event_t etype = nio::NIO_EVENT_T_KERNEL;
    int ch;

    while ((ch = getopt(argc, argv, "hCe:")) > 0) {
        switch (ch) {
        case 'h':
            usage(argv[0]);
            return 0;
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
