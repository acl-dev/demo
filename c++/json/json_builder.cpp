#include <stdio.h>
#include <stdlib.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

static void test_build1() {
	acl::json json;
	acl::json_node& root = json.get_root();
	acl::json_node& a = root.add_array(true);
	a.add_child(false, true).add_text("name", "zsx");
	a.add_child(false, true).add_number("age", 100);
	a.add_child(false, true).add_bool("man", true);

	acl::string buf;
	json.to_string(&buf);
	printf("%s\r\n", buf.c_str());
}

static void test_build2() {
	acl::json json;
	acl::json_node& root = json.get_root();
	acl::json_node& a = root.add_array(true);
	a.add_child(false, true).add_text("name", "zsx").get_parent()
		.add_child(false, true).add_number("age", 100).get_parent()
		.add_child(false, true).add_bool("man", true);

	acl::string buf;
	json.to_string(&buf);
	printf("%s\r\n", buf.c_str());
}

static void test_build3() {
	acl::json json;
	acl::json_node& root = json.get_root();
	acl::json_node& a = root.add_array(true);
	a.add_child(json.create_node().add_text("name", "zsx"))
		.add_child(json.create_node().add_number("age", (long long) 100))
		.add_child(json.create_node().add_bool("man", true));

	acl::string buf;
	json.to_string(&buf);
	printf("%s\r\n", buf.c_str());
}

int main() {
	test_build1();
	test_build2();
	test_build3();
	return 0;
}
