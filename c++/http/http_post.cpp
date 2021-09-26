#include <acl-lib/acl_cpp/lib_acl.hpp>

int main(void) {
	acl::url_coder coder;
	coder.set("name1", "value1")
		.set("name2", "value2");
	acl::string body(coder.to_string());

	acl::http_request req("www.baidu.com:80");
	acl::http_header& hdr = req.request_header();
	hdr.set_url("/")
		.set_method(acl::HTTP_METHOD_POST)
		.set_content_type("application/x-www-form-urlencoded")
		.set_content_length(body.size());

	if (!req.request(body, body.size())) {
		printf("request error\r\n");
		return 1;
	}

	printf("-------------------------------------------------------\r\n");
	acl::string buf;
	hdr.build_request(buf);
	printf("[%s]\r\n", buf.c_str());
	printf("[%s]\r\n", body.c_str());
	printf("-------------------------------------------------------\r\n");

	buf.clear();
	acl::http_client* conn = req.get_client();
	conn->sprint_header(buf);
	printf("[%s]\r\n", buf.c_str());

	body.clear();
	if (!req.get_body(body)) {
		printf("get_body error!\r\n");
		return 1;
	}

	printf("[%s]\r\n", body.c_str());
	return 0;
}
