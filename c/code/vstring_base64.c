#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <acl-lib/acl/lib_acl.h>

int main(int argc, char *argv[]) {
	const char *s = "hello world, hello world, hello world!\r\n";
	ACL_VSTRING *buf = acl_vstring_alloc(128);
	ACL_VSTRING *buf2 = acl_vstring_alloc(128);
	int i, max = 1000, len = (int) strlen(s);

	if (argc >= 2) {
		max = atoi(argv[1]);
	}

	for (i = 0; i < max; i++) {
		if (acl_vstring_base64_encode(buf, s, len) == NULL) {
			printf("base64 encode error!\r\n");
			exit(1);
		}
		if (i == 0) {
			printf("result: %s\r\n", acl_vstring_str(buf));
		}
	}

	for (i = 0; i < max; i++) {
		if (acl_vstring_base64_decode(buf2, acl_vstring_str(buf),
			(int) ACL_VSTRING_LEN(buf)) == NULL) {
			printf("base64 decode error!\r\n");
			exit(1);
		}
		if (i == 0) {
			printf("plain: %s", acl_vstring_str(buf2));
		}
	}

	acl_vstring_free(buf);
	acl_vstring_free(buf2);

	printf("Enter any key to exit ...");
	fflush(stdout);
	getchar();

	return 0;
}
