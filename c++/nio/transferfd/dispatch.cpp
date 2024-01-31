#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class dispatch_accept_callback : public acl::aio_accept_callback {
public:
	dispatch_accept_callback(acl::aio_handle& handle) : handle_(handle) {}
	~dispatch_accept_callback() {}

	bool transfer(acl::aio_socket_stream& conn) {
		if (conns_.empty()) {
			printf("No server available!\r\n");
			return false;
		}

		if (it_ == conns_.end()) {
			it_ = conns_.begin();
		}

		const char* peer = conn.get_peer(true);
		acl_write_fd((*it_)->sock_handle(), (void*) peer,
			(int) strlen(peer), conn.sock_handle());

		++it_;

		return true;
	}

	void remove(acl::aio_socket_stream* conn) {
		conns_.erase(conn);
		it_ = conns_.begin();
	}


protected:
	// @override
	bool accept_callback(acl::aio_socket_stream* conn);

private:
	acl::aio_handle& handle_;
	std::set<acl::aio_socket_stream*> conns_;
	std::set<acl::aio_socket_stream*>::iterator it_;
};

///////////////////////////////////////////////////////////////////////////////

class dispatch_read_callback : public acl::aio_callback {
public:
	dispatch_read_callback(dispatch_accept_callback& dispatch,
		acl::aio_socket_stream* conn)
		: dispatch_(dispatch), conn_(conn) {}

private:
	~dispatch_read_callback() {}

protected:
	// @override
	bool read_wakeup() {
		return false; // return false to close the connection.
	}

	// @override
	void close_callback() {
		printf("Client disconnected!\r\n");
		dispatch_.remove(conn_);
		delete this;
	}

private:
	dispatch_accept_callback& dispatch_;
	acl::aio_socket_stream* conn_;
};

///////////////////////////////////////////////////////////////////////////////

bool dispatch_accept_callback::accept_callback(acl::aio_socket_stream* conn) {
	acl::aio_callback* handler = new dispatch_read_callback(*this, conn);
	conn->add_close_callback(handler);
	conn->add_read_callback(handler);
	conn->read_await();

	conns_.insert(conn);
	it_ = conns_.begin();

	return true;
}

///////////////////////////////////////////////////////////////////////////////

class service_accept_callback : public acl::aio_accept_callback {
public:
	service_accept_callback(acl::aio_handle& handle,
		dispatch_accept_callback& dispatch)
	: handle_(handle), dispatch_(dispatch) {}
	~service_accept_callback() {}

protected:
	// @override
	bool accept_callback(acl::aio_socket_stream* conn) {
		printf("connect from %s, fd=%d\r\n", conn->get_peer(true),
			conn->sock_handle());
		dispatch_.transfer(*conn);
		conn->close();
		return true;
	}

private:
	acl::aio_handle& handle_;
	dispatch_accept_callback& dispatch_;
};

static void usage(const char* procname) {
	printf("usage: %s -h [help] -s addr -e event_type[kernel|select|poll]\r\n", procname);
}

int main(int argc, char* argv[]) {
	int ch;
	acl::string listen_addr("127.0.0.1|9201"), event_type("kernel");
	acl::string dispatch_addr("./dispatch.sock");

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

	//////////////////////////////////////////////////////////////////////

	acl::aio_listen_stream* dispatch = new acl::aio_listen_stream(&handle);
	if (!dispatch->open(dispatch_addr)) {
		printf("listen on %s error %s\r\n", dispatch_addr.c_str(),
			acl::last_serror());
		dispatch->destroy();
		return 1;
	}
	printf("listen on %s ok\r\n", dispatch_addr.c_str());
	dispatch_accept_callback dispatch_callback(handle);
	dispatch->add_accept_callback(&dispatch_callback);

	//////////////////////////////////////////////////////////////////////

	acl::aio_listen_stream* service = new acl::aio_listen_stream(&handle);
	if (!service->open(listen_addr)) {
		printf("listen on %s error %s\r\n", listen_addr.c_str(),
			acl::last_serror());
		service->destroy();
		dispatch->destroy();
		return 1;
	}
	printf("listen on %s ok\r\n", listen_addr.c_str());

	service_accept_callback service_callback(handle, dispatch_callback);
	service->add_accept_callback(&service_callback);

	//////////////////////////////////////////////////////////////////////

	while (handle.check()) {}

	service->destroy();
	dispatch->destroy();

	return 0;
}
