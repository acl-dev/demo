#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/fiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

int main(void)
{
	acl::acl_cpp_init();
	acl::string addr("127.0.0.1:9206");
	acl::log::stdout_open(true);

	acl::server_socket ss;
	if (ss.open(addr) == false) {
		printf("listen %s error %s\r\n", addr.c_str(), acl::last_serror());
		return 1;
	}

	printf("listen %s ok\r\n", addr.c_str());

	go[&] {
		while (true) {
			acl::socket_stream* conn = ss.accept();
			if (conn == NULL) {
				printf("accept error %s\r\n", acl::last_serror());
				break;
			}

			printf("accept one connection\r\n");

			go[=] {
				printf("one client fiber-%d started!\r\n", acl::fiber::self());

				char buf[8192];
				while (true) {
					int ret = conn->read(buf, sizeof(buf), false);
					if (ret == -1) {
						break;
					}
					if (conn->write(buf, ret) == -1) {
						break;
					}
				}
				delete conn;
			};
		}
	};

	acl::fiber::schedule();	// start fiber schedule
	return 0;
}
