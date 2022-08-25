#include <acl-lib/acl_cpp/lib_acl.hpp>

int main(void) {
	acl::redis_client_pipeline pipeline("127.0.0.1:6379");
	pipeline.set_password("Wabjtam123");
	pipeline.start_thread();

	acl::log::stdout_open(true);
	acl::redis cmd;
	cmd.set_pipeline(&pipeline);
	acl::string key, name, value;

	for (int i = 0; i < 10000; i++) {
		key.format("key-%d", i);
		name.format("name-%d", i);
		value.format("value-%d", i);
		if (cmd.hset(key, name, value) < 0) {
			printf(">>hset error=%s, key=%s, name=%s\r\n",
				cmd.result_error(), key.c_str(), name.c_str());
			break;
		}
		cmd.clear();
	}

	printf(">>>hset ok\r\n");

	for (int i = 0; i < 10000; i++) {
		key.format("key-%d", i);
		name.format("name-%d", i);
		if (cmd.hget(key, name, value) == false) {
			printf(">>hget error=%s, key=%s, name=%s\r\n",
				cmd.result_error(), key.c_str(), name.c_str());
			break;
		}
		cmd.clear();
	}

	printf(">>>hget ok\r\n");
	pipeline.stop_thread();
	return 0;
}
