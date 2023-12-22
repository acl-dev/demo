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
	acl::string buf;
	if (!req.getBody(buf)) {
		return false;
	}

	res.setContentLength(buf.size());
	res.setKeepAlive(req.isKeepAlive());

	printf("Read: %s", buf.c_str());
	fflush(stdout);

	// ∑¢ÀÕ http œÏ”¶ÃÂ
	return res.write(buf, buf.size()) && res.write(NULL, 0) && req.isKeepAlive();
}
