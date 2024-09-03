#include "stdafx.h"
#include "unistd.h"

static acl::sslbase_conf* ssl_conf = NULL;

static bool ssl_handshake(acl::socket_stream& conn) {
	acl::sslbase_io *ssl = ssl_conf->create(false);
	if (conn.setup_hook(ssl) == ssl) {
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

static bool run(const char *addr) {
	acl::socket_stream conn;
	if (!conn.open(addr, 10, 10)) {
		delete ssl_conf;
		printf("Open on %s error %s\r\n", addr, acl::last_serror());
		return false;
	}

	printf("Open %s ok\r\n", addr);

	if (!ssl_handshake(conn)) {
		return false;
	}

	if (conn.format("Hello world!\r\n") == -1) {
		printf("write error %s\r\n", acl::last_serror());
		return false;
	}

	char buf[8192];
	int ret = conn.read(buf, sizeof(buf) - 1, false);
	if (ret == -1) {
		printf("read error %s\r\n", acl::last_serror());
		return false;
	}

	buf[ret] = 0;
	printf("read ok: %s\r\n", buf);
	return true;
}

int main() {
	const char *libcrypto = "/usr/local/lib64/libcrypto.so";
	const char *libssl = "/usr/local/lib64/libssl.so";
	acl::openssl_conf::set_libpath(libcrypto, libssl);

	if (!acl::openssl_conf::load()) {
		printf("load openssl error, crypto=%s, ssl=%s\r\n", libcrypto, libssl);
		return 1;
	}
	printf("Load ssl so ok!\r\n");

	ssl_conf = new acl::openssl_conf(false);
	const char *ca_file = "./ssl_ca.pem";
	if (access(ca_file, R_OK) == 0) {
		if (!ssl_conf->load_ca(ca_file, NULL)) {
			printf("load ca error, ca=%s\r\n", ca_file);
			delete ssl_conf;
			return 1;
		}
		printf("Load ssl ca ok!\r\n");
	}

	const char *addr = "0.0.0.0:8288";
	run(addr);

	delete ssl_conf;
	return 0;
}

