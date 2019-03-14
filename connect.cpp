#include <getopt.h>
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
			peer_->kill();
		}

		box_.push(NULL);
	}

private:
	acl::fiber_tbox<int> box_;
	acl::istream& in_;
	acl::ostream& out_;
	fiber_transfer* peer_;
};

static void transfer(acl::socket_stream& conn)
{
	acl::stdin_stream  in;
	acl::stdout_stream out;

	fiber_transfer fiber_local(in, conn);
	fiber_transfer fiber_peer(conn, out);

	fiber_local.set_peer(fiber_peer);
	fiber_peer.set_peer(fiber_local);

	fiber_local.start();
	fiber_peer.start();

	fiber_local.wait();
	fiber_peer.wait();
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
		acl::stdout_stream out;
		out.format("connect proxy(%s) error %s\r\n",
			proxy_addr.c_str(), acl::last_serror());
		return 1;
	}

	transfer(conn);
	return 0;
}
