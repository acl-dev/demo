#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <getopt.h>

#include <string>
#include <memory>

#include <nio/client_socket.hpp>

#include "nio_server.h"

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
        .set_on_accept([](nio::client_socket&, const std::string&) -> bool {
            return true;
        }).set_on_read([] (nio::client_socket& client) -> bool {
            char buf[4096];
            int ret = client.read(buf, sizeof(buf));
            if (ret <= 0) {
                return false;
            }
            if (client.write(buf, ret) != ret) {
                return false;
            }
            return true;
        }).set_on_timeout([] (nio::client_socket&) -> bool {
            return false;
        }).set_on_close([](nio::client_socket&) {
        });

    server.start();
    return 0;
}
