#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/fiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

int main(void) {
	acl::acl_cpp_init();
	acl::string addr("127.0.0.1:9206");
	acl::server_socket ss;
	if (!ss.open(addr)) {
		printf("listen %s error %s\r\n", addr.c_str(), acl::last_serror());
		return 1;
	}
	printf("listen %s ok\r\n", addr.c_str());

	go[&] {
		while (true) {
			acl::socket_stream* conn = ss.accept();
			if (conn == NULL) {
				break;
			}
			go[=] {
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
