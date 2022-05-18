#include <thread>
#include <acl-lib/acl_cpp/lib_acl.hpp>

bool run(void) {
	const char* addr = "127.0.0.1:8088";
	acl::server_socket server;
	if (!server.open(addr)) {
		return false;
	}
	while (true) {
		acl::socket_stream* conn = server.accept();
		if (conn == NULL) {
			break;
		}

		std::thread thread([=] {
			char buf[256];
			int ret = conn->read(buf, sizeof(buf), false);
			if (ret > 0) {
				conn->write(buf, ret);
			}
			delete conn;
		});
		thread.detach();
	}

	return true;
}

int main(void) {
	run();
	return 0;
}
