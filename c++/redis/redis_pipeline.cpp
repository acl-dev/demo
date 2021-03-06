#include <acl-lib/acl_cpp/lib_acl.hpp>

class redis_thread : public acl::thread {
public:
	redis_thread(acl::redis_client_pipeline& pipeline) : pipeline_(pipeline) {}
	~redis_thread(void) {}

protected:
	// @override
	void* run(void) {
		acl::string key, value;
		acl::redis cmd(&pipeline_);
		cmd.set_pipeline(&pipeline_);

		for (int i = 0; i < 10000; i++) {
			key.format("key-%lu-%d", acl::thread::self(), i);
			value.format("val-%lu-%d", acl::thread::self(), i);
			if (!cmd.set(key, value)) {
				printf("set %s error, %s\r\n",
					key.c_str(), cmd.result_error());
				break;
			}

			if (!cmd.get(key, value)) {
				printf("get %s error, %s\r\n",
					key.c_str(), cmd.result_error());
				break;
			}

			if (cmd.del(key) < 0) {
				printf("del %s error: %s\r\n",
					key.c_str(), cmd.result_error());
				break;
			}
			cmd.clear();
		}

		return NULL;
	}

private:
	acl::redis_client_pipeline& pipeline_;
};

int main(void) {
	acl::redis_client_pipeline pipeline("10.110.28.210:9001");
	pipeline.set_password("Wabjtam123");
	pipeline.start_thread();

	std::vector<acl::thread*> threads;
	for (int i = 0; i < 100; i++) {
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
