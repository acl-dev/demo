#include <acl-lib/acl_cpp/lib_acl.hpp>

int main(void) {
	acl::redis_client_cluster conns;
	conns.set("127.0.0.1:6379", 0);

	size_t max = 100;
	acl::redis cmd(&conns);
	for (size_t i = 0; i < max; i++) {
		acl::string key, val;
		key.format("key-%d", (int) i);
		val.format("val-%d", (int) i);
		if (!cmd.set(key, val)) {
			printf("set error: %s\r\n", cmd.result_error());
			return 1;
		}
	}
	printf("Set all keys ok!\r\n\r\n");

	for (size_t i = 0; i < max; i++) {
		acl::string key, val;
		key.format("key-%d", (int) i);
		if (!cmd.get(key, val)) {
			printf("get error: %s\r\n", cmd.result_error());
			return 1;
		}

		if (i < 10) {
			printf("%s\r\n", val.c_str());
		}
		val.clear();
	}
	printf("Get all keys ok!\r\n\r\n");

	for (size_t i = 0; i < max; i++) {
		acl::string key;
		key.format("key-%d", (int) i);
		if (!cmd.del_one(key)) {
			printf("del key error %s\r\n", cmd.result_error());
			return 1;
		}
	}

	printf("Delete all keys ok!\r\n");
	return 0;
}
