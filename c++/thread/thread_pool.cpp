#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class mythread : public acl::thread_job {
public:
	mythread(void) {}

private:
	// @override	
	void* run(void) {
		printf("thread: %lu run\r\n", acl::thread::thread_self());
		sleep(1);
		printf("thread: %lu over\r\n", acl::thread::thread_self());

		delete this;
		return NULL;
	}

	~mythread(void) {}
};

int main(void) {
	size_t max_threads = 2;
	acl::thread_pool* threads = new acl::thread_pool;
	threads->set_limit(max_threads);
	threads->start();

	for (size_t i = 0; i < 10; i++) {
		acl::thread_job* job = new mythread;
		threads->execute(job);
	}

	threads->stop();
	delete threads;
	printf("All threads were finished!\r\n");
	return 0;
}
