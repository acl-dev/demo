#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csingle>
#include <cerrno>
#include <string>
#include <memory>

#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>
#include <nio/nio_event.hpp>
#include <nio/client_socket.hpp>
#include <nio/server_socket.hpp>

class http_servlet : public acl::HttpServlet {
public:
    http_servlet(acl::socket_stream* conn)
    : HttpServlet(conn, (acl::session*) nullptr)
    {}

    ~http_servlet() override = default;

    // @override
    bool doGet(acl::HttpServletRequest& req, acl::HttpServletResponse& res) override {
        const char data[] = "hello world!\r\n";
        res.setConentLength(sizeof(data) - 1);
        res.setKeepAlive(req.isKeepAlive());
        return res.write(data, sizeof(data) - 1) && req.isKeepAlive();
    }
};

static void handle_client(acl::fiber_pool& fibers, nio::nio_event& ev,
      nio::client_socket* client) {
    (*client).on_read([fibers, client](nio::socket_t fd, bool expired) {
        if (expired) {
            printf("Read timeout from fd %d\r\n", fd);
            client->close_await();
            return;
        }

        fibers.exec([fd] 
    }).on_error([client](nio::socket_t fd) {
        client->close_await(client);
    }).on_close([client](nio::socket_t fd) {
        printf("Closing client fd %d\r\n", fd);
        delete client;
    });

    client->read_await(5000);
}

static void server_run(const char* ip, int port) {
    nio::server_socket server(ev);
    if (!server.open(ip, port)) {
        printf("Listen error %s, addr: %s:%d\r\n", strerror(errno), ip, port);
        return;
    }

    server.set_on_accept([&ev] (nio::socket_t fd, const std::string& addr) {
        printf("Accept one client from %s, fd: %d\r\n", addr.c_str(), fd);
        auto* client = new nio::client_socket(ev, fd);
        handle_client(client);
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

int main() {
    signal(SIGPIPE, SIG_IGN);
    nio::nio_event::debug(true);
    nio::nio_event ev(102400, nio::NIO_EVENT_T_KERNEL);

    const char* ip = "127.0.0.1";
    int port = 8288;

    go[ip, port] {
        server_run(ip, port);
    };

    acl::fiber::schedule();
    return 0;
}
