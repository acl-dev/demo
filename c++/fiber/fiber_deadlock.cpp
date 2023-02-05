#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class fiber1 : public acl::fiber {
public:
	fiber1(acl::fiber_mutex& lock1, acl::fiber_mutex& lock2)
	: lock1_(lock1), lock2_(lock2) {}
	~fiber1(void) {}

private:
	acl::fiber_mutex& lock1_;
	acl::fiber_mutex& lock2_;

	// @override
	void run(void) {
		lock1_.lock();
		sleep(1);
		lock2_.lock();
	}
};

class fiber2 : public acl::fiber {
public:
	fiber2(acl::fiber_mutex& lock1, acl::fiber_mutex& lock2)
	: lock1_(lock1), lock2_(lock2) {}
	~fiber2(void) {}

private:
	acl::fiber_mutex& lock1_;
	acl::fiber_mutex& lock2_;

	// @override
	void run(void) {
		lock2_.lock();
		sleep(1);
		lock1_.lock();
	}
};

class checker : public acl::fiber {
public:
	checker(void) {}
	~checker(void) {}

private:
	// @override
	void run(void) {
		sleep(2);
		acl::fiber_mutex::deadlock_show();
	}
};

int main(void) {
	acl::fiber_mutex lock1, lock2;

	fiber1 fb1(lock1, lock2);
	fb1.start();

	fiber2 fb2(lock1, lock2);
	fb2.start();

	checker ck;
	ck.start();

	acl::fiber::schedule();
	return 0;
}
