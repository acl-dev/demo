#include <thread>
#include <acl-lib/acl_cpp/lib_acl.hpp>

static void sender(const acl::string& addr, size_t max) {
	acl::socket_stream conn;
	if (!conn.open(addr, 10, 10)) {
		printf("connect %s error %s\r\n", addr.c_str(), acl::last_serror());
		return;
	}

	char buf[8192];
	memset(buf, 'x', sizeof(buf));

	for (size_t i = 0; i < max; i++) {
		if (conn.write(buf, sizeof(buf)) == -1) {
			printf("write error %s\r\n", acl::last_serror());
			break;
		}
	}
}


int main(int argc, char* argv[]) {
	acl::acl_cpp_init();
	acl::string addr("127.0.0.1|9101");
	if (argc >= 2) {
		addr = argv[1];
	}

	size_t max = 100000;
	if (argc >= 3) {
		max = (size_t) atol(argv[2]);
	}

	size_t nthreads = 2;
	if (argc >= 4) {
		nthreads = (size_t) atol(argv[3]);
	}
	if (nthreads > 20) {
		nthreads = 20;
	}

	std::vector<std::thread*> threads;
	for (size_t i = 0; i < nthreads; i++) {
		std::thread* thread = new std::thread(sender, std::ref(addr), max);
		threads.push_back(thread);
	}

	for (std::vector<std::thread*>::iterator it = threads.begin();
		it != threads.end(); ++it) {
		(*it)->join();
		delete *it;
	}

	return 0;
}
