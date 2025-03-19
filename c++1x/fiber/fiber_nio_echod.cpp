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

static void handle_client(acl::fiber_pool& fibers, nio::client_socket* client) {
    (*client).on_read([&fibers, client](nio::socket_t fd, bool expired) {
        if (expired) {
            printf("Read timeout from fd %d\r\n", fd);
            client->close_await();
            return;
        }

        client->read_disable();

        fibers.exec([] (nio::client_socket* cli) {
            char buf[4096];
            auto ret = cli->read(buf, sizeof(buf));
            if (ret <= 0 ||cli->write(buf, ret) != ret) {
                cli->close_await();
            } else {
                cli->read_await();
            }
        }, client);
    }).on_error([client](nio::socket_t) {
        client->close_await();
    }).on_close([client](nio::socket_t fd) {
        printf("Closing client fd %d\r\n", fd);

        delete client;
    });

    client->read_await(5000);
}

static void server_run(acl::fiber_pool& fibers, nio::nio_event& ev,
        const char* ip, int port) {
    nio::server_socket server(ev);
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

    server.accept_await();

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
