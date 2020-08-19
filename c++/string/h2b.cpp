#include <acl-lib/acl_cpp/lib_acl.hpp>

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("usage: %s text\r\n", argv[0]);
		return 1;
	}

	acl::string buf;
	acl::string encoded = buf.hex_encode(argv[1], strlen(argv[1]));
	printf("text=%s, encoded=%s\r\n", argv[1], encoded.c_str());

	buf.clear();
	acl::string& decoded = buf.hex_decode(encoded.c_str(), encoded.size());
	printf("decoded=%s\r\n", decoded.c_str());

	return 0;
}
