#include <stdio.h>
#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class mythread : public acl::thread {
public:
	mythread(acl::fiber_tbox<bool>& box, int& n) : box_(box), n_(n) {}
	~mythread(void) {}

protected:
	void* run(void) {
		for (int i = 0; i < 10; i++) {
			sleep(1);
			n_++;
			printf("wakeup thread=%lu, n=%d\r\n", this->self(), n_);
		}

		box_.push(NULL);
		return NULL;
	}

private:
	acl::fiber_tbox<bool>& box_;
	int& n_;
};

class myfiber : public acl::fiber {
public:
	myfiber(void) {}

protected:
	~myfiber(void) {}

	// @override
	void run(void) {
		int n = 0;
		mythread thread(box_, n);
		thread.start();

		(void) box_.pop();
		thread.wait();

		printf("thread-%lu: at last n is %d\r\n", acl::thread::self(), n);

		delete this;
	}

private:
	acl::fiber_tbox<bool> box_;
};

int main(void) {
	acl::fiber* fb = new myfiber;
	fb->start();

	acl::fiber::schedule();
	return 0;
}
