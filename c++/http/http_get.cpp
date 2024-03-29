#include <acl-lib/acl_cpp/lib_acl.hpp>

int main(void) {
	acl::http_request req("www.baidu.com:80");
	acl::http_header& hdr = req.request_header();
	hdr.set_url("/s")
		.set_host("www.baidu.com")
		.add_param("ie", "utf-8")
		.add_int("f", 8)
		.add_int("rsv_bp", 1)
		.add_param("tn", "monline_4_dg")
		.add_param("wd", "freebsd");

	if (!req.request(NULL, 0)) {
		printf("request error!\r\n");
		return 1;
	}

	acl::string body;
	if (!req.get_body(body)) {
		printf("get_body error!\r\n");
		return 1;
	}

	printf("%s\r\n", body.c_str());

	//////////////////////////////////////////////////////////////////////

	printf("\r\n\r\nEnter any key to continue ...");
	fflush(stdout);
	getchar();

	acl::http_request req2("http://www.baidu.com");

	if (!req2.request(NULL, 0)) {
		printf("request error\r\n");
		return 1;
	}

	body.clear();
	if (!req2.get_body(body)) {
		printf("get_body error\r\n");
		return 1;
	}
	printf("ok: %s\r\n", body.c_str());

	//////////////////////////////////////////////////////////////////////

	printf("\r\n\r\nEnter any key to continue ...");
	fflush(stdout);
	getchar();

	acl::http_request req3("www.baidu.com");

	if (!req3.request(NULL, 0)) {
		printf("request error\r\n");
		return 1;
	}

	body.clear();
	if (!req3.get_body(body)) {
		printf("get_body error\r\n");
		return 1;
	}
	printf("ok: %s\r\n", body.c_str());
	return 0;
}
