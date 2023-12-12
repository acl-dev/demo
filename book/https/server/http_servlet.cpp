#include "stdafx.h"
#include "http_servlet.h"

http_servlet::http_servlet(acl::socket_stream* stream, acl::session* session)
: HttpServlet(stream, session)
{
}

http_servlet::~http_servlet()
{
}

bool http_servlet::doGet(acl::HttpServletRequest& req, acl::HttpServletResponse& res)
{
	return doPost(req, res);
}

bool http_servlet::doPost(acl::HttpServletRequest& req, acl::HttpServletResponse& res)
{
	status_.clear();
	x_.clear();

	acl::json* json = req.getJson();
	if (json == NULL) {
		logger_error("No json got!");
		return false;
	}

	if (!parse_json(*json)) {
		logger_error("Invalid json=%s", json->to_string().c_str());
		return false;
	}

	acl::string buf;
	build_json(buf);

	res.setContentLength(buf.size());
	res.setKeepAlive(req.isKeepAlive());

	printf("Read: %s\r\n", json->to_string().c_str());
	printf("Send: %s\r\n", buf.c_str());
	printf("\r\n");

	// ·¢ËÍ http ÏìÓ¦Ìå
	return res.write(buf, buf.size()) && res.write(NULL, 0) && req.isKeepAlive();
}

bool http_servlet::parse_json(acl::json& json)
{
	acl::json_node& root = json.get_root();
	acl::json_node* child = root.first_child();
	while (child) {
		const char* tag = child->tag_name();
		if (tag == NULL || *tag == 0) {
			child = root.next_child();
			continue;
		}

		if (strcasecmp(tag, "status") == 0) {
			const char* ptr = child->get_text();
			if (ptr && *ptr) {
				status_ = ptr;
			}
		} else if (strcasecmp(tag, "x") == 0) {
			const char* ptr = child->get_text();
			if (ptr && *ptr) {
				x_ = ptr;
			}
		}

		child = root.next_child();
	}

	if (status_.empty()  || x_.empty()) {
		return false;
	}

	return true;
}

void http_servlet::build_json(acl::string& buf)
{
	int status = atoi(status_.c_str());
	status++;
	status_.format("%d", status);

	int x = atoi(x_.c_str());
	x += 10;
	x_.format("%d", x);

	acl::json json;
	acl::json_node& root = json.get_root();
	root.add_text("status", status_);
	root.add_text("x", x_);

	(void) json.to_string(&buf);
}
