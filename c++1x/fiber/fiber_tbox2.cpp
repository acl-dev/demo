#include <thread>
#include <memory>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

using shared_box = std::shared_ptr<acl::fiber_tbox2<int>>;

const int MAX = 20;

int main(void) {
	shared_box box(new acl::fiber_tbox2<int>());
	int nCount = 0;

	std::thread thr([&nCount, box] {
		go[&nCount, box] {
			for (int i = 0; i < MAX; i++) {
				go[&nCount, box] {
					int nVal = -1;
					box->pop(nVal,100);
					nCount++;
					printf("fiber-%d: pop one: %d\r\n",
						acl::fiber::self(), nVal);
				};
			}
		};

		acl::fiber::schedule();
	});

	for(int i = 0; i < MAX; i++) {
		box->push(i);
	}

	thr.join();
	printf("nCount=%d\r\n", nCount);

	return 0;
}
