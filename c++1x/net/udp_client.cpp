#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

static void run(size_t& count) {
	const char* laddr = "0.0.0.0|0";
	const char* saddr = "127.0.0.1|8088";
	acl::socket_stream ss;
	if (!ss.bind_udp(laddr, 10)) {
		printf("Bind %s in UDP error %s\r\n",
			laddr, acl::last_serror());
		return;
	}

	printf("Bind local %s ok\r\n", ss.get_local(true));

	char buf[1024];
	const char* s = "hello world!\r\n";
	size_t i;
	for (i = 0; i < 1000000; i++) {
		if (ss.sendto(s, strlen(s), saddr) == -1) {
			printf("Write error %s\r\n", acl::last_serror());
			break;
		}

		int ret = ss.read(buf, sizeof(buf), false);
		if (ret == -1) {
			printf("Read error %s\r\n", acl::last_serror());
			break;
		}

		++count;

		if (i > 0 && i % 10000 == 0) {
			char tmp[256];
			snprintf(tmp, sizeof(tmp), "count=%zd", i);
			acl::meter_time(__func__, __LINE__, tmp);
		}
	}

	printf("Over, total=%zd\r\n", i);
}

int main() {
	acl::log::stdout_open(true);

	struct timeval begin;
	gettimeofday(&begin, nullptr);
	size_t count = 0;

	for (size_t j = 0; j < 5; j++) {
		go[&count] {
			run(count);
		};
	}

	acl::fiber::schedule();

	struct timeval end;
	gettimeofday(&end, nullptr);
	double cost = acl::stamp_sub(end, begin);
	double speed = (count * 1000) / (cost > 0 ? cost : 0.0001);

	printf("Total count=%zd, cost=%.2f ms, speed=%.2f\r\n",
		count, cost, speed);
	return 0;
}
