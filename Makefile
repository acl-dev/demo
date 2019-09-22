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
-Wmissing-prototypes \
-D_REENTRANT \
-D_USE_FAST_MACRO \
-DACL_WRITEABLE_CHECK \
-Wno-long-long \
-Wuninitialized \
-D_POSIX_PTHREAD_SEMANTICS \
-Winvalid-pch \
-DACL_PREPARE_COMPILE \
-DUSE_REUSEPORT \
#-DACL_CLIENT_ONLY \
#-Wno-implicit-fallthrough \
#-fstack-protector-all \
#-DDEBUG_MEM
#-DUSE_EPOLL \
#-Wno-tautological-compare \
#-Wno-invalid-source-encoding \
#-Wno-extended-offsetof
#-Wcast-align
#-Winvalid-pch -DACL_PREPARE_COMPILE

# For Darwin
ifeq ($(findstring Darwin, $(OSNAME)), Darwin)
	CFLAGS += -Wno-invalid-source-encoding \
		  -Wno-invalid-offsetof
	ldflags += -liconv
endif

# For Linux
ifeq ($(findstring Linux, $(OSNAME)), Linux)
endif



all: fiber thread_fiber_server thread_fiber_client thread thread_pool thread_mbox fiber_echod http_request redis master_threads_echod mysql mysql_pool tbox thread_cond file_lock redis_thread http_manager redis_set fiber_mutex fiber_event connect fiber_redis thread_fiber_redis thread_echod tcp_server redis_hash gethostbyname
clean cl:
	rm -f fiber thread_fiber_server thread_fiber_client thread thread_pool thread_mbox fiber_echod http_request redis master_threads_echod mysql mysql_pool tbox thread_cond file_lock redis_thread http_manager redis_set fiber_mutex fiber_event connect fiber_redis thread_fiber_redis thread_echod tcp_server redis_hash gethostbyname

fiber: fiber.cpp
	g++ fiber.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o fiber
thread_fiber_client: thread_fiber_client.cpp
	g++ thread_fiber_client.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o thread_fiber_client
thread_fiber_server: thread_fiber_server.cpp
	g++ thread_fiber_server.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o thread_fiber_server
thread: thread.cpp
	g++ thread.cpp -lacl_all -ldl -lz -lpthread -o thread
thread_pool: thread_pool.cpp
	g++ thread_pool.cpp -lacl_all -ldl -lz -lpthread -o thread_pool
thread_mbox: thread_mbox.cpp
	g++ thread_mbox.cpp -lacl_all -ldl -lz -lpthread -o thread_mbox
fiber_echod: fiber_echod.cpp
	g++ fiber_echod.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o fiber_echod
http_request: http_request.cpp
	g++ http_request.cpp -lacl_all $(ldflags) -ldl -lz -lpthread -o http_request
redis: redis.cpp
	g++ redis.cpp -lacl_all -ldl -lz -lpthread -o redis
redis_hash: redis_hash.cpp
	g++ redis_hash.cpp -lacl_all -ldl -lz -lpthread -o redis_hash
redis_thread: redis_thread.cpp
	g++ redis_thread.cpp -lacl_all -ldl -lz -lpthread -o redis_thread
master_threads_echod: master_threads_echod.cpp
	g++ master_threads_echod.cpp -lacl_all -ldl -lz -lpthread -o master_threads_echod
mysql: mysql.cpp
	g++ mysql.cpp -lacl_all -ldl -lz -lpthread -o mysql
mysql_pool: mysql_pool.cpp
	g++ mysql_pool.cpp -lacl_all -ldl -lz -lpthread -o mysql_pool
tbox: tbox.cpp
	g++ tbox.cpp -lacl_all -ldl -lz -lpthread -o tbox
thread_cond: thread_cond.cpp
	g++ thread_cond.cpp -lacl_all -ldl -lz -lpthread -o thread_cond
file_lock: file_lock.cpp
	g++ file_lock.cpp -lacl_all -ldl -lz -lpthread -o file_lock
http_manager: http_manager.cpp
	g++ http_manager.cpp -lacl_all $(ldflags) -ldl -lz -lpthread -o http_manager
redis_set: redis_set.cpp
	g++ redis_set.cpp -lacl_all -ldl -lz -lpthread -o redis_set
fiber_mutex: fiber_mutex.cpp
	g++ fiber_mutex.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o fiber_mutex
fiber_event: fiber_event.cpp
	g++ fiber_event.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o fiber_event
connect: connect.cpp
	g++ connect.cpp -o connect -lfiber_cpp -lacl_all -lfiber $(ldflags) -lz -lpthread -ldl
fiber_redis: fiber_redis.cpp
	g++ fiber_redis.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o fiber_redis
thread_fiber_redis: thread_fiber_redis.cpp
	g++ thread_fiber_redis.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o thread_fiber_redis
thread_echod: thread_echod.cpp
	g++ thread_echod.cpp -lacl_all -lz -ldl -lpthread -o thread_echod
tcp_server: tcp_server.cpp
	g++ tcp_server.cpp -lacl_all -lz -ldl -lpthread -o tcp_server
gethostbyname: gethostbyname.cpp
	g++ gethostbyname.cpp -lacl_all -lpthread -o gethostbyname
