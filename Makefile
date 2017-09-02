all: fiber thread thread_mbox fiber_echod
clean cl:
	rm -f fiber thread thread_mbox fiber_echod
fiber: fiber.cpp
	g++ fiber.cpp -lfiber_cpp -lfiber -lacl_all -lz -ldl -lpthread -o fiber
thread: thread.cpp
	g++ thread.cpp -lacl_all -ldl -lz -lpthread -o thread
thread_mbox: thread_mbox.cpp
	g++ thread_mbox.cpp -lacl_all -ldl -lz -lpthread -o thread_mbox
fiber_echod: fiber_echod.cpp
	g++ fiber_echod.cpp -lfiber_cpp -lfiber -lacl_all -lz -ldl -lpthread -o fiber_echod
