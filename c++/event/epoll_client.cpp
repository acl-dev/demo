/**
 * Copyright (C) 2015-2018
 * All rights reserved.
 *
 * AUTHOR(S)
 *   Zheng Shuxin
 *   E-mail: shuxin.zheng@qq.com
 * 
 * VERSION
 *   Sun 03 Dec 2017 12:20:44 PM CST
 */

#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class client_thread : public acl::thread {
public:
	client_thread(const char* addr, int nloop) : addr_(addr), nloop_(nloop) {}

private:
	~client_thread(void) {}

	void* run(void) {
		if (conn_.open(addr_, 0, 0) == false) {
			printf("open %s error %s\r\n",
				addr_.c_str(), acl::last_serror());
			return NULL;
		}

		printf("thread-%lu connect %s ok\r\n", acl::thread::self(),
			addr_.c_str());
		acl::string s = "hello world\r\n", buf;
		for (int i = 0; i < nloop_; i++) {
			if (conn_.write(s) == -1) {
				printf("write error %s\r\n", acl::last_serror());
				break;
			}

			if (conn_.read(buf, false) == false) {
				printf("read error %s\r\n", acl::last_serror());
				break;
			}

			if (i < 10)
				printf("thread-%lu, write %d ok, read=[%s]\r\n",
					acl::thread::self(), i, buf.c_str());
		}
		printf("write over\r\n");

		return NULL;
	}

private:
	acl::string addr_;
	acl::socket_stream conn_;
	int nloop_;
};

static void usage(const char* procname) {
    printf("usage: %s -h [help] -s addr -c nthreads -n loop\r\n", procname);
}

int main(int argc, char* argv[]) {
	int  ch;
	acl::string addr("127.0.0.1:8887");
	int  nthreads = 1, nloop = 10;

	while ((ch = getopt(argc, argv, "c:s:n:h")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'c':
			nthreads = atoi(optarg);
			break;
		case 's':
			addr = optarg;
			break;
		case 'n':
			nloop = atoi(optarg);
			break;
		default:
			break;
		}
	}

	std::vector<acl::thread*> threads;
	for (int i = 0; i < nthreads; i++) {
		acl::thread* thr = new client_thread(addr, nloop);
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
