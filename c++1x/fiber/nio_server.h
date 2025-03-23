#pragma once
#include <functional>
#include <string>
#include <nio/nio_event.hpp>

namespace acl {
    class fiber_pool;
}

namespace nio {
    class server_socket;
};

using server_on_accept = std::function<bool(nio::client_socket &client,
        const std::string &addr)>;
using client_on_read = std::function<bool(nio::client_socket &client)>;
using client_on_timeout = std::function<bool(nio::client_socket &client)>;
using client_on_close = std::function<void(nio::client_socket &client)>;

class nio_server {
public:
    nio_server(const char* ip, int port);
    ~nio_server();

    void start();

    nio_server& set_on_accept(server_on_accept callback);
    nio_server& set_on_read(client_on_read callback);
    nio_server& set_on_timeout(client_on_timeout callback);
    nio_server& set_on_close(client_on_close callback);

    nio_server& set_nio_event(nio::nio_event_t etype);

private:
    server_on_accept on_accept_;
    client_on_read on_read_;
    client_on_timeout on_timeout_;
    client_on_close on_close_;

    nio::nio_event_t etype_ = nio::NIO_EVENT_T_KERNEL;

private:
    acl::fiber_pool *fibers_;
    std::string ip_;
    int port_;

    void run_await(nio::server_socket *server);
    void handle_client(nio::client_socket *client);
};
