#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class myobj
{
public:
	myobj(void) {}
	~myobj(void) {}

	void run(void)
	{
		printf("hello thread=%lu\r\n", acl::thread::thread_self());
	}
};

#define MAX	100000000

class producer : public acl::thread
{
public:
	producer(acl::mbox<myobj>& mbox) : mbox_(mbox) {}
	~producer(void) {}

private:
	acl::mbox<myobj>& mbox_;

	void* run(void)
	{
		printf("producer: %lu\r\n", acl::thread::thread_self());
		for (int i = 0; i < MAX; i++) {
			myobj* o = new myobj;
			mbox_.push(o);
		}
		printf("producer over\r\n");
		return NULL;
	}
};

class consumer : public acl::thread
{
public:
	consumer(acl::mbox<myobj>& mbox) : mbox_(mbox) {}
	~consumer(void) {}

private:
	acl::mbox<myobj>& mbox_;

	void* run(void)
	{
		printf("consumer: %lu\r\n", acl::thread::thread_self());
		sleep(1);
		printf("wakeup\r\n");

		for (int i = 0; i < MAX; i++) {
			myobj* o = mbox_.pop();
			if (i < 10) {
				o->run();
			}
			delete o;
		}
		return NULL;
	}
};

int main(void)
{
	acl::mbox<myobj> mbox;

	producer producer(mbox);
	producer.set_detachable(false);
	producer.start();

	consumer consumer(mbox);
	consumer.set_detachable(false);
	consumer.start();

	producer.wait();
	consumer.wait();
	return 0;
}
