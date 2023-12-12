#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <acl_cpp/lib_acl.hpp>

static void usage(const char *proc) {
	printf("usage: %s -u https_url -s path_to_ssl -c path_to_crypto\r\n", proc);
}

int main(int argc, char *argv[]) {
	int ch;
	acl::string url = "https://www.baidu.com/";
#ifdef __linux__
	acl::string libssl = "/usr/local/lib64/libssl.so";
	acl::string libcrypto = "/usr/local/lib64/libcrypto.so";
#elif	defined(__APPLE__)
	acl::string libssl = "/usr/local/lib/libssl.dylib";
	acl::string libcrypto = "/usr/local/lib/libcrypto.dylib";
#else
# error "Not support!"
#endif

	acl::log::stdout_open(true);

	while ((ch = getopt(argc, argv, "hu:s:c:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'u':
			url = optarg;
			break;
		case 's':
			libssl = optarg;
			break;
		case 'c':
			libcrypto = optarg;
			break;
		default:
			break;
		}
	}

	acl::istream::set_rbuf_size(81920);

	// Load crypto and ssl libs.
	acl::openssl_conf::set_libpath(libcrypto, libssl);
	if (!acl::openssl_conf::load()) {
		printf("Load ssl failed, libssl=%s, libcrypto=%s\r\n",
			libssl.c_str(), libcrypto.c_str());
		return 1;
	}

	// SSL in client mode.
	acl::openssl_conf ssl_conf(false);

	acl::http_request req(url);
	// Setup SSL conf in request.
	req.set_ssl(&ssl_conf);

	acl::string buf;
	acl::http_header& hdr = req.request_header();
	hdr.accept_gzip(true);

	hdr.build_request(buf);
	printf("request header:\r\n%s\r\n", buf.c_str());

	// Send https request.
	if (!req.request(NULL, 0)) {
		printf("request error!\r\n");
		return 1;
	}

	acl::http_client* conn = req.get_client();
	conn->print_header("response header");

	// Read response from https server.
	acl::string body;

#if 1
	if (!req.get_body(body)) {
		printf("get_body error!\r\n");
		return 1;
	}
	if (body.size() > 10240) {
		printf("Total read: %zd\r\n", body.size());
	} else {
		printf("%s\r\n", body.c_str());
	}
#else
	size_t total = 0;
	while (true) {
		int ret = req.read_body(body);
		if (ret <= 0) {
			break;
		}

		total += body.size();
		//printf("read %zd, total: %zd\r\n", body.size(), total);
		body.clear();
	}
	printf("Total read=%zd\r\n", total);
#endif

	return 0;
}
