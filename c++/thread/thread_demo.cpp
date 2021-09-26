#include <acl-lib/acl_cpp/lib_acl.hpp>

class mythread : public acl::thread {
public:
	mythread(acl::tbox<acl::string>& tbox) : buf_(NULL), tbox_(tbox) {}
	~mythread(void) { delete buf_; }

private:
	// @override
	void* run(void) {
		printf("hello world! thread-%lu\r\n", acl::thread::thread_self());
		buf_ = new acl::string;
		(*buf_) << "hello world: " << acl::thread::thread_self();
		tbox_.push(buf_);
		return buf_;
	}

	acl::string* buf_;
	acl::tbox<acl::string>& tbox_;
};

int main(void) {
	std::vector<acl::thread*> threads;
	acl::tbox<acl::string> tbox;

	for (size_t i = 0; i < 10; i++)
	{
		acl::thread* thr = new mythread(tbox);
		threads.push_back(thr);
		thr->set_detachable(false);
		thr->start();
	}

	for (size_t i = 0; i < 10; i++) {
		acl::string* buf = tbox.pop();
		printf("|%s|\r\n", buf->c_str());
	}

	for (std::vector<acl::thread*>::iterator it = threads.begin();
		it != threads.end(); ++it)
	{
		acl::string* buf;
		(*it)->wait((void**) &buf);
		printf("thread exit: %s\r\n", buf->c_str());
		delete *it;
	}

	printf("All threads finished!\r\n");
	return 0;
}
