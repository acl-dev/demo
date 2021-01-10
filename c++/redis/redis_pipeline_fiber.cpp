#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class redis_fiber : public acl::fiber {
public:
	redis_fiber(acl::redis_client_pipeline& pipeline) : pipeline_(pipeline) {}
	~redis_fiber(void) {}

protected:
	// @override from acl::fiber
	void run(void) {
		acl::string key;
		acl::redis cmd(&pipeline_);
		cmd.set_pipeline(&pipeline_);

		for (int i = 0; i < 10000; i++) {
			key.format("key-%lu-%d-%d", acl::thread::self(),
				acl::fiber::self(), i);
			if (cmd.del(key) < 0) {
				printf("del %s error: %s\r\n",
					key.c_str(), cmd.result_error());
				break;
			}
			cmd.clear();
		}
	}

private:
	acl::redis_client_pipeline& pipeline_;
};

class redis_thread : public acl::thread {
public:
	redis_thread(acl::redis_client_pipeline& pipeline) : pipeline_(pipeline) {}
	~redis_thread(void) {}

protected:
	// @override from acl::thread
	void* run(void) {
		std::vector<acl::fiber*> fibers;
		for (size_t i = 0; i < 100; i++) {
			acl::fiber* fb = new redis_fiber(pipeline_);
			fb->start();
			fibers.push_back(fb);
		}

		acl::fiber::schedule();

		for (std::vector<acl::fiber*>::iterator it = fibers.begin();
			it != fibers.end(); ++it) {
			delete *it;
		}
		return NULL;
	}

private:
	acl::redis_client_pipeline& pipeline_;
};

int main(void) {
	acl::redis_client_pipeline pipeline("127.0.0.1:6379");
	pipeline.start_thread();

	std::vector<acl::thread*> threads;
	for (int i = 0; i < 4; i++) {
		acl::thread* thr = new redis_thread(pipeline);
		threads.push_back(thr);
		thr->start();
	}

	for (std::vector<acl::thread*>::iterator it = threads.begin();
		it != threads.end(); ++it) {

		(*it)->wait();
		delete *it;
	}

	pipeline.stop_thread();
	return 0;
}
