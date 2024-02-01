#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class dispatch_service;

class dispatch_checker : public acl::fiber {
public:
	dispatch_checker(dispatch_service& dispatch, acl::socket_stream* conn)
	: dispatch_(dispatch), conn_(conn) {}

protected:
	~dispatch_checker() {}

	// @override
	void run();

private:
	dispatch_service& dispatch_;
	acl::socket_stream* conn_;
};

class dispatch_service : public acl::fiber {
public:
	dispatch_service(acl::server_socket& ss) : ss_(ss) {}
	~dispatch_service() {}

	bool transfer(acl::socket_stream& conn) {
		if (dispatch_conns_.empty()) {
			printf("No server available!\r\n");
			return false;
		}

		if (it_ == dispatch_conns_.end()) {
			it_ = dispatch_conns_.begin();
		}

		const char* peer = conn.get_peer(true);
		int ret = acl_write_fd((*it_)->sock_handle(), (void*) peer,
			(int) strlen(peer), conn.sock_handle());

		printf(">>>Transfer ret=%d, addr=%s, fd=%d\r\n",
			ret, peer, conn.sock_handle());

		++it_;

		return true;
	}

	void remove(acl::socket_stream* conn) {
		dispatch_conns_.erase(conn);
		it_ = dispatch_conns_.begin();
	}

protected:
	// @over
	void run() {
		while (true) {
			acl::socket_stream* conn = ss_.accept();
			if (conn) {
				dispatch_conns_.insert(conn);
				it_ = dispatch_conns_.begin();

				printf("Got one server connection from %s\r\n", conn->get_peer(true));
				acl::fiber* fb = new dispatch_checker(*this, conn);
				fb->start();
			}
		}
	}

private:
	acl::server_socket& ss_;
	std::set<acl::socket_stream*> dispatch_conns_;
	std::set<acl::socket_stream*>::iterator it_;
};

// Wait for the dispatch connection be readable.
void dispatch_checker::run() {
	char buf[128];
	(void) conn_->read(buf, sizeof(buf) - 1, false);
	dispatch_.remove(conn_);
	delete conn_;
	delete this;
}

///////////////////////////////////////////////////////////////////////////////

class accept_service : public acl::fiber {
public:
	accept_service(acl::server_socket& ss, dispatch_service& dispatch)
	: ss_(ss), dispatch_(dispatch) {}
	~accept_service() {}

protected:
	// @override
	void run() {
		while (true) {
			acl::socket_stream* conn = ss_.accept();
			if (conn) {
				dispatch_.transfer(*conn);
				delete conn;
			}
		}
	}

private:
	acl::server_socket& ss_;
	dispatch_service& dispatch_;
};

static void usage(const char* procname) {
	printf("usage: %s -h [help] -s addr\r\n", procname);
}

int main(int argc, char* argv[]) {
	int ch;
	acl::string listen_addr("127.0.0.1|9201");
	acl::string dispatch_addr("./dispatch.sock");

	while ((ch = getopt(argc, argv, "hs:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			listen_addr = optarg;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}


	//////////////////////////////////////////////////////////////////////

	acl::server_socket dispatcher;
	if (!dispatcher.open(dispatch_addr)) {
		printf("listen on %s error %s\r\n", dispatch_addr.c_str(),
			acl::last_serror());
		return 1;
	}
	printf("listen on %s ok\r\n", dispatch_addr.c_str());

	dispatch_service fbd(dispatcher);
	fbd.start();

	//////////////////////////////////////////////////////////////////////

	acl::server_socket accepter;
	if (!accepter.open(listen_addr)) {
		printf("listen on %s error %s\r\n", listen_addr.c_str(),
			acl::last_serror());
		return 1;
	}
	printf("listen on %s ok\r\n", listen_addr.c_str());

	accept_service fba(accepter, fbd);
	fba.start();

	//////////////////////////////////////////////////////////////////////

	acl::fiber::schedule();

	return 0;
}
