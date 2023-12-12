#include "stdafx.h"

class http_servlet : public acl::HttpServlet
{
public:
	http_servlet(acl::socket_stream* stream, acl::session* session);
	~http_servlet(void);

protected:
	// override
	bool doGet(acl::HttpServletRequest& req, acl::HttpServletResponse& res);

	// override
	bool doPost(acl::HttpServletRequest&, acl::HttpServletResponse& res);

private:
	acl::string status_;
	acl::string x_;

	bool parse_json(acl::json& json);
	void build_json(acl::string& buf);
};
