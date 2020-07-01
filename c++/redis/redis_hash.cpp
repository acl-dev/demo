#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <unistd.h>

int main(void)
{
	acl::redis_client_cluster cluster;
	cluster.set("127.0.0.1:9001", 0);

	acl::redis cmd(&cluster);
	std::map<acl::string, acl::string> attrs;
	attrs["name1"] = "value1";
	attrs["name2"] = "value2";
	attrs["name3"] = "value3";

	const char* key = "key";
	if (!cmd.hmset(key, attrs)) {
		printf("hmset error=%s\r\n", cmd.result_error());
		return 1;
	}
	printf("hmset ok, key=%s\r\n", key);
	cmd.clear();

	int ttl = 5;
	if (cmd.expire(key, ttl) <= 0) {
		printf("expire error=%s, key=%s, ttl=%d\r\n",
			cmd.result_error(), key, ttl);
		return 1;
	}
	printf("expire ok, key=%s, ttl=%d\r\n", key, ttl);
	cmd.clear();

	for (int i = 0; i < 3; i++) {
		sleep(1);
		printf("sleep %d seconds\r\n", i + 1);
	}
	
	if (cmd.hset(key, "name4", "value4") == -1) {
		printf("hset error=%s, key=%s\r\n", cmd.result_error(), key);
		return 1;
	}
	printf("hset ok, key=%s\r\n", key);
	cmd.clear();

	ttl = 5;
	if (cmd.expire(key, 5) <= 0) {
		printf("expire error=%s, key=%s, ttl=%d\r\n",
			cmd.result_error(), key, ttl);
		return 1;
	}
	printf("expire ok, key=%s, ttl=%d\r\n", key, ttl);
	attrs.clear();

	for (int i = 0; i < 4; i++) {
		sleep(1);
		printf("sleep %d seconds\r\n", i + 1);
	}

	if (!cmd.hgetall(key, attrs)) {
		printf("hgetall error=%s, key=%s\r\n", cmd.result_error(), key);
		return 1;
	}

	printf("hgetall result for key=%s\r\n", key);
	for (std::map<acl::string, acl::string>::const_iterator
		cit = attrs.begin(); cit != attrs.end(); ++cit) {

		printf("%s=%s\r\n", cit->first.c_str(), cit->second.c_str());
	}

	return 1;
}
