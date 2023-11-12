#include <stdio.h>
#include <stdlib.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

int main() {
    const char* addr = "127.0.0.1:8088";
    acl::server_socket server;
    if (!server.open(addr)) {
        printf("Listen on %s error(%s)\r\n", addr, acl::last_serror());
        return 1;
    }

    printf("Listen on %s ok\r\n", addr);

    go[&server] { // 创建协程用来接收客户端连接
        while (true) {
            // 使用 std::shared_ptr 存放 acl::socket_stream 对象
            acl::shared_stream conn = server.shared_accept();
            if (conn == nullptr) {
                printf("Accept error(%s)\r\n", acl::last_serror());
                break;
            }
            go[conn] {  // 创建协程用来处理客户端数据读写
                char buf[8192];
                while (true) {
                    int ret = conn->read(buf, sizeof(buf), false);
                    if (ret == -1) {
                        printf("Client disconnected\r\n");
                        break;
                    }
                    if (conn->write(buf, ret) == -1) {
                        printf("Write to client error(%s)\r\n", acl::last_serror());
                        break;
                    }
                }
            };
        }
    };

    acl::fiber::schedule();
    return 0;
}
