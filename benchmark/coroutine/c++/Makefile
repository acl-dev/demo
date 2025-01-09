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
-D_REENTRANT \
-D_USE_FAST_MACRO \
-Wno-long-long \
-Wuninitialized \
-D_POSIX_PTHREAD_SEMANTICS \
-Winvalid-pch \
-std=c++11

#-Waggregate-return

ldflags = -lfiber_cpp -lacl_all -lfiber -ldl -lz -lpthread

# For Darwin
ifeq ($(findstring Darwin, $(OSNAME)), Darwin)
	CFLAGS += -Wno-invalid-source-encoding \
		  -Wno-invalid-offsetof
	ldflags += -liconv
endif

# For Linux
ifeq ($(findstring Linux, $(OSNAME)), Linux)
endif

ifeq ($(findstring WSL, $(OSALL)), WSL)
	ldflags += -liconv
endif

LIBRARY_PATH = /usr/local/lib
LIBRARY_NAME = libjemalloc.a

CHECK_JEMALLOC := $(shell if [ -f $(LIBRARY_PATH)/$(LIBRARY_NAME) ]; then echo "yes"; else echo "no"; fi)

ifeq ($(CHECK_JEMALLOC),yes)
	ldflags += -ljemalloc -Wl,-rpath,$(LIBRARY_PATH)
endif

all: fiber_pool

clean cl:
	@(rm -f fiber_pool fiber_event)
	@(rm -f *.o)
	@echo "All have been removed!"

fiber_pool: fiber_pool.o
	g++ fiber_pool.o -o fiber_pool $(ldflags)
fiber_pool.o: fiber_pool.cpp
	g++ $(CFLAGS) fiber_pool.cpp -o fiber_pool.o

fiber_event: fiber_event.o
	g++ fiber_event.o -o fiber_event $(ldflags)
fiber_event.o: fiber_event.cpp
	g++ $(CFLAGS) fiber_event.cpp -o fiber_event.o
