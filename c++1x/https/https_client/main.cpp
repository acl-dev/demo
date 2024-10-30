#include "stdafx.h"

static void https_get(acl::sslbase_conf& ssl_conf, const char* url)
{
    acl::http_request req(url, 10, 10);
    req.set_ssl(&ssl_conf);
    req.request_header().accept_gzip(true);

    if (!req.request(nullptr, 0)) {
        printf("Send request error\r\n");
        return;
    }

    acl::string buf;

    if (!req.get_body(buf)) {
        printf("Get response body error!\r\n");
        return;
    }

    printf("%s", buf.c_str());
    fflush(stdout);
}

static void usage(const char* procname)
{
	printf("usage: %s -h [help]\r\n"
		" -l ssl_lib_path\r\n"
		" -u url\r\n"
        " -D [if in debug mode]\r\n"
		, procname);
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
	acl::string libpath("/usr/local/lib/libcrypto.dylib; /usr/local/lib/libssl.dylib");
#else
	acl::string libpath("/usr/local/lib64/libcrypto.so; /usr/local/lib64/libssl.so");
#endif
	int  ch;
	acl::sslbase_conf* ssl_conf;
	acl::string url("https://www.baidu.com/");

	acl::acl_cpp_init();

	while ((ch = getopt(argc, argv, "hl:u:D")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
        case 'D':
	        acl::log::stdout_open(true);
            break;
		case 'l':
			libpath = optarg;
			break;
		case 'u':
            url = optarg;
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

    go[ssl_conf, &url] {
        https_get(*ssl_conf,url);
    };

    acl::fiber::schedule();

	delete ssl_conf;
	return 0;
}
