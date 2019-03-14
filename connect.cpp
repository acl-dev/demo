#include <getopt.h>
#include <unistd.h>
#include <acl-lib/acl/lib_acl.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>

class fiber_transfer : public acl::fiber
{
public:
	fiber_transfer(acl::istream& in, acl::ostream& out)
	: in_(in), out_(out), peer_(NULL)
	{} 

	~fiber_transfer(void) {}

	void set_peer(fiber_transfer& peer)
	{
		peer_ = &peer;
	}

	void unset_peer(void)
	{
		peer_ = NULL;
	}

	void wait(void)
	{
		(void) box_.pop();
	}

protected:
	// @override
	void run(void)
	{
		char buf[8192];
		while (true) {
			int ret = in_.read(buf, sizeof(buf), false);
			if (ret == -1) {
				break;
			}
			if (out_.write(buf, ret) == -1) {
				break;
			}
		}

		if (peer_) {
			fiber_transfer* peer = peer_;
			peer_ = NULL;
			peer->unset_peer();
			peer->kill();
		}

		box_.push(NULL);
	}

private:
	acl::fiber_tbox<int> box_;
	acl::istream& in_;
	acl::ostream& out_;
	fiber_transfer* peer_;
};

class fiber_proxy : public acl::fiber
{
public:
	fiber_proxy(acl::socket_stream& conn) : conn_(conn) {}
	~fiber_proxy(void) {}

protected:
	// @override
	void run(void)
	{
		acl::stdin_stream  in;
		acl::stdout_stream out;

		ACL_VSTREAM* stream = in.get_vstream();
		acl_non_blocking(ACL_VSTREAM_SOCK(stream), ACL_NON_BLOCKING);

		fiber_transfer fiber_local(in, conn_);
		fiber_transfer fiber_peer(conn_, out);

		fiber_local.set_peer(fiber_peer);
		fiber_peer.set_peer(fiber_local);

		fiber_local.start();
		fiber_peer.start();

		fiber_local.wait();
		fiber_peer.wait();
		acl::fiber::schedule_stop();
	}

private:
	acl::socket_stream& conn_;
};

static void transfer(acl::socket_stream& conn)
{
	fiber_proxy fb(conn);
	fb.start();

	acl::fiber::schedule();
}

static void usage(const char* procname)
{
	printf("usage: %s -h [help] -P proxy_addr -R remote_addr\r\n", procname);
}

int main(int argc, char* argv[])
{
	acl::string proxy_addr, remote_addr;
	int ch;

	while ((ch = getopt(argc, argv, "hP:R:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'P':
			proxy_addr = optarg;
			break;
		case 'R':
			remote_addr = optarg;
			break;
		default:
			break;
		}
	}

	if (proxy_addr.empty() || remote_addr.empty()) {
		usage(argv[0]);
		return 1;
	}

	acl::socket_stream conn;

	if (conn.open(proxy_addr, 0, 0) == false) {
		printf("connect proxy(%s) error %s\r\n",
			proxy_addr.c_str(), acl::last_serror());
		return 1;
	}

#if 0
	acl::string buf;
	buf.format("CONNECT %s HTTP/1.1\r\n\r\n", remote_addr.c_str());
	if (conn.write(buf) == -1) {
		printf("write to %s error\r\n", remote_addr.c_str());
		return 1;
	}
#endif

	acl::http_request req(&conn);
	req.request_header().set_method(acl::HTTP_METHOD_CONNECT)
		.set_url(remote_addr)
		.set_host(remote_addr);
	if (req.request(NULL, 0) == false) {
		printf("send connect request failed\r\n");
		return 1;
	}

	int status = req.http_status();
	if (status != 200) {
		printf("invalid status=%d\r\n", status);
		return 1;
	}

	transfer(conn);
	return 0;
}
