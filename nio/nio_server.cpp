#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class client_read_callback : public acl::aio_callback {
public:
	client_read_callback(acl::aio_socket_stream* conn) : conn_(conn) {}

private:
	~client_read_callback(void) {}

protected:
	// @override
	bool read_callback(char* data, int len) {
		if (strncmp(data, "quit", 4) == 0) {
			conn_->format("Bye!\r\n");
			conn_->close();
			return false;
		}

		conn_->write(data, len);
		return true;
	}

	// @override
	void close_callback(void) {
		printf("disconnect from %s, fd=%d\r\n", conn_->get_peer(true),
			conn_->sock_handle());
		delete this;
	}

private:
	acl::aio_socket_stream* conn_;
};

class server_accept_callback : public acl::aio_accept_callback {
public:
	server_accept_callback(acl::aio_listen_stream& listener)
	: listener_(listener) {}
	~server_accept_callback(void) {}

protected:
	// @override
	bool accept_callback(acl::aio_socket_stream* conn) {
		printf("connect from %s, fd=%d\r\n", conn->get_peer(true),
			conn->sock_handle());
		acl::aio_callback* handler = new client_read_callback(conn);
		conn->add_close_callback(handler);
		conn->add_read_callback(handler);
		conn->read();
		return true;
	}

private:
	acl::aio_listen_stream& listener_;
};

static void usage(const char* procname) {
	printf("usage: %s -h [help] -s addr -e event_type[kernel|select|poll]\r\n", procname);
}

int main(int argc, char* argv[]) {
	int ch;
	acl::string listen_addr("127.0.0.1|9201"), event_type("kernel");

	while ((ch = getopt(argc, argv, "hs:e:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			listen_addr = optarg;
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

	acl::aio_listen_stream* listener = new acl::aio_listen_stream(&handle);
	if (!listener->open(listen_addr)) {
		printf("listen %s error %s\r\n", listen_addr.c_str(),
			acl::last_serror());
		listener->destroy();
		return 1;
	}
	printf("listen on %s ok\r\n", listen_addr.c_str());

	server_accept_callback callback(*listener);
	listener->add_accept_callback(&callback);

	while (handle.check()) {}

	listener->destroy();
	return 0;
}
