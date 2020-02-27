#include <acl-lib/acl/lib_acl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <getopt.h>

void resolve2(const char* domain)
{
	char   **pptr;
	struct hostent *hptr;
	char   str[32];

	if ((hptr = gethostbyname(domain)) == NULL) {
		printf("gethostbyname error for host: %s\n",domain);
		return;
	}

	printf("official hostname:%s\n", hptr->h_name);
	for (pptr = hptr->h_aliases; *pptr != NULL; pptr++) {
		printf(" alias:%s\n",*pptr);
	}

	switch(hptr->h_addrtype) {
	case AF_INET:
	case AF_INET6:
		pptr=hptr->h_addr_list;
		for(; *pptr!=NULL; pptr++) {
			printf(" address:%s\n", inet_ntop(hptr->h_addrtype,
				*pptr, str, sizeof(str)));
		}
		printf(" first address: %s\n", inet_ntop(hptr->h_addrtype,
			hptr->h_addr, str, sizeof(str)));
		break;
	default:
		printf("unknown address type\n");
		break;
	}
}
static void resolve(const char* domain)
{
	ACL_ITER iter;
	int herr;
	ACL_DNS_DB* db = acl_gethostbyname(domain, &herr);

	printf("%s\r\n", domain);

	if (db == NULL) {
		printf("acl_gethostbyname %s error %s\r\n",
			domain, acl_netdb_strerror(herr));
		return;
	}

	acl_foreach(iter, db) {
		const ACL_HOSTNAME* hn = (const ACL_HOSTNAME*) iter.data;
		printf("    ip: %s, ttl: %d\r\n", hn->ip, hn->ttl);
	}

	acl_netdb_free(db);
}

static void usage(const char* procname)
{
	printf("usage: %s -h help -n domain\r\n", procname);
}

int main(int argc, char* argv[])
{
	int  ch;
	char buf[256];

	snprintf(buf, sizeof(buf), "www.baidu.com");

	while ((ch = getopt(argc, argv, "hn:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'n':
			snprintf(buf, sizeof(buf), "%s", optarg);
			break;
		default:
			break;
		}
	}

	resolve(buf);
	printf("\r\n");
	resolve2(buf);
	return (0);
}
