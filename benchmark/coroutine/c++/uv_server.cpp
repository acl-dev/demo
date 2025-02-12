#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <uv.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

#define DEFAULT_PORT 7000
#define DEFAULT_BACKLOG 128

static uv_loop_t *loop;
static struct sockaddr_in addr;
static int nconns = 0;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

// 释放写请求资源
void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

// 分配接收缓冲区
static void alloc_buffer(uv_handle_t *, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

static int getfd(uv_stream_t *client) {
    uv_os_fd_t fd;
    int ret = uv_fileno((uv_handle_t*) client, &fd);
    if (ret == 0) {
        return (int) fd;
    } else {
        return -1;
    }
}

// 写操作完成回调
static void echo_write(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}

static void on_client_close(uv_handle_t* peer) {
      free(peer);
      --nconns;
      printf(">>>connection count: %d<<<\n", nconns);
}

// 读取数据回调
static void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        // 创建写请求
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init((char*) malloc(nread), nread);
        memcpy(req->buf.base, buf->base, nread);
        
        // 将接收到的数据写回客户端
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
    }
    // 处理错误或关闭连接
    else if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error %s, fd=%d\r\n",
                uv_err_name(nread), getfd(client));
        } else {
            fprintf(stdout, "Read over, fd=%d\r\n", getfd(client));
        }
        uv_close((uv_handle_t*) client, on_client_close);
    } else if (nread == 0) {
        printf("Read 0\r\n");
    }
    
    free(buf->base);
}

// 新连接回调
static void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }
    
    // 创建客户端对象
    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    
    int ret;
    // 接受连接
    if ((ret = uv_accept(server, (uv_stream_t*) client)) == 0) {
        // 开始读取数据
        printf("Accept one fd: %d\r\n", getfd((uv_stream_t*) (client)));
        ret = uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
        if (ret != 0) {
            printf("uv_read_start error: %s\r\n", uv_strerror(ret));
            uv_close((uv_handle_t*) client, on_client_close);
        } else {
            ++nconns;
        }
    } else {
        fprintf(stderr, "Accept error %s\r\n", uv_err_name(ret));
        uv_close((uv_handle_t*) client, on_client_close);
    }
}

// 关闭服务端回调
static void on_server_close(uv_handle_t* ) {
    printf("Server closed\n");
}

static void run() {
    loop = uv_default_loop();

    // 初始化TCP服务端
    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    // 绑定地址
    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);
    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    
    // 开始监听
    int r = uv_listen((uv_stream_t*) &server, DEFAULT_BACKLOG, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return;
    }
    
    printf("Echo server listening on port %d\n", DEFAULT_PORT);
    
    // 运行事件循环
#if 0
    uv_run(loop, UV_RUN_DEFAULT);
#else
    while (true) {
        uv_run(loop, UV_RUN_ONCE);
        //uv_run(loop, UV_RUN_NOWAIT);
    }
#endif
    
    // 关闭服务端
    uv_close((uv_handle_t*) &server, on_server_close);
    uv_loop_close(loop);
}

static void usage(const char *proc) {
    printf("usage: %s -h [help]\r\n"
        " -F [if running in fiber mode, default: false]\r\n"
        " -e fiber_event[kernel|poll|select, default: kernel]\r\n"
        , proc);
}

int main(int argc, char *argv[]) {
    int ch;
    bool fiber_mode = false;
    acl::fiber_event_t fiber_event = acl::FIBER_EVENT_T_KERNEL;

    while ((ch = getopt(argc, argv, "hFe:")) > 0) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                return 0;
            case 'F':
                fiber_mode = true;
                break;
            case 'e':
                if (strcmp(optarg, "poll") == 0) {
                    fiber_event = acl::FIBER_EVENT_T_POLL;
                } else if (strcmp(optarg, "select") == 0) {
                    fiber_event = acl::FIBER_EVENT_T_SELECT;
                }
                break;
            default:
                usage(argv[0]);
                return 1;
        }
    }
    
    if (fiber_mode) {
        printf("Running in fiber mode...\r\n");
        go[] {
            run();
        };
        acl::fiber::stdout_open(true);
        acl::fiber::schedule(fiber_event);
    } else {
        printf("Running in no fiber mode...\r\n");
        run();
    }

    return 0;
}
