#include <acl-lib/acl_cpp/lib_acl.hpp>

class redis_thread : public acl::thread
{
public:
	redis_thread(acl::redis_client_cluster& conns) : conns_(conns) {}
	~redis_thread(void) {}

protected:
	void* run(void)
	{
		for (int i = 0; i < 1000; i++) {
			acl::redis cmd(&conns_);
			acl::string key, val;
			key.format("key-%lu-%d", this->thread_id(), i);
			val.format("val-%lu-%d", this->thread_id(), i);
			if (cmd.set(key, val) == false) {
				printf("set error: %s\r\n", cmd.result_error());
				break;
			}
		}

		return NULL;
	}

private:
	acl::redis_client_cluster& conns_;
};

int main(void)
{
	acl::redis_client_cluster conns;
	conns.set("127.0.0.1:6379", 0);

	std::vector<acl::thread*> threads;
	for (int i = 0; i < 10; i++) {
		acl::thread* thr = new redis_thread(conns);
		thr->set_detachable(false);
		threads.push_back(thr);
		thr->start();
	}

	for (std::vector<acl::thread*>::iterator it = threads.begin();
		it != threads.end(); ++it) {

		(*it)->wait();
		delete *it;
	}

	return 0;
}
