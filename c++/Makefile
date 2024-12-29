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

# For WSL
ifeq ($(findstring WSL, $(OSALL)), WSL)
	ldflags += -liconv
endif

BIN=bin
$(shell mkdir -p $(BIN))

all: h2b \
	tbox \
	zlib \
	file_lock \
	connect \
	tcp_server \
	udp_server \
	udp_client \
	gethostbyname \
	nio_client \
	nio_server \
	thread_demo \
	thread_pool \
	thread_mbox \
	thread_cond \
	thread_echod \
	http_get \
	http_post \
	http_pool \
	http_manager \
	https_get \
	mysql \
	mysql_pool \
	redis_demo \
	redis_hash \
	redis_thread \
	redis_set \
	redis_pipeline \
	redis_pipeline2 \
	redis_pipeline_fiber \
	fiber_demo \
	fiber_tbox \
	fiber_echod \
	fiber_mutex \
	fiber_event \
	fiber_redis \
	fiber_nslookup \
	fiber_kill \
	thread_fiber_redis \
	thread_fiber_client \
	thread_fiber_server \
	master_threads_echod \
	json_builder

clean cl:
	@(rm -f	$(BIN)/h2b \
		$(BIN)/tbox \
		$(BIN)/zlib \
		$(BIN)/file_lock \
		$(BIN)/connect \
		$(BIN)/tcp_server \
		$(BIN)/udp_server \
		$(BIN)/udp_client \
		$(BIN)/gethostbyname \
		$(BIN)/nio_client \
		$(BIN)/nio_server \
		$(BIN)/select \
		$(BIN)/poll \
		$(BIN)/poll_server \
		$(BIN)/epoll_client \
		$(BIN)/epoll_server \
		$(BIN)/thread_demo \
		$(BIN)/thread_pool \
		$(BIN)/thread_mbox \
		$(BIN)/thread_cond \
		$(BIN)/thread_echod \
		$(BIN)/http_get \
		$(BIN)/http_post \
		$(BIN)/http_pool \
		$(BIN)/http_manager \
		$(BIN)/https_get \
		$(BIN)/mysql \
		$(BIN)/mysql_pool \
		$(BIN)/redis_demo \
		$(BIN)/redis_hash \
		$(BIN)/redis_thread \
		$(BIN)/redis_set \
		$(BIN)/redis_pipeline \
		$(BIN)/redis_pipeline2 \
		$(BIN)/redis_pipeline_fiber \
		$(BIN)/fiber_demo \
		$(BIN)/fiber_tbox \
		$(BIN)/fiber_echod \
		$(BIN)/fiber_mutex \
		$(BIN)/fiber_event \
		$(BIN)/fiber_redis \
		$(BIN)/fiber_stack \
		$(BIN)/fiber_deadlock \
		$(BIN)/fiber_nslookup \
		$(BIN)/fiber_kill \
		$(BIN)/thread_fiber_redis \
		$(BIN)/thread_fiber_client \
		$(BIN)/thread_fiber_server \
		$(BIN)/master_threads_echod \
		$(BIN)/json_builder)
	@echo "All programs have been removed!"
	@(rm -f */*.o)
	@echo "All objects have been removed!"

h2b: string/h2b.o
	g++ string/h2b.o -lacl_all -ldl -lz -lpthread -o $(BIN)/h2b
string/h2b.o: string/h2b.cpp
	g++ $(CFLAGS) string/h2b.cpp -o string/h2b.o

tbox: common/tbox.o
	g++ common/tbox.o -lacl_all -ldl -lz -lpthread -o $(BIN)/tbox
common/tbox.o: common/tbox.cpp
	g++ $(CFLAGS) common/tbox.cpp -o common/tbox.o

zlib: common/zlib.o
	g++ common/zlib.o -lacl_all -ldl -lz -lpthread -o $(BIN)/zlib
common/zlib.o: common/zlib.cpp
	g++ $(CFLAGS) common/zlib.cpp -o common/zlib.o

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

udp_server: net/udp_server.o
	g++ net/udp_server.o -lacl_all -lz -ldl -lpthread -o $(BIN)/udp_server
net/udp_server.o: net/udp_server.cpp
	g++ $(CFLAGS) net/udp_server.cpp -o net/udp_server.o

udp_client: net/udp_client.o
	g++ net/udp_client.o -lacl_all -lz -ldl -lpthread -o $(BIN)/udp_client
net/udp_client.o: net/udp_client.cpp
	g++ $(CFLAGS) net/udp_client.cpp -o net/udp_client.o

gethostbyname: net/gethostbyname.o
	g++ net/gethostbyname.o -lacl_all -lpthread -o $(BIN)/gethostbyname
net/gethostbyname.o: net/gethostbyname.cpp
	g++ $(CFLAGS) net/gethostbyname.cpp -o net/gethostbyname.o

nio_client: nio/nio_client.o
	g++ nio/nio_client.o -lacl_all -lpthread -o $(BIN)/nio_client
nio/nio_client.o: nio/nio_client.cpp
	g++ $(CFLAGS) nio/nio_client.cpp -o nio/nio_client.o

nio_server: nio/nio_server.o
	g++ nio/nio_server.o -lacl_all -lpthread -o $(BIN)/nio_server
nio/nio_server.o: nio/nio_server.cpp
	g++ $(CFLAGS) nio/nio_server.cpp -o nio/nio_server.o

select2: event/select2.o
	g++ event/select2.o -lacl_all -lpthread -o $(BIN)/select2
event/select2.o: event/select2.cpp
	g++ $(CFLAGS) event/select2.cpp -o event/select2.o

select: event/select.o
	g++ event/select.o -lacl_all -lpthread -o $(BIN)/select
event/select.o: event/select.cpp
	g++ $(CFLAGS) event/select.cpp -o event/select.o

poll: event/poll.o
	g++ event/poll.o -lacl_all -lpthread -o $(BIN)/poll
event/poll.o: event/poll.cpp
	g++ $(CFLAGS) event/poll.cpp -o event/poll.o

poll_server: event/poll_server.o
	g++ event/poll_server.o -lacl_all -lpthread -o $(BIN)/poll_server
event/poll_server.o: event/poll_server.cpp
	g++ $(CFLAGS) event/poll_server.cpp -o event/poll_server.o

epoll: event/epoll.o
	g++ event/epoll.o -lacl_all -lpthread -o $(BIN)/epoll
event/epoll.o: event/epoll.cpp
	g++ $(CFLAGS) event/epoll.cpp -o event/epoll.o

epoll2: event/epoll2.o
	g++ event/epoll2.o -lfiber_cpp -lacl_all -lfiber -lpthread -ldl -o $(BIN)/epoll2
event/epoll2.o: event/epoll2.cpp
	g++ $(CFLAGS) event/epoll2.cpp -o event/epoll2.o

epoll_client: event/epoll_client.o
	g++ event/epoll_client.o -lacl_all -lpthread -o $(BIN)/epoll_client
event/epoll_client.o: event/epoll_client.cpp
	g++ $(CFLAGS) event/epoll_client.cpp -o event/epoll_client.o

epoll_server: event/epoll_server.o
	g++ event/epoll_server.o -lacl_all -lpthread -o $(BIN)/epoll_server
event/epoll_server.o: event/epoll_server.cpp
	g++ $(CFLAGS) event/epoll_server.cpp -o event/epoll_server.o

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

http_get: http/http_get.o
	g++ http/http_get.o -lacl_all $(ldflags) -ldl -lz -lpthread -o $(BIN)/http_get
http/http_get.o: http/http_get.cpp
	g++ $(CFLAGS) http/http_get.cpp -o http/http_get.o

http_post: http/http_post.o
	g++ http/http_post.o -lacl_all $(ldflags) -ldl -lz -lpthread -o $(BIN)/http_post
http/http_post.o: http/http_post.cpp
	g++ $(CFLAGS) http/http_post.cpp -o http/http_post.o

http_pool: http/http_pool.o
	g++ http/http_pool.o -lacl_all $(ldflags) -ldl -lz -lpthread -o $(BIN)/http_pool
http/http_pool.o: http/http_pool.cpp
	g++ $(CFLAGS) http/http_pool.cpp -o http/http_pool.o

http_manager: http/http_manager.o
	g++ http/http_manager.o -lacl_all $(ldflags) -ldl -lz -lpthread -o $(BIN)/http_manager
http/http_manager.o: http/http_manager.cpp
	g++ $(CFLAGS) http/http_manager.cpp -o http/http_manager.o

https_get: http/https_get.o
	g++ http/https_get.o -lacl_all $(ldflags) -ldl -lz -lpthread -o $(BIN)/https_get
http/https_get.o: http/https_get.cpp
	g++ $(CFLAGS) http/https_get.cpp -o http/https_get.o

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

redis_pipeline: redis/redis_pipeline.o
	g++ redis/redis_pipeline.o -lacl_all -ldl -lz -lpthread -o $(BIN)/redis_pipeline
redis/redis_pipeline.o: redis/redis_pipeline.cpp
	g++ $(CFLAGS) redis/redis_pipeline.cpp -o redis/redis_pipeline.o

redis_pipeline2: redis/redis_pipeline2.o
	g++ redis/redis_pipeline2.o -lacl_all -ldl -lz -lpthread -o $(BIN)/redis_pipeline2
redis/redis_pipeline2.o: redis/redis_pipeline2.cpp
	g++ $(CFLAGS) redis/redis_pipeline2.cpp -o redis/redis_pipeline2.o

redis_pipeline_fiber: redis/redis_pipeline_fiber.o
	g++ redis/redis_pipeline_fiber.o -lfiber_cpp -lacl_all -lfiber -ldl -lz -lpthread -o $(BIN)/redis_pipeline_fiber
redis/redis_pipeline_fiber.o: redis/redis_pipeline_fiber.cpp
	g++ $(CFLAGS) redis/redis_pipeline_fiber.cpp -o redis/redis_pipeline_fiber.o

fiber_demo: fiber/fiber_demo.o
	g++ fiber/fiber_demo.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_demo
fiber/fiber_demo.o: fiber/fiber_demo.cpp
	g++ $(CFLAGS) fiber/fiber_demo.cpp -o fiber/fiber_demo.o

fiber_tbox: fiber/fiber_tbox.o
	g++ fiber/fiber_tbox.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_tbox
fiber/fiber_tbox.o: fiber/fiber_tbox.cpp
	g++ $(CFLAGS) fiber/fiber_tbox.cpp -o fiber/fiber_tbox.o

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

fiber_nslookup: fiber/fiber_nslookup.o
	g++ fiber/fiber_nslookup.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_nslookup
fiber/fiber_nslookup.o: fiber/fiber_nslookup.cpp
	g++ $(CFLAGS) fiber/fiber_nslookup.cpp -o fiber/fiber_nslookup.o

fiber_kill: fiber/fiber_kill.o
	g++ fiber/fiber_kill.o -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread -o $(BIN)/fiber_kill
fiber/fiber_kill.o: fiber/fiber_kill.cpp
	g++ $(CFLAGS) fiber/fiber_kill.cpp -o fiber/fiber_kill.o

fiber_stack: fiber/fiber_stack.o
	g++ fiber/fiber_stack.o -lfiber_cpp -lacl_all -lfiber -lunwind -lunwind-x86_64 -lz -ldl -lpthread -o $(BIN)/fiber_stack
fiber/fiber_stack.o: fiber/fiber_stack.cpp
	g++ -O0 -c -g -W fiber/fiber_stack.cpp -o fiber/fiber_stack.o

fiber_deadlock: fiber/fiber_deadlock.o
	g++ fiber/fiber_deadlock.o -lfiber_cpp -lacl_all -lfiber -lunwind -lunwind-x86_64 -lz -ldl -lpthread -o $(BIN)/fiber_deadlock
fiber/fiber_deadlock.o: fiber/fiber_deadlock.cpp
	g++ -O0 -c -g -W fiber/fiber_deadlock.cpp -o fiber/fiber_deadlock.o

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

json_builder: json/json_builder.o
	g++ json/json_builder.o -lacl_all -ldl -lz -lpthread -o $(BIN)/json_builder
json/json_builder.o: json/json_builder.cpp
	g++ $(CFLAGS) json/json_builder.cpp -o json/json_builder.o
