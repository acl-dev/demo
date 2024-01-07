#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>

#define	 STACK_SIZE	128000

static int __rw_timeout = 10;

static void start_test(const char* addr, acl::sslbase_conf& ssl_conf)
{
	acl::http_request req(addr);
	req.set_ssl(&ssl_conf);

	const char* data = "hello world!\r\n";
	int max = 4;
	acl::string buf;

	for (int i = 0; i < max; i++) {
		req.request_header().set_url("/")
			.set_keep_alive(true)
			.set_content_length((long long) strlen(data))
			.set_content_type("text/json")
			.set_host("test.com");

		if (!req.request(data, strlen(data))) {
			printf("Send request error\r\n");
			break;
		}

		buf.clear();
		if (!req.get_body(buf)) {
			printf("Get response body error!\r\n");
			break;
		}

		printf("Get: %s", buf.c_str());
		fflush(stdout);

		req.reset();
		buf.clear();
	}
}

static void usage(const char* procname)
{
	printf("usage: %s -h [help]\r\n"
		" -l ssl_lib_path\r\n"
		" -s listen_addr\r\n"
		" -r rw_timeout\r\n"
		, procname);
}

int main(int argc, char *argv[])
{
	acl::string addr("127.0.0.1:1900");
#ifdef __APPLE__
	acl::string libpath("/usr/local/lib/libcrypto.dylib; /usr/local/lib/libssl.dylib");
#else
	acl::string libpath("/usr/local/lib64/libcrypto.so; /usr/local/lib64/libssl.so");
#endif
	int  ch;
	acl::sslbase_conf* ssl_conf;

	acl::acl_cpp_init();
	acl::log::stdout_open(true);

	while ((ch = getopt(argc, argv, "hs:r:l:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			addr = optarg;
			break;
		case 'r':
			__rw_timeout = atoi(optarg);
			break;
		case 'l':
			libpath = optarg;
			break;
		default:
			break;
		}
	}

	if (libpath.find("crypto") != NULL) {
		const std::vector<acl::string>& libs = libpath.split2("; \t");
		if (libs.size() != 2) {
			printf("invalid libpath=%s\r\n", libpath.c_str());
			return 1;
		}
		acl::openssl_conf::set_libpath(libs[0], libs[1]);
		if (!acl::openssl_conf::load()) {
			printf("load %s error\r\n", libpath.c_str());
			return 1;
		}

		ssl_conf = new acl::openssl_conf(false);
	} else {
		printf("invalid ssl lib=%s\r\n", libpath.c_str());
		return 1;
	}

	start_test(addr, *ssl_conf);
	delete ssl_conf;
	return 0;
}
