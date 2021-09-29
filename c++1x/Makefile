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

all: httpd redis_thread redis_pipeline fiber_echod thread_pool tcp_server tcp_client

clean cl:
	@(rm -f	$(BIN)/httpd)
	@(rm -f $(BIN)/httpd.cf)
	@(rm -f	$(BIN)/redis_thread)
	@(rm -f	$(BIN)/redis_pipeline)
	@(rm -f $(BIN)/fiber_echod)
	@(rm -f $(BIN)/thread_pool)
	@(rm -f $(BIN)/tcp_server)
	@(rm -f $(BIN)/tcp_client)
	@echo "All programs have been removed!"
	@(rm -f */*.o)
	@echo "All objects have been removed!"

httpd: httpd/httpd.o
	g++ httpd/httpd.o -o $(BIN)/httpd $(ldflags)
	@(cp httpd/httpd.cf $(BIN)/)
httpd/httpd.o: httpd/httpd.cpp
	g++ $(CFLAGS) httpd/httpd.cpp -o httpd/httpd.o

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

tcp_client: net/tcp_client.o
	g++ net/tcp_client.o -o $(BIN)/tcp_client $(ldflags)
net/tcp_client.o: net/tcp_client.cpp
	g++ $(CFLAGS) net/tcp_client.cpp -o net/tcp_client.o
