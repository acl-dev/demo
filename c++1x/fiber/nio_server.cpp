#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <errno.h>

#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

#include <nio/client_socket.hpp>
#include <nio/server_socket.hpp>

#include "nio_server.h"

nio_server::nio_server(const char* ip, int port)
: ip_(ip), port_(port)
{
    fibers_ = new acl::fiber_pool(100, 1000, -1, 500, 64000, false);
}

nio_server::~nio_server() {
    delete fibers_;
}

nio_server& nio_server::set_on_accept(server_on_accept callback) {
    on_accept_ = std::move(callback);
    return *this;
}

nio_server& nio_server::set_on_read(client_on_read callback) {
    on_read_ = std::move(callback);
    return *this;
}

nio_server& nio_server::set_on_timeout(client_on_timeout callback) {
    on_timeout_ = std::move(callback);
    return *this;
}

nio_server& nio_server::set_on_close(client_on_close callback) {
    on_close_ = std::move(callback);
    return *this;
}

nio_server& nio_server::set_nio_event(nio::nio_event_t etype) {
    etype_ = etype;
    return *this;
}

void nio_server::start() {
    go[] {
        while (true) { acl::fiber::delay(1000); }
    };
    go[this] {
        run_await();
    };

    acl::fiber::share_epoll(true);
    acl::fiber::schedule();
}

void nio_server::run_await() {
    nio::nio_event ev(102400, etype_, nio::NIO_EVENT_F_DIRECT);
    nio::server_socket server(ev);

    if (!server.open(ip_.c_str(), port_)) {
        printf("Open %s:%d error %s\r\n", ip_.c_str(), port_, strerror(errno));
        return;
    }

    server.set_on_accept([this, &ev] (nio::socket_t fd, const std::string& addr) {
        auto* client = new nio::client_socket(ev, fd);
        if (on_accept_ && !on_accept_(*client, addr)) {
            delete client;
            return;
        }
        handle_client(client);
    }).set_on_error([]() {
    }).set_on_close([]() {
    });

    server.accept_await();

    while (true) {
        ev.wait(1000);
    }
}

void nio_server::handle_client(nio::client_socket* client) {
    (*client).on_read([this, client](nio::socket_t, bool expired) {
        if (expired) {
            if (on_timeout_ && on_timeout_(*client)) {
                client->read_await(5000);
            } else {
                client->close_await();
            }
            return;
        }

        client->read_disable();

        fibers_->exec([this, client] {
            if (on_read_ && on_read_(*client)) {
                client->read_await(5000);
            } else {
                client->close_await();
            }
        });
    }).on_error([client](nio::socket_t) {
        client->close_await();
    }).on_close([this, client](nio::socket_t) {
        if (on_close_) {
            on_close_(*client);
        }
        delete client;
    });

    client->read_await(5000);
}
