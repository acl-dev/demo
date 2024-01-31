#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class client_read_callback : public acl::aio_callback {
public:
	client_read_callback(acl::aio_socket_stream* conn) : conn_(conn) {}

private:
	~client_read_callback() {}

protected:
	// @override
	bool read_callback(char* data, int len) {
		conn_->write(data, len);
		return true;
	}

	// @override
	void close_callback() {
		printf("Disconnected from client\r\n");
		delete this;
	}

private:
	acl::aio_socket_stream* conn_;
};

class transfer_callback : public acl::aio_open_callback {
public:
	transfer_callback(acl::aio_handle& handle,
		acl::aio_socket_stream* dispatch)
	: handle_(handle), dispatch_(dispatch) {}

private:
	~transfer_callback() {}

protected:
	// @override
	bool open_callback() {
		printf("Connect dispatch ok\r\n");
		dispatch_->add_read_callback(this);
		dispatch_->readable_await();
		return true;
	}

	// @override
	void close_callback() {
		printf("Disconnect from dispatch and exit now\r\n");
		handle_.stop();
		delete this;
	}

	// @override
	bool read_wakeup() {
		char buf[256];
		int cfd;

		int ret = acl_read_fd(dispatch_->sock_handle(),
			buf, (int) sizeof(buf) - 1, &cfd);
		if (ret <= 0 || cfd < 0) {
			printf("Read from dispatch error %s\r\n", acl::last_serror());
			return false;
		}

		buf[ret] = 0;
		printf("Get one client from addr %s\r\n", buf);

		acl::aio_socket_stream* conn = new acl::aio_socket_stream(&handle_, cfd);
		client_read_callback* handler = new client_read_callback(conn);
		conn->add_close_callback(handler);
		conn->add_read_callback(handler);
		conn->read_await();
		return true;
	}

private:
	acl::aio_handle& handle_;
	acl::aio_socket_stream* dispatch_;
};

static void usage(const char* procname) {
	printf("usage: %s -h [help] -s addr -e event_type[kernel|select|poll]\r\n", procname);
}

int main(int argc, char* argv[]) {
	int ch;
	acl::string server_addr("./dispatch.sock"), event_type("kernel");

	while ((ch = getopt(argc, argv, "hs:e:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			server_addr = optarg;
			break;
		case 'e':
			event_type = optarg;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	acl::aio_handle_type type;
	if (event_type == "kernel") {
		type = acl::ENGINE_KERNEL;
	} else if (event_type == "select") {
		type = acl::ENGINE_SELECT;
	} else if (event_type == "poll") {
		type = acl::ENGINE_POLL;
	} else {
		printf("invalid event_type %s\r\n", event_type.c_str());
		return 1;
	}

	acl::aio_handle handle(type);

	acl::aio_socket_stream* conn = acl::aio_socket_stream::open(&handle, server_addr, 5);
	if (conn == NULL) {
		printf("Connect %s error %s\r\n", server_addr.c_str(), acl::last_serror());
		return 1;
	}

	transfer_callback* handler = new transfer_callback(handle, conn);
	conn->add_close_callback(handler);
	conn->add_open_callback(handler);

	while (handle.check()) {}

	return 0;
}
