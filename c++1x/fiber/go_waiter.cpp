#include <stdio.h>
#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/fiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

static void wait4fiber(void) {
	for (int i = 0; i < 10; i++) {
		sleep(1);
		printf("%s: wakeup i=%d, thread=%lu, fiber=%d\r\n",
			__func__, i, acl::thread::self(), acl::fiber::self());
	}
}

static void wait4thread(void) {
	const char* myname = "wait4thread";
	int n = 0;

	go_wait_thread[&] {
		for (int i = 0; i < 10; i++) {
			sleep(1);
			printf("%s: wakeup i=%d, thread=%lu, fiber=%d\r\n",
				myname, i, acl::thread::self(), acl::fiber::self());
			n++;
		}
	};

	printf("%s: result=%d, thread=%lu, fiber=%d\r\n",
		__func__, n, acl::thread::self(), acl::fiber::self());
}

int main(void) {
	printf("current thread=%lu\r\n", acl::thread::self());

	go wait4fiber;
	go wait4fiber;
	go wait4thread;
	go wait4thread;

	acl::fiber::schedule();
	return 0;
}
