#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class fiber_echo : public acl::fiber {
public:
	fiber_echo(acl::socket_stream* conn) : conn_(conn) {}

protected:
	~fiber_echo() { delete conn_; }

	// @override
	void run() {
		char buf[4096];
		while (true) {
			int ret = conn_->read(buf, sizeof(buf), false);
			if (ret <= 0) {
				break;
			}

			if (conn_->write(buf, (size_t) ret) != ret) {
				break;
			}
		}

		delete this;
	}

private:
	acl::socket_stream* conn_;
};

class fiber_server : public acl::fiber {
public:
	fiber_server() {}
	~fiber_server() {}

	bool open(const char* addr) {
		if (ss_.open(addr)) {
			return true;
		}

		printf("Listen on %s error %s\r\n", addr, acl::last_serror());
		return false;
	}

protected:
	// @override
	void run() {
		while (true) {
			acl::socket_stream* conn = ss_.accept();

			if (this->killed()) {
				printf("The listner fiber has been killed!\r\n");
				break;
			}

			if (conn == NULL) {
				printf("Accept error %s\r\n", acl::last_serror());
				break;
			}

			printf("Accept one client and start one echo fiber\r\n");
			acl::fiber* fb = new fiber_echo(conn);
			fb->start();
		}
	}

private:
	acl::server_socket ss_;
};

class fiber_killer : public acl::fiber {
public:
	fiber_killer(acl::fiber_tbox<bool>& box, acl::fiber& fb)
	: box_(box), fiber_(fb) {}
	~fiber_killer() {}

protected:
	// @override
	void run() {
		printf("The killer fiber is waiting message ...\r\n");
		(void) box_.pop();
		printf("Begin to kill the fiber-%d\r\n", fiber_.get_id());
		fiber_.kill();
		printf("Kill ok!\r\n");
	}

private:
	acl::fiber_tbox<bool>& box_;
	acl::fiber& fiber_;
};

class server_thread : public acl::thread {
public:
	server_thread(acl::fiber_tbox<bool>& box, const char* addr)
	: box_(box), addr_(addr) {}

	~server_thread() {}

protected:
	// @override
	void* run() {
		fiber_server server;
		if (!server.open(addr_)) {
			return NULL;
		}

		printf("Start server fiber listen on %s...\r\n", addr_.c_str());
		server.start();

		fiber_killer killer(box_, server);
		printf("Start killer fiber\r\n");
		killer.start();

		acl::fiber::schedule();
		return NULL;
	}

private:
	acl::fiber_tbox<bool>& box_;
	acl::string addr_;
};

int main(void) {
	const char* addr = "127.0.0.1:8388";

	acl::fiber_tbox<bool> box;
	server_thread thread(box, addr);
	thread.set_detachable(false);
	thread.start();

	sleep(1);

	// Notify the killer fiber to kill the server fiber.
	box.push(NULL);

	thread.wait();
	return 0;
}
