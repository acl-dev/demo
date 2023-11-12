#include <stdio.h>
#include <stdlib.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class fiber_client : public acl::fiber {
public:
    fiber_client(acl::socket_stream* conn) : conn_(conn) {}

private:
    acl::socket_stream* conn_;
    ~fiber_client() { delete conn_; }

protected:
    // @override
    void run() {
        char buf[8192];

        while (true) {
            int ret = conn_->read(buf, sizeof(buf), false);
            if (ret == -1) {
                printf("Client disconnected!\r\n");
                break;
            }
            if (conn_->write(buf, (size_t) ret) == -1) {
                printf("Write to client error(%s)\r\n", acl::last_serror());
                break;
            }
        }
        delete this;
    }
};

class fiber_server : public acl::fiber {
public:
    fiber_server(acl::server_socket& server) : server_(server) {}

private:
    acl::server_socket& server_;
    ~fiber_server() {}

protected:
    // @override
    void run() {
        while (true) {
            acl::socket_stream* conn = server_.accept();
            if (conn == NULL) {
                printf("Accept error(%s)\r\n", acl::last_serror());
                break;
            }

            acl::fiber* fb = new fiber_client(conn);
            fb->start();
        }
        delete this;
    }
};

int main() {
    const char* addr = "127.0.0.1:8088";
    acl::server_socket server;
    if (!server.open(addr)) {  // 绑定并监听服务端口
        printf("Listen %s error(%s)\r\n", addr, acl::last_serror());
        return 1;
    }

    printf("Listen on %s ok\r\n", addr);

    // 创建并启动一个服务器协程
    acl::fiber* fb = new fiber_server(server);
    fb->start();

    // 启动协程调度器
    acl::fiber::schedule();
    return 0;
}
