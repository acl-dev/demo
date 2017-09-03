all: fiber thread thread_pool thread_mbox fiber_echod http_request redis master_threads_echod mysql mysql_pool
clean cl:
	rm -f fiber thread thread_pool thread_mbox fiber_echod http_request redis master_threads_echod mysql mysql_pool
fiber: fiber.cpp
	g++ fiber.cpp -lfiber_cpp -lfiber -lacl_all -lz -ldl -lpthread -o fiber
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
