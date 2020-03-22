#include <vector>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class http_thread : public acl::thread {
public:
	http_thread(acl::http_request_pool& pool) : pool_(pool) {}
	~http_thread(void) {}

protected:
	void* run(void) {
		for (int i = 0; i < 2; i++) {
			acl::http_request* conn = (acl::http_request*)
				pool_.peek();
			if (conn == NULL) {
				break;
			}
			if (http_get(*conn)) {
				pool_.put(conn, true);
			} else {
				pool_.put(conn, false);
			}
		}
		return NULL;
	}
private:
	acl::http_request_pool& pool_;

	bool http_get(acl::http_request& conn) {
		acl::http_header& hdr = conn.request_header();
		hdr.set_url("/")
			.set_keep_alive(true)
			.set_host("www.baidu.com")
			.accept_gzip(true);
		if (conn.request(NULL, 0) == false) {
			return false;
		}

		acl::string body;
		if (conn.get_body(body) == false) {
			return false;
		}
		printf("got body length: %ld\r\n", (long) body.size());
		return true;
	}
};

int main(void) {
	acl::http_request_pool pool("www.baidu.com|80", 10);

	std::vector<acl::thread*> threads;
	for (int i = 0; i < 5; i++) {
		acl::thread* thr = new http_thread(pool);
		thr->start();
		threads.push_back(thr);
	}

	for (std::vector<acl::thread*>::iterator it = threads.begin();
		it != threads.end(); ++it) {
		(*it)->wait(NULL);
		delete *it;
	}
	return 0;
}
