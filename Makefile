all: fiber thread_fiber_server thread_fiber_client thread thread_pool thread_mbox fiber_echod http_request redis master_threads_echod mysql mysql_pool tbox thread_cond
clean cl:
	rm -f fiber thread_fiber_server thread_fiber_client thread thread_pool thread_mbox fiber_echod http_request redis master_threads_echod mysql mysql_pool tbox thread_cond
fiber: fiber.cpp
	g++ fiber.cpp -lfiber_cpp -lfiber -lacl_all -lz -ldl -lpthread -o fiber
thread_fiber_client: thread_fiber_client.cpp
	g++ thread_fiber_client.cpp -lfiber_cpp -lfiber -lacl_all -lz -ldl -lpthread -o thread_fiber_client
thread_fiber_server: thread_fiber_server.cpp
	g++ thread_fiber_server.cpp -lfiber_cpp -lfiber -lacl_all -lz -ldl -lpthread -o thread_fiber_server
thread: thread.cpp
	g++ thread.cpp -lacl_all -ldl -lz -lpthread -o thread
thread_pool: thread_pool.cpp
	g++ thread_pool.cpp -lacl_all -ldl -lz -lpthread -o thread_pool
thread_mbox: thread_mbox.cpp
	g++ thread_mbox.cpp -lacl_all -ldl -lz -lpthread -o thread_mbox
fiber_echod: fiber_echod.cpp
	g++ fiber_echod.cpp -lfiber_cpp -lfiber -lacl_all -lz -ldl -lpthread -o fiber_echod
http_request: http_request.cpp
	g++ http_request.cpp -lacl_all -ldl -lz -lpthread -o http_request
redis: redis.cpp
	g++ redis.cpp -lacl_all -ldl -lz -lpthread -o redis
master_threads_echod: master_threads_echod.cpp
	g++ master_threads_echod.cpp -lacl_all -ldl -lz -lpthread -o master_threads_echod
mysql: mysql.cpp
	g++ mysql.cpp -lacl_all -ldl -lz -lpthread -o mysql
mysql_pool: mysql_pool.cpp
	g++ mysql_pool.cpp -lacl_all -ldl -lz -lpthread -o mysql_pool
tbox: tbox.cpp
	g++ tbox.cpp -lacl_all -ldl -lz -lpthread -o tbox
thread_cond:
	g++ thread_cond.cpp -lacl_all -ldl -lz -lpthread -o thread_cond
