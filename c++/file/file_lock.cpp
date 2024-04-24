#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

int main(void) {
	const char* filepath = "file.dummy";
	acl::fstream fp;

	if (fp.open(filepath, O_RDWR | O_CREAT, 0600) == false) {
		printf("open %s error %s\r\n", filepath, acl::last_serror());
		return 1;
	}

	if (!fp.try_lock()) {
		printf("lock %s error=%s\r\n", filepath, acl::last_serror());
		return 1;
	}

	printf("lock %s ok\r\n", filepath);

	acl::ifstream fp2;
	fp2.open_read(filepath);
	fp2.close();

	sleep(5);

	if (!fp.unlock()) {
		printf("unlock %s error %s\r\n", filepath, acl::last_serror());
		return 1;
	}

	printf("unlock %s ok\r\n", filepath);
	return 0;
}
