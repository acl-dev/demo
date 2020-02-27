#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

int main(void)
{
	const char* filepath = "file.dummy";
	acl::fstream fp;

	if (fp.open(filepath, O_RDWR | O_CREAT, 0600) == false) {
		printf("open %s error %s\r\n", filepath, acl::last_serror());
		return 1;
	}

	printf("begin to lock %s\r\n", filepath);

	if (!fp.lock()) {
		printf("lock %s error=%s\r\n", filepath, acl::last_serror());
		return 1;
	}

	printf("lock %s ok\r\n", filepath);
	sleep(5);

	if (!fp.unlock()) {
		printf("unlock %s error %s\r\n", filepath, acl::last_serror());
		return 1;
	}

	printf("unlock %s ok\r\n", filepath);
	return 0;
}
