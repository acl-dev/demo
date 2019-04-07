#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class fiber_redis : public acl::fiber
{
public:
	fiber_redis(acl::redis_client_cluster& cluster) : cluster_(cluster) {}
private:
	~fiber_redis(void) {}
private:
	acl::redis_client_cluster& cluster_;
	// @override
	void run(void) {
		const char* key = "hash-key";
		for (int i = 0; i < 100; i++) {
			acl::redis cmd(&cluster_);
			acl::string name, val;
			name.format("hash-name-%d", i);
			val.format("hash-val-%d", i);
			if (cmd.hset(key, name, val) == -1) {
				printf("hset error: %s, key=%s, name=%s\r\n",
					cmd.result_error(), key, name.c_str());
				break;
			} else {
				printf("hset ok, key=%s, name=%s\r\n",
					key, name.c_str());
			}
		}
		delete this;
	}
};

int main(void)
{
	const char* redis_addr = "127.0.0.1:9000";
	acl::redis_client_cluster cluster;
	cluster.set(redis_addr, 0);
	for (int i = 0; i < 100; i++) {
		acl::fiber* fb = new fiber_redis(cluster);
		fb->start();
	}
	acl::fiber::schedule();
	return 0;
}
