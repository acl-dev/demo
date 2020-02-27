#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <getopt.h>

class master_service : public acl::master_threads
{
public:
	master_service(void) {}
	~master_service(void) {}

private:
	// @override
	bool thread_on_accept(acl::socket_stream* conn)
	{
		printf("accept from %s\r\n", conn->get_peer());
		conn->set_rw_timeout(30);
		return true;
	}

	// @override
	bool thread_on_read(acl::socket_stream* conn)
	{
		acl::string buf;
		if (conn->gets(buf) == false)
			return false;
		else if (conn->puts(buf) == false)
			return false;
		return true;
	}

	// @override
	bool thread_on_timeout(acl::socket_stream* conn)
	{
		printf("read timeout from %s\r\n", conn->get_peer());
		return false;
	}

	// @override
	void thread_on_close(acl::socket_stream* conn)
	{
		printf("close from %s\r\n", conn->get_peer());
	}

	// @override
	void proc_on_init(void)
	{
		printf("process init now\r\n");
	}
};

int main(int argc, char* argv[])
{
	acl::string addrs("127.0.0.1:9001, 9002");
	bool daemon_mode = false;
	int ch;

	while ((ch = getopt(argc, argv, "s:D")) > 0) {
		switch (ch) {
		case 's':
			addrs = optarg;
			break;
		case 'D':
			daemon_mode = true;
			break;
		default:
			break;
		}
	}
	master_service ms;
	if (daemon_mode) {
		ms.run_daemon(argc, argv);
	} else {
		printf("run_alone mode, addrs: %s\r\n", addrs.c_str());
		ms.run_alone(addrs, NULL, 0, 100);
	}
	return 0;
}
