all: fiber_tbox thread_tbox tbox_wait
clean cl:
	rm -f fiber_tbox thread_tbox tbox_wait
fiber_tbox: fiber_tbox.cpp
	g++ fiber_tbox.cpp -o fiber_tbox -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread
thread_tbox: thread_tbox.cpp
	g++ thread_tbox.cpp -o thread_tbox -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread
tbox_wait: tbox_wait.cpp
	g++ tbox_wait.cpp -o tbox_wait -lfiber_cpp -lacl_all -lfiber -lz -ldl -lpthread
