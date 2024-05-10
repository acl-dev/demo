#include <thread>
#include <memory>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

using shared_box = std::shared_ptr<acl::fiber_tbox2<int>>;

int main(void) {
	std::thread th([] {
		int nCount = 0;

		go[&nCount] {
			shared_box box(new acl::fiber_tbox2<int>());

			go[&nCount, box] {
				for (int i = 0; i < 10; i++) {
					go[&nCount, box] {
						int nVal = 0;
						box->pop(nVal,100);
						nCount++;
						printf("pop one: %d\r\n", nVal);
					};
				}
			};

			for(int i = 0; i < 10; i++) {
				box->push(i);
			}

			//acl::fiber::delay(0);
		};

		acl::fiber::schedule();
	});

	th.join();
	return 0;
}
