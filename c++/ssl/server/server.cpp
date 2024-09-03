#include "stdafx.h"

static acl::sslbase_conf* ssl_conf = NULL;

class thread_client : public acl::thread {
public:
	thread_client(acl::socket_stream *conn) : conn_(conn) {}

protected:
	void *run() {
		if (!ssl_handshake()) {
			delete this;
			return NULL;
		}

		char buf[8192];
		while (true) {
			int ret = conn_->read(buf, sizeof(buf) - 1, false);
			if (ret == -1) {
				break;
			}
			if (conn_->write(buf, (size_t) ret) != ret) {
				break;
			}
		}

		delete this;
		return NULL;
	}

private:
	acl::socket_stream* conn_;

	~thread_client() {
		delete conn_;
	}

	bool ssl_handshake() {
		acl::sslbase_io *ssl = ssl_conf->create(false);
		if (conn_->setup_hook(ssl) == ssl) {
			printf("setup_hook ssl error\r\n");
			return false;
		}

		if (!ssl->handshake()) {
			printf("ssl ssl_handshake error!\r\n");
			ssl->destroy();
			return false;
		}
		if (!ssl->handshake_ok()) {
			printf("ssl handshake error\r\n");
			ssl->destroy();
			return false;
		}

		printf("ssl handshake ok!\r\n");
		return true;
	}
};

int main() {
	const char *libcrypto = "/usr/local/lib64/libcrypto.so";
	const char *libssl = "/usr/local/lib64/libssl.so";
	acl::openssl_conf::set_libpath(libcrypto, libssl);

	if (!acl::openssl_conf::load()) {
		printf("load openssl error, crypto=%s, ssl=%s\r\n", libcrypto, libssl);
		return 1;
	}
	printf("Load ssl so ok!\r\n");

	ssl_conf = new acl::openssl_conf(true);
	const char *crt_file = "./ssl_crt.pem", *key_file = "./ssl_key.pem";
	if (!ssl_conf->add_cert(crt_file, key_file)) {
		printf("add_cert error, crt=%s, key=%s\r\n", crt_file, key_file);
		delete ssl_conf;
		return 1;
	}
	printf("Load ssl cert ok!\r\n");

	const char *addr = "0.0.0.0:8288";
	acl::server_socket ss;
	if (!ss.open(addr)) {
		delete ssl_conf;
		printf("Open on %s error %s\r\n", addr, acl::last_serror());
		return 1;
	}

	printf("Open %s ok\r\n", addr);
	while (true) {
		acl::socket_stream* conn = ss.accept();
		if (conn == NULL) {
			printf("Accept error %s\r\n", acl::last_serror());
			break;
		}

		acl::thread* thr = new thread_client(conn);
		thr->set_detachable(true);
		thr->start();
	}

	delete ssl_conf;
	return 0;
}
