#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class myfiber : public acl::fiber {
public:
	myfiber(acl::fiber_event& event, unsigned long long& count)
	: event_(event)
	, count_(count)
	{}

protected:
	// @override
	void run(void) {
		for (int i = 0; i < 1000; i++) {
			event_.wait();
			count_++;
			if (count_ % 100000 == 0) {
				printf("thread-%ld, fiber-%u, count=%llu\r\n",
					acl::thread::self(), acl::fiber::self(),
					count_);
			}
			event_.notify();
		}

		delete this;
	}

private:
	acl::fiber_event& event_;
	unsigned long long& count_;

	~myfiber(void) {}
};

class mythread : public acl::thread {
public:
	mythread(acl::fiber_event& event, unsigned long long& count)
	: event_(event)
	, count_(count)
	{}

	~mythread(void) {}

protected:
	// @override
	void* run(void) {
		for (int i = 0; i < 1000; i++) {
			acl::fiber* fb = new myfiber(event_, count_);
			fb->start();
		}

		acl::fiber::schedule();
		return NULL;
	}

private:
	acl::fiber_event& event_;
	unsigned long long& count_;
};

int main(void) {
	unsigned long long count = 0;
	acl::fiber_event event;

	std::vector<acl::thread*> threads;
	for (int i = 0; i < 2; i++) {
		acl::thread* thr = new mythread(event, count);
		threads.push_back(thr);
		thr->start();
	}

	for (std::vector<acl::thread*>::iterator it = threads.begin();
		it != threads.end(); ++it) {

		(*it)->wait();
		delete *it;
	}

	printf("at last count=%llu\r\n", count);
	return 0;
}
