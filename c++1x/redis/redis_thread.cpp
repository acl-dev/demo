#include <thread>
#include <vector>
#include <acl-lib/acl_cpp/lib_acl.hpp>

static void thread_main(acl::redis_client_cluster& conns, size_t id, size_t max) {
	acl::redis cmd(&conns);
	acl::string key, val;
	for (size_t i = 0; i < max; i++) {
		key.format("key-%zd-%zd", id, i);
		val.format("val-%zd", i);
		if (!cmd.set(key, val)) {  // add the specified key and value to redis
			printf("set %s error=%s\r\n", key.c_str(), cmd.result_error());
			break;
		}
		if (cmd.del(key) == -1) {  // delete the specified key from redis
			printf("del %s error=%s\r\n", key.c_str(), cmd.result_error());
			break;
		}
		cmd.clear();  // clear the temporary memory
	}
}

int main(void) {
	acl::redis_client_cluster conns;  // the connections with the redis cluster
	conns.set("127.0.0.1:6379", 0);   // set one address of the redis cluster
	size_t max = 5;
	const size_t nthreads = 10;

	std::thread threads[nthreads];
	for (size_t i = 0; i < nthreads; i++) {  // create some threads to test redis
		threads[i] = std::thread(thread_main, std::ref(conns), i, max);
	}
	for (size_t i = 0; i < nthreads; i++) {
		threads[i].join();  // wait for the thread to exit
	}
	return 0;
}
