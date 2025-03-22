#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <getopt.h>

#include <string>
#include <memory>

#include <nio/client_socket.hpp>
#include <acl-lib/acl_cpp/lib_acl.hpp>

#include "nio_server.h"

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

static void usage(const char* proc) {
    printf("usage: %s -h [help]\r\n"
        " -e nio_event type[kernel|poll|select, default: kernel]\r\n", proc);
}

int main(int argc, char *argv[]) {
    nio::nio_event_t etype = nio::NIO_EVENT_T_KERNEL;
    int ch;

    while ((ch = getopt(argc, argv, "he:")) > 0) {
        switch (ch) {
        case 'h':
            usage(argv[0]);
            return 0;
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

    nio_server server(ip, port);
    server.set_nio_event(etype)
        .set_on_accept([](nio::client_socket& client, const std::string&) -> bool {
            auto* conn = new acl::socket_stream;
            conn->open(client.sock_handle());
            auto* serv = new http_servlet(conn);
            auto* ctx = new std::tuple<acl::socket_stream*, http_servlet*>(conn, serv);
            client.set_ctx(ctx);
            return true;
        }).set_on_read([] (nio::client_socket& client) -> bool {
            auto* ctx = (std::tuple<acl::socket_stream*, http_servlet*>*) client.get_ctx();
            auto* serv = std::get<1>(*ctx);
            return serv->doRun();
        }).set_on_timeout([] (nio::client_socket&) -> bool {
            return false;
        }).set_on_close([](nio::client_socket& client) {
            auto* ctx = (std::tuple<acl::socket_stream*, http_servlet*>*) client.get_ctx();
            auto* conn = std::get<0>(*ctx);
            auto* serv = std::get<1>(*ctx);
            conn->unbind_sock();
            delete conn;
            delete serv;
            delete ctx;
        });

    server.start();
    return 0;
}
