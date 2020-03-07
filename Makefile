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
#-Wno-implicit-fallthrough \
#-fstack-protector-all \
#-Wno-tautological-compare \
#-Wno-invalid-source-encoding \
#-Wno-extended-offsetof
#-Wcast-align

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

clean cl:
	@(rm -f	$(BIN)/tbox \
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
		$(BIN)/master_threads_echod)
	@echo "All programs have been removed!"
	@(rm -f */*.o)
	@echo "All objects have been removed!"

tbox: common/tbox.o
	g++ common/tbox.o -lacl_all -ldl -lz -lpthread -o $(BIN)/tbox
common/tbox.o: common/tbox.cpp
	g++ $(CFLAGS) common/tbox.cpp -o common/tbox.o

file_lock: file/file_lock.o
	g++ file/file_lock.cpp -lacl_all -ldl -lz -lpthread -o $(BIN)/file_lock
file/file_lock.o: file/file_lock.cpp
	g++ $(CFLAGS) file/file_lock.cpp -o file/file_lock.o

connect: net/connect.o
	g++ net/connect.o -lfiber_cpp -lacl_all -lfiber $(ldflags) -lz -lpthread -ldl -o $(BIN)/connect
net/connect.o: net/connect.cpp
	g++ $(CFLAGS) net/connect.cpp -o net/connect.o

tcp_server: net/tcp_server.o
	g++ net/tcp_server.o -lacl_all -lz -ldl -lpthread -o $(BIN)/tcp_server
net/tcp_server.o: net/tcp_server.cpp
	g++ $(CFLAGS) net/tcp_server.cpp -o net/tcp_server.o

gethostbyname: net/gethostbyname.o
	g++ net/gethostbyname.o -lacl_all -lpthread -o $(BIN)/gethostbyname
net/gethostbyname.o: net/gethostbyname.cpp
	g++ $(CFLAGS) net/gethostbyname.cpp -o net/gethostbyname.o

thread_demo: thread/thread_demo.o
	g++ thread/thread_demo.o -lacl_all -ldl -lz -lpthread -o $(BIN)/thread_demo
thread/thread_demo.o: thread/thread_demo.cpp
	g++ $(CFLAGS) thread/thread_demo.cpp -o thread/thread_demo.o

thread_pool: thread/thread_pool.o
	g++ thread/thread_pool.o -lacl_all -ldl -lz -lpthread -o $(BIN)/thread_pool
thread/thread_pool.o: thread/thread_pool.cpp
	g++ $(CFLAGS) thread/thread_pool.cpp -o thread/thread_pool.o

thread_mbox: thread/thread_mbox.o
	g++ thread/thread_mbox.o -lacl_all -ldl -lz -lpthread -o $(BIN)/thread_mbox
thread/thread_mbox.o: thread/thread_mbox.cpp
	g++ $(CFLAGS) thread/thread_mbox.cpp -o thread/thread_mbox.o

thread_cond: thread/thread_cond.o
	g++ thread/thread_cond.o -lacl_all -ldl -lz -lpthread -o $(BIN)/thread_cond
thread/thread_cond.o: thread/thread_cond.cpp
	g++ $(CFLAGS) thread/thread_cond.cpp -o thread/thread_cond.o

thread_echod: thread/thread_echod.o
	g++ thread/thread_echod.o -lacl_all -lz -ldl -lpthread -o $(BIN)/thread_echod
thread/thread_echod.o: thread/thread_echod.cpp
	g++ $(CFLAGS) thread/thread_echod.cpp -o thread/thread_echod.o

http_request: http/http_request.o
	g++ http/http_request.o -lacl_all $(ldflags) -ldl -lz -lpthread -o $(BIN)/http_request
http/http_request.o: http/http_request.cpp
	g++ $(CFLAGS) http/http_request.cpp -o http/http_request.o

http_manager: http/http_manager.o
	g++ http/http_manager.o -lacl_all $(ldflags) -ldl -lz -lpthread -o $(BIN)/http_manager
http/http_manager.o: http/http_manager.cpp
	g++ $(CFLAGS) http/http_manager.cpp -o http/http_manager.o

mysql: db/mysql.o
	g++ db/mysql.o -lacl_all -ldl -lz -lpthread -o $(BIN)/mysql
db/mysql.o: db/mysql.cpp
	g++ $(CFLAGS) db/mysql.cpp -o db/mysql.o

mysql_pool: db/mysql_pool.o
	g++ db/mysql_pool.o -lacl_all -ldl -lz -lpthread -o $(BIN)/mysql_pool
db/mysql_pool.o: db/mysql_pool.cpp
	g++ $(CFLAGS) db/mysql_pool.cpp -o db/mysql_pool.o

redis_demo: redis/redis_demo.o
	g++ redis/redis_demo.o -lacl_all -ldl -lz -lpthread -o $(BIN)/redis_demo
redis/redis_demo.o: redis/redis_demo.cpp
	g++ $(CFLAGS) redis/redis_demo.cpp -o redis/redis_demo.o

redis_hash: redis/redis_hash.o
	g++ redis/redis_hash.o -lacl_all -ldl -lz -lpthread -o $(BIN)/redis_hash
redis/redis_hash.o: redis/redis_hash.cpp
	g++ $(CFLAGS) redis/redis_hash.cpp -o redis/redis_hash.o

redis_thread: redis/redis_thread.o
	g++ redis/redis_thread.o -lacl_all -ldl -lz -lpthread -o $(BIN)/redis_thread
redis/redis_thread.o: redis/redis_thread.cpp
	g++ $(CFLAGS) redis/redis_thread.cpp -o redis/redis_thread.o

redis_set: redis/redis_set.o
	g++ redis/redis_set.o -lacl_all -ldl -lz -lpthread -o $(BIN)/redis_set
redis/redis_set.o: redis/redis_set.cpp
	g++ $(CFLAGS) redis/redis_set.cpp -o redis/redis_set.o

fiber_demo: fiber/fiber_demo.o
	g++ fiber/fiber_demo.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_demo
fiber/fiber_demo.o: fiber/fiber_demo.cpp
	g++ $(CFLAGS) fiber/fiber_demo.cpp -o fiber/fiber_demo.o

fiber_echod: fiber/fiber_echod.o
	g++ fiber/fiber_echod.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_echod
fiber/fiber_echod.o: fiber/fiber_echod.cpp
	g++ $(CFLAGS) fiber/fiber_echod.cpp -o fiber/fiber_echod.o

fiber_mutex: fiber/fiber_mutex.o
	g++ fiber/fiber_mutex.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_mutex
fiber/fiber_mutex: fiber/fiber_mutex.o
	g++ $(CFLAGS) fiber/fiber_mutex.cpp -o fiber/fiber_mutex.o

fiber_event: fiber/fiber_event.o
	g++ fiber/fiber_event.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_event
fiber/fiber_event.o: fiber/fiber_event.cpp
	g++ $(CFLAGS) fiber/fiber_event.cpp -o fiber/fiber_event.o

fiber_redis: fiber/fiber_redis.o
	g++ fiber/fiber_redis.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_redis
fiber/fiber_redis.o: fiber/fiber_redis.cpp
	g++ $(CFLAGS) fiber/fiber_redis.cpp -o fiber/fiber_redis.o

thread_fiber_redis: fiber/thread_fiber_redis.o
	g++ fiber/thread_fiber_redis.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/thread_fiber_redis
fiber/thread_fiber_redis.o: fiber/thread_fiber_redis.cpp
	g++ $(CFLAGS) fiber/thread_fiber_redis.cpp -o fiber/thread_fiber_redis.o

thread_fiber_client: fiber/thread_fiber_client.o
	g++ fiber/thread_fiber_client.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/thread_fiber_client
fiber/thread_fiber_client.o: fiber/thread_fiber_client.cpp
	g++ $(CFLAGS) fiber/thread_fiber_client.cpp -o fiber/thread_fiber_client.o

thread_fiber_server: fiber/thread_fiber_server.o
	g++ fiber/thread_fiber_server.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/thread_fiber_server
fiber/thread_fiber_server.o: fiber/thread_fiber_server.cpp
	g++ $(CFLAGS) fiber/thread_fiber_server.cpp -o fiber/thread_fiber_server.o

master_threads_echod: server/master_threads_echod.o
	g++ server/master_threads_echod.o -lacl_all -ldl -lz -lpthread -o $(BIN)/master_threads_echod
server/master_threads_echod.o: server/master_threads_echod.cpp
	g++ $(CFLAGS) server/master_threads_echod.cpp -o server/master_threads_echod.o
