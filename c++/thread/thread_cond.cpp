#include <acl-lib/acl_cpp/lib_acl.hpp>

static long long __count, __add, __dec;

class consumer : public acl::thread {
public:
	consumer(acl::thread_cond& cond, acl::thread_mutex& mutex)
	: cond_(cond)
	, mutex_(mutex)
	{}

	~consumer(void) {}

private:
	void* run(void) {
		long long timeo = 10000;

		mutex_.lock();

		while (true) {
			if (cond_.wait(timeo, true) == false) {
				printf("wait %s\r\n", acl::last_serror());
			}

			while (__count > 0) {
				--__count;
			}

			if (++__dec % 100000 == 0) {
				printf("count=%lld, add=%lld, dec=%lld\r\n",
					__count, __add, __dec);
			}
		}

		mutex_.unlock();
		return NULL;
	}

private:
	acl::thread_cond&  cond_;
	acl::thread_mutex& mutex_;
};

int main(void) {
	acl::thread_mutex mutex;
	acl::thread_cond  cond(&mutex);

	acl::thread* thr = new consumer(cond, mutex);
	thr->set_detachable(false);
	thr->start();

	while (true) {
		assert(mutex.lock());
		__count++;
		__add++;
		assert(mutex.unlock());
		assert(cond.notify());
	}

	thr->wait();
	delete thr;
	return 0;
}
