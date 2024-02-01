#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class echo_handler : public acl::fiber {
public:
	echo_handler(int cfd) {
		conn_.open(cfd, false);
	}

private:
	~echo_handler() {}

protected:
	// @override
	void run() {
		char buf[512];

		while (true) {
			int ret = conn_.read(buf, sizeof(buf), false);
			if (ret <= 0) {
				break;
			}

			if (conn_.write(buf, (size_t) ret) != ret) {
				break;
			}
		}

		delete this;
	}

private:
	acl::socket_stream conn_;
};

class transfer_handler : public acl::fiber {
public:
	transfer_handler(acl::socket_stream& conn) : conn_(conn) {}
	~transfer_handler() {}

protected:
	// @override
	void run() {
		while (true) {
			char buf[256];
			int  cfd;

			int ret = acl_read_fd(conn_.sock_handle(), buf,
					(int) sizeof(buf) - 1, &cfd);
			if (ret <= 0 || cfd < 0) {
				printf("Read from dispatch error %s\r\n", acl::last_serror());
				break;
			}

			buf[ret] = 0;
			printf("Get one client from addr %s\r\n", buf);

			echo_handler* handler = new echo_handler(cfd);
			handler->start();
		}

		printf("transfer_handler exit now\r\n");
	}

private:
	acl::socket_stream& conn_;
};

static void usage(const char* procname) {
	printf("usage: %s -h [help] -s addr\r\n", procname);
}

int main(int argc, char* argv[]) {
	int ch;
	acl::string server_addr("./dispatch.sock");

	while ((ch = getopt(argc, argv, "hs:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			server_addr = optarg;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	acl::socket_stream conn;
	if (!conn.open(server_addr, 10, 10)) {
		printf("Connect %s error %s\r\n", server_addr.c_str(), acl::last_serror());
		return 1;
	}
	printf("Connect %s ok\r\n", server_addr.c_str());

	transfer_handler handler(conn);
	handler.start();

	acl::fiber::schedule();
	return 0;
}
