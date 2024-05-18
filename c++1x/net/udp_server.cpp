#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

static void run() {
	const char* addr = "0.0.0.0|8088";
	acl::socket_stream ss;
	if (!ss.bind_udp(addr, 10, acl::OPEN_FLAG_REUSEPORT)) {
		printf("Bind %s in UDP error %s\r\n",
			addr, acl::last_serror());
		return;
	}

	printf("Bind udp at %s ok\r\n", addr);

	char buf[1024];
	size_t i = 0;

	while (true) {
		time_t begin = time(NULL);
		int ret = ss.read(buf, sizeof(buf), false);
		if (ret == -1) {
			time_t now = time(NULL);
			printf("Read error %s, cost=%ld\r\n",
				acl::last_serror(), now - begin);

			if (errno == ETIMEDOUT) {
				continue;
			}

			break;
		}

		if (ss.write(buf, (size_t) ret) == -1) {
			printf("Write error %s\r\n", acl::last_serror());
			break;
		}

		if (++i % 10000 == 0) {
			char tmp[256];
			snprintf(tmp, sizeof(tmp), "count=%zd", i);
			acl::meter_time(__func__, __LINE__, tmp);
		}
	}
}

int main() {
	acl::log::stdout_open(true);

	for (size_t j = 0; j < 5; j++) {
		go[&] {
			run();
		};
	}

	acl::fiber::schedule();
	return 0;
}
