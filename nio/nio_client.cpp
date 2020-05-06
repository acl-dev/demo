#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class client_handler : public acl::aio_open_callback {
public:
	client_handler(acl::aio_socket_stream* conn,
		int cocurrent, int& ndisconnect, int nloop, int& nread)
	: conn_(conn)
	, connected_(false)
	, cocurrent_(cocurrent)
	, ndisconnect_(ndisconnect)
	, nloop_(nloop)
	, nread_(nread)
	, i_(0) {}

private:
	~client_handler(void) {}

protected:
	// @override
	bool timeout_callback(void) {
		if (connected_) {
			printf("read timeout\r\n");
		} else {
			printf("connect timeout\r\n");
		}
		return false;
	}

	// @override 
	bool open_callback(void) {
		connected_ = true;
		conn_->add_read_callback(this);

		buf_.format("hello-%d\r\n", i_++);
		conn_->write(buf_.c_str(), (int) buf_.size());
		conn_->gets(5, false);
		return true;
	}

	// @override
	bool read_callback(char* data, int) {
		nread_++;
		if (nread_ <= 10) {
			printf(">>data=%s", data);
		}

		if (i_ >= nloop_) {
			conn_->format("quit\r\n");
			conn_->close();
			return false;
		}
		buf_.format("hello=%d\r\n", i_++);
		conn_->write(buf_, (int) buf_.size());
		return true;
	}

	// @override
	void close_callback(void) {
		printf("disconnet from %s, fd=%d\r\n", conn_->get_peer(true),
			conn_->sock_handle());
		acl::aio_handle& handle = conn_->get_handle();
		if (++ndisconnect_ >= cocurrent_) {
			printf("ndisconnect=%d, cocurrent=%d, nread=%d\r\n",
				ndisconnect_, cocurrent_, nread_);
			handle.stop();
		}
		delete this;
	}

private:
	acl::aio_socket_stream* conn_;
	bool connected_;
	int  cocurrent_;
	int& ndisconnect_;
	int  nloop_;
	int& nread_;
	int  i_;
	acl::string buf_;
};

static void usage(const char* procname) {
	printf("usage: %s -h [help] -s addr -e event_type[kernel|select|poll]"
		" -N dns_addr -c cocurrent -n nloop\r\n", procname);
}

int main(int argc, char* argv[]) {
	int ch, cocurrent = 100, nloop = 100;
	acl::string listen_addr("127.0.0.1|9201"), event_type("kernel");
	acl::string dns_addr("8.8.8.8:53");

	while ((ch = getopt(argc, argv, "hs:e:N:c:n:")) > 0) {
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
		case 'N':
			dns_addr = optarg;
			break;
		case 'c':
			cocurrent = atoi(optarg);
			break;
		case 'n':
			nloop = atoi(optarg);
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
	if (!dns_addr.empty()) {
		handle.set_dns(dns_addr, 5);
	}

	int ndisconnect = 0, nread = 0;

	for (int i = 0; i < cocurrent; i++) {
		acl::aio_socket_stream* conn = acl::aio_socket_stream::open(
			&handle, listen_addr, 5);
		if (conn == NULL) {
			printf("connect %s error %s\r\n", listen_addr.c_str(),
				acl::last_serror());
			break;
		}

		acl::aio_open_callback* handler = new client_handler(conn,
			cocurrent, ndisconnect, nloop, nread);
		conn->add_open_callback(handler);
		conn->add_close_callback(handler);
		conn->add_timeout_callback(handler);
	}

	while (handle.check()) {}
	handle.check();

	return 0;

}
