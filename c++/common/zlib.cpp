#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

static int test_compress(const char* filename) {
	acl::string buf;

	if (!acl::ifstream::load(filename, buf)) {
		printf("load %s error %s\r\n", filename, acl::last_serror());
		return -1;
	}

	printf("Load from %s ok, len=%zd\r\n", filename, buf.size());

	acl::zlib_stream zstream;
	acl::string out, out2;

	if (!zstream.zlib_compress(buf.c_str(), buf.size(),
		&out, acl::zlib_best_compress)) {
		printf("zlib_compress error\r\n");
		return -1;
	}

	printf("After compress, len: %zd\r\n", out.length());

	if (!zstream.zlib_uncompress(out.c_str(), (int) out.length(), &out2)) {
		printf("zlib_uncompress error\r\n");
		return -1;
	}

	if (out2 != buf) {
		printf("zlib_uncompress failed\r\n");
	} else {
		printf("After uncompress, len=%zd, result: %s\r\n",
			out2.length(), out2.c_str());
	}

	return 0;
}

static void usage(const char* proc) {
	printf("usage: %s -h [help] -f filename\r\n", proc);
}

int main(int argc, char* argv[]) {
	int ch;
	acl::string filename("in.txt");

	while ((ch = getopt(argc, argv, "hf:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'f':
			filename = optarg;
			break;
		default:
			break;
		}
	}

	acl::log::stdout_open(true);
	return test_compress(filename);;
}
