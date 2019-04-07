#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class mythread : public acl::thread
{
public:
	mythread(acl::fiber_tbox<int>& box) : box_(box) {}
	~mythread(void) {}
private:
	acl::fiber_tbox<int>& box_;
	// @override
	void* run(void) {
		int i;
		for (i = 0; i < 5; i++) {
			/* 假设这是一个比较耗时的操作*/
			printf("sleep one second\r\n");
			sleep(1);
		}
		int* n = new int(i);
		box_.push(n);
		return NULL; 
	}
};

class myfiber : public acl::fiber
{
public:
	myfiber(void) {}
	~myfiber(void) {}
private:
	// @override
	void run(void) {
		acl::fiber_tbox<int> box;
		mythread thread(box);
		thread.set_detachable(true);
		thread.start();
		int* n = box.pop();
		printf("n is %d\r\n", *n);
		delete n;
	}
};

int main(void)
{
	myfiber fb;
	fb.start();
	acl::fiber::schedule();
	return 0;
}
