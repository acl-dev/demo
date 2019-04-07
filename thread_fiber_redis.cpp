#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

// 每个协程共享相同的 cluster 对象，向 redis-server 中添加数据
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
		for (int i = 0; i < 10; i++) {
			acl::redis cmd(&cluster_);
			acl::string name, val;
			name.format("hash-name-%d", i);
			val.format("hash-val-%d", i);
			if (cmd.hset(key, name, val) == -1) {
				printf("hset error: %s, key=%s, name=%s\r\n",
					cmd.result_error(), key, name.c_str());
				break;
			} else {
				printf("set key %s, name %s ok\r\n",
					key, name.c_str());
			}
		}
		delete this;
	}
};

// 每个线程运行一个独立的协程调度器
class mythread : public acl::thread
{
public:
	mythread(acl::redis_client_cluster& cluster) : cluster_(cluster) {}
	~mythread(void) {}
private:
	acl::redis_client_cluster& cluster_;
	// @override
	void* run(void) {
		for (int i = 0; i < 10; i++) {
			acl::fiber* fb = new fiber_redis(cluster_);
			fb->start();
		}
		acl::fiber::schedule();
		return NULL;
	}
};

int main(void)
{
	const char* redis_addr = "127.0.0.1:9000";
	acl::redis_client_cluster cluster;
	cluster.set(redis_addr, 0);
	cluster.bind_thread(true);

	// 创建多个线程，共享 redis 集群连接池管理对象：cluster，即所有线程中的
	// 所有协程共享同一个 cluster 集群管理对象
	std::vector<acl::thread*> threads;
	for (int i = 0; i < 4; i++) {
		acl::thread* thr = new mythread(cluster);
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
