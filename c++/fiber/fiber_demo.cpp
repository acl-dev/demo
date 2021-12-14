#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/fiber.hpp>

class myfiber : public acl::fiber {
public:
	myfiber(void) {}

protected:
	~myfiber(void) {}

	// @override
	void run(void) {
		printf("fiber-%d-%d running\r\n", get_id(), acl::fiber::self());
		delete this;
	}
};

int main(void) {
	int  n = 10;

	acl::acl_cpp_init();
	acl::log::stdout_open(true);

	for (int i = 0; i < n; i++) {
		acl::fiber* f = new myfiber();
		f->start();
	}

	acl::fiber::schedule();
	return 0;
}
