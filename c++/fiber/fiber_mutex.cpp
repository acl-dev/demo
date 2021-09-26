#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class myfiber : public acl::fiber {
public:
	myfiber(acl::fiber_mutex& lock) : lock_(lock) {}

protected:
	void run(void) {
		for (int i = 0; i < 5; i++) {
			lock_.lock();
			printf("locked by fiber-%u and sleep\r\n",
				acl::fiber::self());
			sleep(1);
			printf("fiber-%u wakeup\r\n", acl::fiber::self());
			lock_.unlock();
		}

		delete this;
	}

private:
	acl::fiber_mutex& lock_;
	~myfiber(void) {}
};

int main(void) {
	acl::fiber_mutex lock;

	acl::fiber* fb1 = new myfiber(lock);
	fb1->start();

	acl::fiber* fb2 = new myfiber(lock);
	fb2->start();
	acl::fiber::schedule();

	return 0;
}
