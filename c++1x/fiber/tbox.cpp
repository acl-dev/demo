#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory>
#include <thread>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/fiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>


int main(void) {
	acl::acl_cpp_init();
	acl::log::stdout_open(true);

	auto waiter1 = std::make_shared<acl::fiber_tbox<bool>>();
	auto waiter2 = std::make_shared<acl::fiber_tbox<bool>>();

	std::thread thr1([=] {
		go[=] {
			printf("Begin pop message\r\n");
			bool* r = waiter1->pop();
			printf("Got r=%p\r\n", r);
			acl::fiber::schedule_stop();
		};

		acl::fiber::schedule();
		waiter2->push(NULL);
	});

	std::thread thr2([=] {
		printf("Begin sleep 2 seconds\r\n");
		::sleep(1);

		printf(">>>Wakeup and send to box\n");
		waiter1->push(NULL);

		printf(">>>Waiting for waiter\r\n");
		waiter2->pop();
	});

	thr1.join();
	thr2.join();

	printf("All over\r\n");
	return 0;
}
