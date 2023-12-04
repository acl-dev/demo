#include <stdio.h>
#include <stdlib.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

static void test_build1() {
	acl::json json;
	acl::json_node& root = json.get_root();
	acl::json_node& a = root.add_array(true);
	a.add_text("name", "zsx")
		.add_number("age", 100)
		.add_bool("man", true);

	acl::string buf;
	json.to_string(&buf);
	printf("%s\r\n", buf.c_str());
}

static void test_build2() {
	acl::json json;
	acl::json_node& root = json.get_root();
	acl::json_node& a = root.add_array(true);
	a.add_child(json.create_node("name", "zsx"))
		.add_child(json.create_node("age", (long long) 100))
		.add_child(json.create_node("man", true));

	acl::string buf;
	json.to_string(&buf);
	printf("%s\r\n", buf.c_str());
}

int main() {
	test_build1();
	test_build2();
	return 0;
}
