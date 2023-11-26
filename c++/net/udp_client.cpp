#include <stdio.h>
#include <stdlib.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

int main() {
    const char* laddr = "127.0.0.1|0";
    const char* saddr = "127.0.0.1|8088";
    acl::socket_stream ss;
    if (!ss.bind_udp(laddr, 10)) {
        printf("Bind %s error %s\r\n", laddr, acl::last_serror());
        return 1;
    }

    const char* s = "hello world!\r\n";
    char buf[500];

    for (int i = 0; i < 100; i++) {
        int ret = ss.sendto(s, strlen(s), saddr);
        if (ret == -1) {
            printf("Sendto %s error %s\r\n", saddr, acl::last_serror());
            break;
        }

        ret = ss.read(buf, sizeof(buf), false);
        if (ret == -1) {
            printf("Read error %s\r\n", acl::last_serror());
            break;
        }

        printf("Read %d bytes from %s ok\r\n", ret, ss.get_peer(true));
    }

    return 0;
}
