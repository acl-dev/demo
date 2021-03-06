#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class myobj
{
public:
	myobj(void) : i_(0) {}
	~myobj(void) {}
	void set(int i) {
		i_ = i;
	}
	void test(void) {
		printf("hello world, i=%d\r\n", i_);
	}
private:
	int i_;
};

// 消费者协程，从消息管道中读取消息
class fiber_consumer : public acl::fiber
{
public:
	fiber_consumer(acl::fiber_tbox<myobj>& box) : box_(box) {}
private:
	~fiber_consumer(void) {}
private:
	acl::fiber_tbox<myobj>& box_;
	// @override
	void run(void) {
		while (true) {
			myobj* o = box_.pop();
			// 如果读到空消息，则结束
			if (o == NULL) {
				break;
			}
			o->test();
			delete o;
		}
		delete this;
	}
};

// 生产者线程，向消息管道中放置消息
class thread_producer : public acl::thread
{
public:
	thread_producer(acl::fiber_tbox<myobj>& box) : box_(box) {}
	~thread_producer(void) {}
private:
	acl::fiber_tbox<myobj>& box_;
	void* run(void) {
		for (int i = 0; i < 10; i++) {
			myobj* o = new myobj;
			o->set(i);
			box_.push(o);
		}
		box_.push(NULL);
		return NULL;
	}
};

int main(void)
{
	acl::fiber_tbox<myobj> box;
	// 创建并启动消费者协程
	acl::fiber* consumer = new fiber_consumer(box);
	consumer->start();
	// 创建并启动生产者线程
	acl::thread* producer = new thread_producer(box);
	producer->start();
	// 启动协程调度器
	acl::fiber::schedule();
	// 等待生产者线程退出
	producer->wait();
	delete producer;
	return 0; 
}
