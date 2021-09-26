#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/lib_fiber.hpp>

class producer_fiber : public acl::fiber {
public:
	producer_fiber(const char* addr, int count, int& nfibers)
	: addr_(addr), count_(count), nfibers_(nfibers) {}

private:
	~producer_fiber(void) {}

private:
	acl::string addr_;
	int   count_;
	int&  nfibers_;

	// override
	void run(void) {
		int conn_timeout = 10, rw_timeout = 30;
		acl::socket_stream conn;
		if (conn.open(addr_, conn_timeout, rw_timeout) == false) {
			if (--nfibers_ == 0) {
				acl::fiber::schedule_stop();
			}
			delete this;
			return;
		}

		acl::string buf;
		for (int i = 0; i < count_; i++) {
			buf.format("%d\r\n", i);
			if (conn.write(buf) == -1) {
				printf("write error %s\r\n", acl::last_serror());
				break;
			}
			if (conn.gets(buf) == false) {
				printf("gets error %s\r\n", acl::last_serror());
				break;
			}

			int n = atoi(buf.c_str());
			if (n != i) {
				printf("invalid n=%d, i=%d\r\n", n, i);
				break;
			}
		}

		if (--nfibers_ == 0) {
			acl::fiber::schedule_stop();
		}
		delete this;
	}
};

class producer_thread : public acl::thread {
public:
	producer_thread(const char* addr, int count)
	: addr_(addr), count_(count) {}

private:
	~producer_thread(void) {}

private:
	acl::string addr_;
	int count_;

	// override
	void* run(void) {
		int nfibers = 100;

		for (int i = 0; i < nfibers; i++) {
			acl::fiber* fb = new producer_fiber(
				addr_, count_, nfibers);
			fb->start();
		}

		acl::fiber::schedule();

		printf("thread-%lu finished\r\n",
			(unsigned long) acl::thread::thread_self());

		return NULL;
	}
};

int main(int argc, char* argv[]) {
	acl::string addr("127.0.0.1:9001");
	int nthreads = 2, count = 1000;

	if (argc >= 2) {
		addr = argv[1];
	}
	if (argc >= 3) {
		nthreads = atoi(argv[2]);
	}
	if (argc >= 4) {
		count = atoi(argv[3]);
	}

	std::vector<acl::thread*> threads;
	for (int i = 0; i < nthreads; i++) {
		acl::thread* thr = new producer_thread(addr, count);
		thr->set_detachable(false);
		threads.push_back(thr);
		thr->start();
	}

	for (std::vector<acl::thread*>::iterator it = threads.begin();
		it != threads.end(); ++it) {
		(*it)->wait();
		delete *it;
	}

	return  0;
}
