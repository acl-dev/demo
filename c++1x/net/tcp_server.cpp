#include <thread>
#include <acl-lib/acl_cpp/lib_acl.hpp>

#define MB	(1024 * 1024)

static void echo(acl::socket_stream* conn) {
	long long len = 0;
	char buf[8192];
	while (!conn->eof()) {
		int ret = conn->read(buf, sizeof(buf), false);
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

	delete conn;
}

int main(void) {
	acl::acl_cpp_init();
	const char* addr = "0.0.0.0|9101";
	acl::log::stdout_open(true);

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
		std::thread thread(echo, conn);
		thread.detach();
	}

	return 0;
}
