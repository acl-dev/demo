OSNAME  = $(shell uname -s)
OSTYPE  = $(shell uname -m)
OSALL   = $(shell uname -a)
ldflags =
CFLAGS = -c -g -W \
-O3 \
-fPIC \
-Wall \
-Werror \
-Wshadow \
-Wpointer-arith \
-D_REENTRANT \
-D_USE_FAST_MACRO \
-Wno-long-long \
-Wuninitialized \
-D_POSIX_PTHREAD_SEMANTICS \
-Winvalid-pch \
-std=c++11

#-Waggregate-return

ldflags = -lfiber_cpp -lacl_all -lfiber -ldl -lz -lpthread

# For Darwin
ifeq ($(findstring Darwin, $(OSNAME)), Darwin)
	CFLAGS += -Wno-invalid-source-encoding \
		  -Wno-invalid-offsetof
	ldflags += -liconv
endif

# For Linux
ifeq ($(findstring Linux, $(OSNAME)), Linux)
endif

ifeq ($(findstring WSL, $(OSALL)), WSL)
	ldflags += -liconv
endif

BIN=bin

$(shell mkdir -p $(BIN) )

all: fiber_httpd redis_thread redis_pipeline fiber_echod thread_pool \
	tcp_server tcp_server2 tcp_client udp_server udp_client \
	go_waiter fiber_tbox tbox fiber_tbox2 fiber_mutex fiber_sbox

clean cl:
	@(rm -f	$(BIN)/fiber_httpd)
	@(rm -f $(BIN)/fiber_httpd.cf)
	@(rm -f	$(BIN)/redis_thread)
	@(rm -f	$(BIN)/redis_pipeline)
	@(rm -f $(BIN)/fiber_echod)
	@(rm -f $(BIN)/thread_pool)
	@(rm -f $(BIN)/tcp_server)
	@(rm -f $(BIN)/tcp_server2)
	@(rm -f $(BIN)/udp_server)
	@(rm -f $(BIN)/udp_client)
	@(rm -f $(BIN)/tcp_client)
	@(rm -f $(BIN)/go_waiter)
	@(rm -f $(BIN)/fiber_tbox)
	@(rm -f $(BIN)/fiber_tbox2)
	@(rm -f $(BIN)/tbox)
	@(rm -f $(BIN)/fiber_mutex)
	@(rm -f $(BIN)/fiber_sbox)
	@echo "All programs have been removed!"
	@(rm -f */*.o)
	@echo "All objects have been removed!"

fiber_httpd: httpd/fiber_httpd.o
	g++ httpd/fiber_httpd.o -o $(BIN)/fiber_httpd $(ldflags)
	@(cp httpd/fiber_httpd.cf $(BIN)/)
httpd/fiber_httpd.o: httpd/fiber_httpd.cpp
	g++ $(CFLAGS) httpd/fiber_httpd.cpp -o httpd/fiber_httpd.o

redis_thread: redis/redis_thread.o
	g++ redis/redis_thread.o -o $(BIN)/redis_thread $(ldflags)
redis/redis_thread.o: redis/redis_thread.cpp
	g++ $(CFLAGS) redis/redis_thread.cpp -o redis/redis_thread.o

redis_pipeline: redis/redis_pipeline.o
	g++ redis/redis_pipeline.o -o $(BIN)/redis_pipeline $(ldflags)
redis/redis_pipeline.o: redis/redis_pipeline.cpp
	g++ $(CFLAGS) redis/redis_pipeline.cpp -o redis/redis_pipeline.o

fiber_echod: fiber/fiber_echod.o
	g++ fiber/fiber_echod.o -o $(BIN)/fiber_echod $(ldflags)
fiber/fiber_echod.o: fiber/fiber_echod.cpp
	g++ $(CFLAGS) fiber/fiber_echod.cpp -o fiber/fiber_echod.o

go_waiter: fiber/go_waiter.o
	g++ fiber/go_waiter.o -o $(BIN)/go_waiter $(ldflags)
fiber/go_waiter.o: fiber/go_waiter.cpp
	g++ $(CFLAGS) fiber/go_waiter.cpp -o fiber/go_waiter.o

fiber_tbox: fiber/fiber_tbox.o
	g++ fiber/fiber_tbox.o -o $(BIN)/fiber_tbox $(ldflags)
fiber/fiber_tbox.o: fiber/fiber_tbox.cpp
	g++ $(CFLAGS) fiber/fiber_tbox.cpp -o fiber/fiber_tbox.o

fiber_tbox2: fiber/fiber_tbox2.o
	g++ fiber/fiber_tbox2.o -o $(BIN)/fiber_tbox2 $(ldflags)
fiber/fiber_tbox2.o: fiber/fiber_tbox2.cpp
	g++ $(CFLAGS) fiber/fiber_tbox2.cpp -o fiber/fiber_tbox2.o

tbox: fiber/tbox.o
	g++ fiber/tbox.o -o $(BIN)/tbox $(ldflags)
fiber/tbox.o: fiber/tbox.cpp
	g++ $(CFLAGS) fiber/tbox.cpp -o fiber/tbox.o

thread_pool: thread_pool/thread_pool.o thread_pool/main.o
	g++ thread_pool/thread_pool.o thread_pool/main.o -o $(BIN)/thread_pool $(ldflags)
thread_pool/thread_pool.o: thread_pool/thread_pool.cpp
	g++ $(CFLAGS) thread_pool/thread_pool.cpp -o thread_pool/thread_pool.o
thread_pool/main.o: thread_pool/main.cpp
	g++ $(CFLAGS) thread_pool/main.cpp -o thread_pool/main.o

tcp_server: net/tcp_server.o
	g++ net/tcp_server.o -o $(BIN)/tcp_server $(ldflags)
net/tcp_server.o: net/tcp_server.cpp
	g++ $(CFLAGS) net/tcp_server.cpp -o net/tcp_server.o

tcp_server2: net/tcp_server2.o
	g++ net/tcp_server2.o -o $(BIN)/tcp_server2 $(ldflags)
net/tcp_server2.o: net/tcp_server2.cpp
	g++ $(CFLAGS) net/tcp_server2.cpp -o net/tcp_server2.o

tcp_client: net/tcp_client.o
	g++ net/tcp_client.o -o $(BIN)/tcp_client $(ldflags)
net/tcp_client.o: net/tcp_client.cpp
	g++ $(CFLAGS) net/tcp_client.cpp -o net/tcp_client.o

udp_server: net/udp_server.o
	g++ net/udp_server.o -o $(BIN)/udp_server $(ldflags)
net/udp_server.o: net/udp_server.cpp
	g++ $(CFLAGS) net/udp_server.cpp -o net/udp_server.o

udp_client: net/udp_client.o
	g++ net/udp_client.o -o $(BIN)/udp_client $(ldflags)
net/udp_client.o: net/udp_client.cpp
	g++ $(CFLAGS) net/udp_client.cpp -o net/udp_client.o


fiber_mutex: fiber/fiber_mutex.o
	g++ fiber/fiber_mutex.o -o $(BIN)/fiber_mutex $(ldflags)
fiber/fiber_mutex.o: fiber/fiber_mutex.cpp
	g++ $(CFLAGS) fiber/fiber_mutex.cpp -o fiber/fiber_mutex.o

fiber_sbox: fiber/fiber_sbox.o
	g++ fiber/fiber_sbox.o -o $(BIN)/fiber_sbox $(ldflags)
fiber/fiber_sbox.o: fiber/fiber_sbox.cpp
	g++ $(CFLAGS) fiber/fiber_sbox.cpp -o fiber/fiber_sbox.o
