all: fiber thread thread_mbox
clean cl:
	rm -f fiber thread thread_mbox
fiber: fiber.cpp
	g++ fiber.cpp -lfiber_cpp -lfiber -lacl_all -lz -ldl -lpthread -o fiber
thread: thread.cpp
	g++ thread.cpp -lacl_all -ldl -lz -lpthread -o thread
thread_mbox: thread_mbox.cpp
	g++ thread_mbox.cpp -lacl_all -ldl -lz -lpthread -o thread_mbox
