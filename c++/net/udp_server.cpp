#include <stdio.h>
#include <stdlib.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

int main() {
    const char* addr = "127.0.0.1|8088";
    acl::socket_stream ss;
    if (!ss.bind_udp(addr, -1, acl::OPEN_FLAG_REUSEPORT)) {
        printf("Bind %s error %s\r\n", addr, acl::last_serror());
        return 1;
    }

    printf("Bind %s ok!\r\n", addr);

    char buf[1500];
    while (true) {
        int ret = ss.read(buf, sizeof(buf), false);
        if (ret == -1) {
            printf("Read error %s\r\n", acl::last_serror());
            break;
        }

        const char* peer = ss.get_peer(true);
        if (peer) {
            printf("Read from %s %d bytes!\r\n", peer, ret);
        }

        if (ss.write(buf, ret) == -1) {
            printf("Write to %s error %s\r\n",
                    peer ? peer : "unknown", acl::last_serror());
            break;
        }
    }

    return 0;
}
