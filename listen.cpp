#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

int main(void)
{
	acl::log::stdout_open(true);
	const char* addr = "127.0.0.1:9999";
	int fd = acl_inet_listen(addr, 100, ACL_INET_FLAG_REUSEPORT);
	if (fd < 0)
		printf("listen error %s\r\n", acl::last_serror());
	int fd2 = acl_inet_listen(addr, 100, ACL_INET_FLAG_REUSEPORT);
	if (fd2 < 0)
		printf("listen error %s\r\n", acl::last_serror());

	acl::server_socket ss;
	for (int i = 0; i < 100; i++) {
		ss.open(addr);
		if (ss.open(addr) == false) {
			printf("listen %s error %s\r\n", addr, acl::last_serror());
			break;
		}
		else
			printf("listen %s\r\n", addr);
		ss.close();
	}

	return 0;
}
