#include <acl-lib/acl_cpp/lib_acl.hpp>

class thread_client : public acl::thread
{
public:
	thread_client(acl::socket_stream* conn) : conn_(conn) {}

protected:
	// @override
	void* run(void) {
		printf("thread-%d running\r\n", acl::thread::self());

		char buf[8192];
		while (true) {
			int ret = conn_->read(buf, sizeof(buf), false);
			if (ret == -1)
				break;
			if (conn_->write(buf, ret) == -1)
				break;
		}

		delete conn_;
		delete this;
		return NULL;
	}

private:
	acl::socket_stream* conn_;

	~thread_client(void) {}
};

class thread_server : public acl::thread
{
public:
	thread_server(acl::server_socket& ss) : ss_(ss) {}
	~thread_server(void) {}

protected:
	// @override
	void* run(void) {
		while (true) {
			acl::socket_stream* conn = ss_.accept();
			if (conn == NULL) {
				printf("accept error %s\r\n", acl::last_serror());
				break;
			}

			printf("accept ok, fd: %d\r\n", conn->sock_handle());
			// create one thread for one connection
			thread_client* thr = new thread_client(conn);
			thr->set_detachable(true);
			// start the thread
			thr->start();
		}
		return NULL;
	}

private:
	acl::server_socket& ss_;
};

int main(void)
{
	acl::acl_cpp_init();
	acl::string addr("127.0.0.1:9206");
	acl::log::stdout_open(true);

	acl::server_socket ss;
	if (ss.open(addr) == false) {
		printf("listen %s error %s\r\n", addr.c_str(), acl::last_serror());
		return 1;
	}
	printf("listen %s ok\r\n", addr.c_str());

	thread_server thr(ss);
	thr.start();		// start listen thread
	thr.wait();
	return 0;
}
