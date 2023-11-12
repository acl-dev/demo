all: fiber_tbox thread_tbox tbox_wait fiber_server1 fiber_server2 fiber_kill
clean cl:
	rm -f fiber_tbox thread_tbox tbox_wait fiber_server1 fiber_server2 fiber_kill

fiber_tbox: fiber_tbox.cpp
	g++ fiber_tbox.cpp -o fiber_tbox -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread
thread_tbox: thread_tbox.cpp
	g++ thread_tbox.cpp -o thread_tbox -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread
tbox_wait: tbox_wait.cpp
	g++ tbox_wait.cpp -o tbox_wait -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread
fiber_server1: fiber_server1.cpp
	g++ fiber_server1.cpp -o fiber_server1 -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread
fiber_server2: fiber_server2.cpp
	g++ fiber_server2.cpp -o fiber_server2 -std=c++11 -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread
fiber_kill: fiber_kill.cpp
	g++ fiber_kill.cpp -o fiber_kill -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread
