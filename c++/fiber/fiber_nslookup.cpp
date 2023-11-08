#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <acl-lib/fiber/libfiber.hpp>

class ns_lookup : public acl::fiber {
public:
	ns_lookup(const char* name) : name_(name) {}

private:
	~ns_lookup(void) {}

private:
	std::string name_;

	// @override
	void run(void) {
		struct hostent *ent = gethostbyname(name_.c_str());
		if (ent == NULL) {
			printf("gethostbyname error=%s, name=%s\r\n",
				hstrerror(h_errno), name_.c_str());
			delete this;
			return;
		}

		printf("h_name=%s, h_length=%d, h_addrtype=%d\r\n",
			ent->h_name, ent->h_length, ent->h_addrtype);
		for (int i = 0; ent->h_addr_list[i]; i++) {
			char *addr = ent->h_addr_list[i];
			char  ip[64];
			const char *ptr;

			ptr = inet_ntop(ent->h_addrtype, addr, ip, sizeof(ip));
			printf(">>>addr: %s\r\n", ptr);
		}

		delete this;
	}
};

int main(void) {
	const char* name1 = "www.google.com", *name2 = "www.baidu.com",
		*name3 = "zsx.xsz.zzz";

	acl::fiber* fb = new ns_lookup(name1);
	fb->start();

	fb = new ns_lookup(name2);
	fb->start();

	fb = new ns_lookup(name3);
	fb->start();

	acl::fiber::schedule();
	return 0;
}
