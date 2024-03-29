#include <stdio.h>
#include <stdlib.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>  // fiber_tbox inherits from box in lib_acl_cpp
#include <acl-lib/fiber/libfiber.hpp>

class myfiber : public acl::fiber {
public:
	myfiber(acl::fiber_tbox<bool>& box) : box_(box) {}
	~myfiber(void) {}

private:
	acl::fiber_tbox<bool>& box_;

	// @override
	void run(void) {
		printf("fiber-%d-%d running\r\n", get_id(), acl::fiber::self());
		func1();
	}

	void func1(void) {
		func2();
	}

	void func2(void) {
		func3();
	}

	void func3(void) {
		box_.pop();
	}
};

class checker : public acl::fiber {
public:
	checker(acl::fiber_tbox<bool>& box, acl::fiber& fb)
	: box_(box), fb_(fb) {}
	~checker(void) {}

private:
	acl::fiber_tbox<bool>& box_;
	acl::fiber& fb_;

	void run(void) {
		std::vector<acl::fiber_frame> stack;
		acl::fiber::stacktrace(fb_, stack, 50);
		show_stack(stack);
		printf("\r\n");

		box_.push(NULL);
	}

	void show_stack(const std::vector<acl::fiber_frame>& stack) {
		for (std::vector<acl::fiber_frame>::const_iterator
			cit = stack.begin(); cit != stack.end(); ++cit) {
			printf("0x%lx(%s)+0x%lx\r\n",
				(*cit).pc, (*cit).func.c_str(), (*cit).off);
		}
	}
};

int main(void) {
	acl::acl_cpp_init();
	acl::log::stdout_open(true);

	acl::fiber_tbox<bool> box;
	acl::fiber* f1 = new myfiber(box);
	f1->start();

	acl::fiber* f2 = new checker(box, *f1);
	f2->start();

	acl::fiber::schedule();

	delete f1;
	delete f2;
	return 0;
}
