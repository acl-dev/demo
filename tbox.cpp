#include <acl-lib/acl_cpp/lib_acl.hpp>

class myobj
{
public:
	myobj(void) {}
	~myobj(void) {}
};

static acl::atomic_long __curr_producer = 0;

class producer : public acl::thread
{
public:
	producer(acl::tbox<myobj>& tbox, int max) : tbox_(tbox), max_(max) {}
	~producer(void) {}

private:
	acl::tbox<myobj>& tbox_;
	int max_;

	void* run(void)
	{
		for (int i = 0; i < max_; i++) {
			myobj* o = new myobj;
			(void) tbox_.push(o);
		}

		__curr_producer--;
		printf("thread-%lu over, max=%d\r\n", acl::thread::self(), max_);
		return NULL;
	}
};

class consumer : public acl::thread
{
public:
	consumer(acl::tbox<myobj>& tbox, int nproducers)
	: tbox_(tbox), nproducers_(nproducers) {}
	~consumer(void) {}

private:
	acl::tbox<myobj>& tbox_;
	int nproducers_;

	void* run(void)
	{
		int n = 0;
		while (true) {
			myobj* o = tbox_.pop(500);
			if (o) {
				delete o;
				n++;
			}
			if (__curr_producer == 0 && tbox_.size() == 0)
				break;
		}
		printf("thread-%lu pop over, n=%d\r\n", acl::thread::self(), n);
		return NULL;
	}
};

int main(void)
{
	int max = 10000000, max_producers = 2;
	acl::tbox<myobj> tbox;

	__curr_producer = max_producers;

	std::vector<acl::thread*> producers;
	for (int i = 0; i < max_producers; i++) {
		acl::thread* thr = new producer(tbox, max);
		thr->set_detachable(false);
		producers.push_back(thr);
		thr->start();
	}

	consumer c1(tbox, 2);
	c1.set_detachable(false);
	c1.start();

	consumer c2(tbox, 2);
	c2.set_detachable(false);
	c2.start();

	for (std::vector<acl::thread*>::iterator it = producers.begin();
		it != producers.end(); ++it) {

		(*it)->wait();
		delete *it;
	}

	c1.wait();
	c2.wait();
	return 0;
}
