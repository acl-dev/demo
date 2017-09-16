#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/lib_fiber.hpp>

class tcp_fiber : public acl::fiber
{
public:
	tcp_fiber(acl::socket_stream* conn) : conn_(conn) {}

private:
	acl::socket_stream* conn_;

	~tcp_fiber(void)
	{
		delete conn_;
	}

	// @override
	void run(void)
	{
		int n, i = 0;
		acl::string buf;
		while (!conn_->eof()) {
			if (conn_->gets(buf) == false)
				break;
			n = atoi(buf.c_str());
			if (n != i) {
				printf("invalid read: %d, i=%d\r\n", n, i);
				break;
			}
			i++;

			if (conn_->puts(buf) == -1) {
				printf("write error %s\r\n", acl::last_serror());
				break;
			}
		}

		delete this;
	}
};

class consumer_fiber : public acl::fiber
{
public:
	consumer_fiber(acl::mbox<acl::socket_stream>& mbox) : mbox_(mbox) {}
	~consumer_fiber(void) {}

private:
	acl::mbox<acl::socket_stream>& mbox_;

	// @override
	void run(void)
	{
		while (true) {
			acl::socket_stream* conn = mbox_.pop();
			if (conn == NULL)
				continue;

			printf("thread-%lu: accept one, fd=%d\r\n",
				acl::thread::thread_self(),
				conn->sock_handle());
			acl::fiber* fb = new tcp_fiber(conn);
			fb->start();
		}
	}
};

class consumer_thread : public acl::thread
{
public:
	consumer_thread(acl::mbox<acl::socket_stream>& mbox) : mbox_(mbox) {}
	~consumer_thread(void) {}

private:
	acl::mbox<acl::socket_stream> mbox_;

	// @override
	void* run(void)
	{
		consumer_fiber fiber(mbox_);
		fiber.start();

		acl::fiber::schedule();
		return NULL;
	}
};

int main(int argc, char* argv[])
{
	acl::string addr("127.0.0.1:9001");
	int rw_timeout = -1;

	if (argc >= 2)
		addr = argv[1];
	if (argc >= 3)
		rw_timeout = atoi(argv[2]);

	acl::server_socket server;
	if (server.open(addr) == false) {
		printf("listen %s error %s\r\n",
			addr.c_str(), acl::last_serror());
		return 0;
	}
	printf("open %s ok, rw_timeout: %d\r\n", addr.c_str(), rw_timeout);

	acl::mbox<acl::socket_stream> mbox1, mbox2;
	consumer_thread thr1(mbox1), thr2(mbox2);
	thr1.start();
	thr2.start();

	int i = 0;
	while (true) {
		acl::socket_stream* conn = server.accept();
		if (conn == NULL) {
			printf("accept error %s\r\n", acl::last_serror());
			break;
		}

		conn->set_rw_timeout(rw_timeout);
		if (i++ % 2 == 0)
			mbox1.push(conn);
		else
			mbox2.push(conn);
	}

	return  0;
}
