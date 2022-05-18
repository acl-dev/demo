OSNAME  = $(shell uname -s)
OSTYPE  = $(shell uname -m)
ldflags =
CFLAGS = -c -g -W \
-O3 \
-fPIC \
-Wall \
-Werror \
-Wshadow \
-Wpointer-arith \
-Waggregate-return \
-D_REENTRANT \
-D_USE_FAST_MACRO \
-DACL_WRITEABLE_CHECK \
-Wno-long-long \
-Wuninitialized \
-D_POSIX_PTHREAD_SEMANTICS \
-Winvalid-pch \
-DACL_PREPARE_COMPILE \
-DUSE_REUSEPORT \
-std=c++11

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

BIN=bin

$(shell mkdir -p $(BIN) )

all: fiber_httpd redis_thread redis_pipeline fiber_echod thread_pool \
	tcp_server tcp_server2 tcp_client go_waiter

clean cl:
	@(rm -f	$(BIN)/fiber_httpd)
	@(rm -f $(BIN)/fiber_httpd.cf)
	@(rm -f	$(BIN)/redis_thread)
	@(rm -f	$(BIN)/redis_pipeline)
	@(rm -f $(BIN)/fiber_echod)
	@(rm -f $(BIN)/thread_pool)
	@(rm -f $(BIN)/tcp_server)
	@(rm -f $(BIN)/tcp_server2)
	@(rm -f $(BIN)/tcp_client)
	@(rm -f $(BIN)/go_waiter)
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
