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
    // Register IO read event callback to handle the read process.
    (*client).on_read([&fibers, client](nio::socket_t fd, bool expired) {
        if (expired) {
            printf("Read timeout from fd %d\r\n", fd);
            client->close_await();
            return;
        }

        // First: disable the IO read callback.
        client->read_disable();

        // Push the read process in fiber_pool, which will be handled by one fiber.
        fibers.exec([] (nio::client_socket* cli) {
            // Read from the client socket in one fiber.
            char buf[4096];
            auto ret = cli->read(buf, sizeof(buf));

            // Echo back to the client.
            if (ret <= 0 ||cli->write(buf, ret) != ret) {
                // Cloe the client socket if some error happens.
                cli->close_await();
            } else {
                // Enable the IO read process again.
                cli->read_await(5000);
            }
        }, client);
    }).on_error([client](nio::socket_t) {
        client->close_await();  // Close the client socket.
    }).on_close([client](nio::socket_t fd) {
        printf("Closing client fd %d\r\n", fd);

        // Free the client object when connection disconnecting.
        delete client;
    });

    // Enable the IO read process with 5000 ms timeout limit for the first time.
    client->read_await(5000);
}

static void server_run(acl::fiber_pool& fibers, nio::nio_event& ev,
        const char* ip, int port) {
    // Create one server socket and bind it with the specified address.
    nio::server_socket server;
    if (!server.open(ip, port)) {
        printf("Listen error %s, addr: %s:%d\r\n", strerror(errno), ip, port);
        return;
    }

    printf("Listen %s:%d ok\r\n", ip, port);

    // Register the event callback to handle an established connection.
    server.set_on_accept([&fibers, &ev] (nio::socket_t fd, const std::string& addr) {
        printf("Accept one client from %s, fd: %d\r\n", addr.c_str(), fd);

        // Create one socket client object and handle it.
        auto* client = new nio::client_socket(ev, fd);
        handle_client(fibers, client);
    }).set_on_error([]() { // Some error happen in the server socket.
        printf("Accept error %s\r\n", strerror(errno));
    }).set_on_close([]() { // When server socket closing.
        printf("server socket closed\r\n");
    });

    // Enable the server socket in async status waiting for connections.
    server.accept_await(ev);

    // Event loop process.
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

    // Create the fiber pool to execute any task.
    std::shared_ptr<acl::fiber_pool> fibers
        (new acl::fiber_pool(10, 100, -1, 500, 32000, false));

    // Create one backend fiber to show the fiber pool's running status.
    go[fibers] {
        while (true) {
            acl::fiber::delay(1000);
            printf("box_min: %zd, box_max: %zd, box_count: %zd, box_idle: %zd\r\n",
                    fibers->get_box_min(), fibers->get_box_max(),
                    fibers->get_box_count(), fibers->get_box_idle());
        }
    };

    // Create asynchronous nio fiber. 
    go[fibers, etype, flags, ip, port] {
        // Create asynchronous event handle.
        nio::nio_event ev(102400, etype, flags);

        // Start the IO event loop process in the current fiber.
        server_run(*fibers, ev, ip, port);
    };

    // Enable all fibers sharing the same epoll handle in fiber internal.
    acl::fiber::share_epoll(true);

    // Start the fiber schedule process.
    acl::fiber::schedule();
    return 0;
}
