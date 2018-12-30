#include <acl-lib/acl_cpp/lib_acl.hpp>

int main(void)
{
	size_t max = 1000;

	std::vector<acl::string> members;
	members.reserve(max);
	acl::string member;
	for (size_t i = 0; i < max; i++) {
		member.format("member-%lu", i);
		members.push_back(member);
	}

	acl::redis_client_cluster conns;
	conns.set("127.0.0.1:6379", 0);

	const char* key = "redis-set-key";
	acl::redis cmd(&conns);
	int ret = cmd.sadd(key, members);
	if (ret < 0) {
		printf("sadd key=%s, error=%s\r\n", key, cmd.result_error());
		return 1;
	}
	printf("sadd ok, key=%s, ret=%d\r\n", key, ret);

	cmd.clear();
	members.clear();

	ret = cmd.smembers(key, &members);
	if (ret < 0) {
		printf("smembers error=%s, key=%s\r\n", cmd.result_error(), key);
		return 1;
	}
	printf("smembers ok, key=%s, ret=%d\r\n", key, ret);

	return 0;
}
