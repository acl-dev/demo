#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include <net_event/net_event.h>
#include <net_event/net_iostuff.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

static socket_t listen_addr(const char *ip, int port) {
	struct sockaddr_in sa;

	memset(&sa, 0, sizeof(sa));
	sa.sin_family      = PF_INET;
	sa.sin_port        = htons(port);
	sa.sin_addr.s_addr = inet_addr(ip);

	int lfd = socket(PF_INET, SOCK_STREAM, 0);
	int on = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if (bind(lfd, (const struct sockaddr*) &sa, sizeof(sa)) < 0) {
		close(lfd);
		return -1;
	}
	if (listen(lfd, 8192) < 0) {
		close(lfd);
		return -1;
	}
	return lfd;
}

static void read_callback(NET_EVENT *ev, NET_FILE *fe) {
	char buf[1024];
	int ret = read(fe->fd, buf, sizeof(buf));
	if (ret <= 0) {
		net_event_close(ev, fe);
		close(fe->fd);
		net_file_free(fe);
	} else if (write(fe->fd, buf, ret) <= 0) {
		net_event_close(ev, fe);
		close(fe->fd);
		net_file_free(fe);
	}
}

static void listen_callback(NET_EVENT *ev, NET_FILE *fe) {
	struct sockaddr_in sa;
	socklen_t len = (socklen_t) sizeof(sa);
	memset(&sa, 0, sizeof(sa));
	socket_t cfd = accept(fe->fd, (struct sockaddr*) &sa, &len);
	if (cfd == -1) {
		printf("accept error %s\r\n", strerror(errno));
	} else {
		printf("accept one fd %d\r\n", cfd);
		net_non_blocking(cfd, 1);
		net_tcp_nodelay(cfd, 1);
		fe = net_file_alloc(cfd);
		net_event_add_read(ev, fe, read_callback);
	}
}

static void run(int lfd, int event_type, int event_max, bool fiber_mode) {
	NET_EVENT *ev = net_event_create(event_max, event_type);
	assert(ev);

    if (fiber_mode) {
        go[ev, lfd] {
            while (true) {
                socket_t fd = accept(lfd, nullptr, nullptr);
                if (fd == -1) {
                    printf("accept error %s\r\n", strerror(errno));
                    exit (1);
                }

                printf("accept one fd %d\r\n", fd);
                net_non_blocking(fd, 1);
                net_tcp_nodelay(fd, 1);
                NET_FILE *fe = net_file_alloc(fd);
                net_event_add_read(ev, fe, read_callback);
            }
        };
    } else {
        NET_FILE *fe = net_file_alloc(lfd);
        net_event_add_read(ev, fe, listen_callback);
    }

	while (1) {
		net_event_wait(ev, 1000);
	}
}

static void usage(const char *procname) {
	printf("usage: %s -s listen_ip\r\n"
		" -p listen_port\r\n"
		" -t event_type[kernel|poll|select]\r\n"
        " -m event_maxsize[default: 1024]\r\n"
        " -F [if using fiber mode, default: false]\r\n"
        " -f fiber_event[kernel|poll|select, default: kernel]\r\n"
		, procname);
}

int main(int argc, char *argv[]) {
	int ch, port = 8088, event_type = NET_EVENT_TYPE_KERNEL, event_max = 1024;
	char addr[64];
    bool fiber_mode = false;
    acl::fiber_event_t fiber_event = acl::FIBER_EVENT_T_KERNEL;

	signal(SIGPIPE, SIG_IGN);

	snprintf(addr, sizeof(addr), "127.0.0.1");

	while ((ch = getopt(argc, argv, "hs:p:t:m:Ff:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			snprintf(addr, sizeof(addr), "%s", optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 't':
            if (strcasecmp(optarg, "poll") == 0) {
                event_type = NET_EVENT_TYPE_POLL;
            } else if (strcasecmp(optarg, "select") == 0) {
                event_type = NET_EVENT_TYPE_SELECT;
            }
			break;
        case 'm':
            event_max = atoi(optarg);
            break;
        case 'F':
            fiber_mode = true;
            break;
        case 'f':
            if (strcmp(optarg, "poll") == 0) {
                fiber_event = acl::FIBER_EVENT_T_POLL;
            } else if (strcmp(optarg, "select") == 0) {
                fiber_event = acl::FIBER_EVENT_T_SELECT;
            }
            break;
		default:
			break;
		}
	}

	socket_t lfd = listen_addr(addr, port);
	if (lfd == -1) {
		printf("listen %s:%d error %s\r\n", addr, port, strerror(errno));
		return 1;
	}

	printf("NET_FILE size is %zd\r\n", sizeof(NET_FILE));

	printf("listen on %s:%d\r\n", addr, port);

    if (fiber_mode) {
        printf("Run event_server in fiber mode...\r\n");
        go[lfd, event_type, event_max] {
            run(lfd, event_type, event_max, true);
        };

        acl::fiber::share_epoll(true);
        acl::fiber::schedule(fiber_event);
    } else {
        printf("Run event_server in aio mode...\r\n");
        run(lfd, event_type, event_max, false);
    }

	return 0;
}
