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

BIN=bin

all: fiber_demo \
	tbox \
	file_lock \
	connect \
	tcp_server \
	gethostbyname \
	thread_demo \
	thread_pool \
	thread_mbox \
	thread_cond \
	thread_echod \
	http_request \
	http_manager \
	mysql \
	mysql_pool \
	redis_demo \
	redis_hash \
	redis_thread \
	redis_set \
	fiber_demo \
	fiber_echod \
	fiber_mutex \
	fiber_event \
	fiber_redis \
	thread_fiber_redis \
	thread_fiber_client \
	thread_fiber_server \
	master_threads_echod

<<<<<<< HEAD
clean cl:
	rm -f	$(BIN)/tbox \
		$(BIN)/file_lock \
		$(BIN)/connect \
		$(BIN)/tcp_server \
		$(BIN)/gethostbyname \
		$(BIN)/thread_demo \
		$(BIN)/thread_pool \
		$(BIN)/thread_mbox \
		$(BIN)/thread_cond \
		$(BIN)/thread_echod \
		$(BIN)/http_request \
		$(BIN)/http_manager \
		$(BIN)/mysql \
		$(BIN)/mysql_pool \
		$(BIN)/redis_demo \
		$(BIN)/redis_hash \
		$(BIN)/redis_thread \
		$(BIN)/redis_set \
		$(BIN)/fiber_demo \
		$(BIN)/fiber_echod \
		$(BIN)/fiber_mutex \
		$(BIN)/fiber_event \
		$(BIN)/fiber_redis \
		$(BIN)/thread_fiber_redis \
		$(BIN)/thread_fiber_client \
		$(BIN)/thread_fiber_server \
		$(BIN)/master_threads_echod

tbox: common/tbox.cpp
	g++ common/tbox.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/tbox
file_lock: file/file_lock.cpp
	g++ file/file_lock.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/file_lock
connect: net/connect.cpp
	g++ net/connect.cpp -lfiber_cpp -lacl_all -lfiber $(ldflags) -lz -lpthread -ldl -o $(BIN)/connect
tcp_server: net/tcp_server.cpp
	g++ net/tcp_server.cpp -lacl_all -lz -ldl -lpthread -o $(BIN)/tcp_server
gethostbyname: net/gethostbyname.cpp
	g++ net/gethostbyname.cpp -lacl_all -lpthread -o $(BIN)/gethostbyname
thread_demo: thread/thread_demo.cpp
	g++ thread/thread_demo.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/thread_demo
thread_pool: thread/thread_pool.cpp
	g++ thread/thread_pool.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/thread_pool
thread_mbox: thread/thread_mbox.cpp
	g++ thread/thread_mbox.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/thread_mbox
thread_cond: thread/thread_cond.cpp
	g++ thread/thread_cond.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/thread_cond
thread_echod: thread/thread_echod.cpp
	g++ thread/thread_echod.cpp -lacl_all -lz -ldl -lpthread -o $(BIN)/thread_echod
http_request: http/http_request.cpp
	g++ http/http_request.cpp -lacl_all $(ldflags) -ldl -lz -lpthread -o $(BIN)/http_request
http_manager: http/http_manager.cpp
	g++ http/http_manager.cpp -lacl_all $(ldflags) -ldl -lz -lpthread -o $(BIN)/http_manager
mysql: db/mysql.cpp
	g++ db/mysql.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/mysql
mysql_pool: db/mysql_pool.cpp
	g++ db/mysql_pool.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/mysql_pool
redis_demo: redis/redis_demo.cpp
	g++ redis/redis_demo.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/redis_demo
redis_hash: redis/redis_hash.cpp
	g++ redis/redis_hash.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/redis_hash
redis_thread: redis/redis_thread.cpp
	g++ redis/redis_thread.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/redis_thread
redis_set: redis/redis_set.cpp
	g++ redis/redis_set.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/redis_set
fiber_demo: fiber/fiber_demo.cpp
	g++ fiber/fiber_demo.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_demo
fiber_echod: fiber/fiber_echod.cpp
	g++ fiber/fiber_echod.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_echod
fiber_mutex: fiber/fiber_mutex.cpp
	g++ fiber/fiber_mutex.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_mutex
fiber_event: fiber/fiber_event.cpp
	g++ fiber/fiber_event.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_event
fiber_redis: fiber/fiber_redis.cpp
	g++ fiber/fiber_redis.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_redis
thread_fiber_redis: fiber/thread_fiber_redis.cpp
	g++ fiber/thread_fiber_redis.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/thread_fiber_redis
thread_fiber_client: fiber/thread_fiber_client.cpp
	g++ fiber/thread_fiber_client.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/thread_fiber_client
thread_fiber_server: fiber/thread_fiber_server.cpp
	g++ fiber/thread_fiber_server.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/thread_fiber_server
master_threads_echod: server/master_threads_echod.cpp
	g++ server/master_threads_echod.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/master_threads_echod
=======
all: fiber thread_fiber_server thread_fiber_client thread thread2 thread_pool thread_mbox fiber_echod http_request redis master_threads_echod mysql mysql_pool tbox thread_cond file_lock redis_thread http_manager redis_set fiber_mutex fiber_event connect fiber_redis thread_fiber_redis thread_echod tcp_server redis_hash gethostbyname
clean cl:
	rm -f fiber thread_fiber_server thread_fiber_client thread thread2 thread_pool thread_mbox fiber_echod http_request redis master_threads_echod mysql mysql_pool tbox thread_cond file_lock redis_thread http_manager redis_set fiber_mutex fiber_event connect fiber_redis thread_fiber_redis thread_echod tcp_server redis_hash gethostbyname

fiber: fiber.cpp
	g++ fiber.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o fiber
thread_fiber_client: thread_fiber_client.cpp
	g++ thread_fiber_client.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o thread_fiber_client
thread_fiber_server: thread_fiber_server.cpp
	g++ thread_fiber_server.cpp -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o thread_fiber_server
thread: thread.cpp
	g++ thread.cpp -lacl_all -ldl -lz -lpthread -o thread
thread2: thread2.cpp
	g++ thread2.cpp -lacl_all -ldl -lz -lpthread -o thread2
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
>>>>>>> a86eaf1e2b49faa31d5741a87b1c77cd756bbee6
