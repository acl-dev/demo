#include <acl-lib/acl_cpp/lib_acl.hpp>

class mythread : public acl::thread
{
public:
	mythread(void) {}
	~mythread(void) {}

private:
	// @override
	void* run(void)
	{
		printf("hello world! thread-%lu\r\n", acl::thread::thread_self());
		return NULL;
	}
};

int main(void)
{
	std::vector<acl::thread*> threads;

	for (size_t i = 0; i < 10; i++)
	{
		acl::thread* thr = new mythread;
		threads.push_back(thr);
		thr->set_detachable(false);
		thr->start();
	}

	for (std::vector<acl::thread*>::iterator it = threads.begin();
		it != threads.end(); ++it)
	{
		(*it)->wait();
		delete *it;
	}

	printf("All threads finished!\r\n");
	return 0;
}
