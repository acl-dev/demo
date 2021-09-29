#include <thread>
#include <acl-lib/acl_cpp/lib_acl.hpp>

#define MB	(1024 * 1024)

static void echo(acl::socket_stream* conn, size_t dlen) {
	long long len = 0;
	char* buf = new char[dlen];
	while (!conn->eof()) {
		int ret = conn->read(buf, dlen, false);
		if (ret == -1) {
			break;
		}
		len += ret;

		if (len > 0 && (len % (100 * MB)) == 0) {
			char info[64];
			snprintf(info, sizeof(info), "size=%lld", len);
			acl::meter_time(__func__, __LINE__, info);
		}
	}

	delete []buf;
	delete conn;
}

int main(int argc, char* argv[]) {
	acl::acl_cpp_init();
	const char* addr = "0.0.0.0|9101";
	acl::log::stdout_open(true);

	size_t len = 8192;
	if (argc >= 2) {
		len = (size_t) atol(argv[1]);
	}
	if (len == 0 || len > 10240000) {
		len = 10240000;
	}

	acl::server_socket ss;
	if (!ss.open(addr)) {
		printf("listen %s error %s\r\n", addr, acl::last_serror());
		return 1;
	}

	while (true) {
		acl::socket_stream* conn = ss.accept();
		if (conn == NULL) {
			printf("accept error %s\r\n", acl::last_serror());
			break;
		}
		std::thread thread(echo, conn, len);
		thread.detach();
	}

	return 0;
}
