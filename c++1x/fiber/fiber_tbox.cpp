#include <stdio.h>
#include <thread>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/fiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

int main(void) {
	acl::acl_cpp_init();
	acl::fiber_tbox<acl::socket_stream> box;

	for (int port = 8081; port < 8084; port++) {
		acl::string addr;
		addr.format("127.0.0.1|%d", port);
		acl::socket_stream* stream = new acl::socket_stream;
		if (stream->bind_udp(addr)) {
			box.push(stream);
		} else {
			printf("bind addr=%s error=%s\r\n", addr.c_str(), acl::last_serror());
			delete stream;
		}
	}

	std::thread thread([&] {
		go[&] {
			printf("fiber-%d: wait...\r\n", acl::fiber::self());

			while (true) {
				acl::socket_stream* stream = box.pop();
				if (stream == NULL) {
					break;
				}

				printf("fiber-%d: got stream bind on %s\r\n", acl::fiber::self(), stream->get_local(true));

				go[=] {
					while (!stream->eof()) {
						char buf[1024];
						int ret = stream->read(buf, sizeof(buf) - 1, false);
						if (ret <= 0) {
							break;
						}

						if (stream->write(buf, (size_t) ret) != ret) {
							break;
						}
					}
					delete stream;
				};
			}
		};

		acl::fiber::schedule();
	});

	thread.join();
	return 0;
}
