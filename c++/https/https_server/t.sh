#!/bin/sh

os=$(echo `uname -s`)
if [ $os == "Darwin" ]; then
	./https_server -l "/usr/local/lib/libcrypto.dylib; /usr/local/lib/libssl.dylib" -c ./ssl_crt.pem -k ./ssl_key.pem
else
	./https_server -l "/usr/local/lib64/libcrypto.so; /usr/local/lib64/libssl.so" -c ./ssl_crt.pem -k ./ssl_key.pem
fi
