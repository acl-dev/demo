#include <acl-lib/acl/lib_acl.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

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
	return (0);
}
