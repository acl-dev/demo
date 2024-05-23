#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

int main() {
	acl::fiber_mutex locker;
	for (int i = 0; i < 2; i++) {
		go[&] {
			printf("fiber-%d begin to lock\r\n", acl::fiber::self());
			if (!locker.lock()) {
				printf("fiber-%d lock failed\r\n", acl::fiber::self());
				return;
			}
			printf("fiber-%d lock ok\r\n", acl::fiber::self());

			bool ret = locker.trylock();
			printf("fiber-%d tryock %s!\r\n", acl::fiber::self(), ret ? "ok" : "failed");
			::sleep(100);
		};
	}

	acl::fiber::schedule();
	return 0;
}
