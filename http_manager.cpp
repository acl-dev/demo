#include <getopt.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

static bool test(acl::http_request_pool& hpool, const char* url, bool keep_alive)
{
	acl::http_request* req = (acl::http_request*) hpool.peek();
	if (req == NULL) {
		printf("peek connection error\r\n");
		return false;
	}
	req->reset();

	acl::http_header& hdr = req->request_header();
	hdr.set_url(url).set_keep_alive(true).set_host(hpool.get_addr());

	acl::string buf;
	hdr.build_request(buf);
	printf("request:[%s]\r\n", buf.c_str());

	if (req->request(NULL, 0) == false) {
		printf("request error\r\n");
		hpool.put(req, false);
		return 1;
	}

	acl::http_client* conn = req->get_client();
	if (conn == NULL) {
		printf("conn NULL\r\n");
		return false;
	}

	conn->print_header();

	long long length = req->body_length();
	if (length > 0) {
		acl::string body;
		if (req->get_body(body) == false) {
			printf("get_body error\r\n");
			hpool.put(req, false);
			return false;
		}
	}
	printf("url=%s, addr=%s, ok, status=%d\r\n", url, hpool.get_addr(),
		req->http_status());
	hpool.put(req, keep_alive && req->keep_alive());

	return true;
}

static void usage(const char* procname)
{
	printf("usage: %s -h[help] -s addrs -u url -k [if keep_alive, default: false]\r\n", procname);
}

int main(int argc, char* argv[])
{
	int ch, count = 1;
	bool keep_alive = false;
	acl::string url;
	std::vector<acl::string> addrs;
	while ((ch = getopt(argc, argv, "hs:u:n:k")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			addrs.push_back(optarg);
			break;
		case 'u':
			url = optarg;
			break;
		case 'n':
			count = atoi(optarg);
			break;
		case 'k':
			keep_alive = true;
			break;
		default:
			break;
		}
	}

	if (url.empty()) {
		printf("no url\r\n");
		usage(argv[0]);
		return 1;
	}
	if (addrs.empty()) {
		printf("no addr\r\n");
		usage(argv[0]);
		return 1;
	}
	acl::http_request_manager manager;
	for (std::vector<acl::string>::const_iterator cit = addrs.begin();
		cit != addrs.end(); ++cit) {

		manager.set(*cit, 0);
	}

	for (int i = 0; i < count; i++) {
		acl::http_request_pool* hpool = (acl::http_request_pool*)
			manager.peek();
		if (hpool == NULL) {
			printf("peek http pool error\r\n");
			return 1;
		}
		struct timeval begin, end;
		gettimeofday(&begin, NULL);
		test(*hpool, url, keep_alive);
		gettimeofday(&end, NULL);
		printf("addr=%s, cost=%.2f ms\r\n", hpool->get_addr(),
			acl::stamp_sub(end, begin));
	}

	return 0;
}
