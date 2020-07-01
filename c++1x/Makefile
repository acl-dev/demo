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
-D_REENTRANT \
-D_USE_FAST_MACRO \
-DACL_WRITEABLE_CHECK \
-Wno-long-long \
-Wuninitialized \
-D_POSIX_PTHREAD_SEMANTICS \
-Winvalid-pch \
-DACL_PREPARE_COMPILE \
-DUSE_REUSEPORT \
-std=c++11

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

BIN=bin

$(shell mkdir -p $(BIN) )

all: httpd

clean cl:
	@(rm -f	$(BIN)/httpd)
	@(rm -f $(BIN)/httpd.cf)
	@echo "All programs have been removed!"
	@(rm -f */*.o)
	@echo "All objects have been removed!"

httpd: httpd/httpd.o
	g++ httpd/httpd.o -o $(BIN)/httpd $(ldflags)
	@(cp httpd/httpd.cf $(BIN)/)
httpd/httpd.o: httpd/httpd.cpp
	g++ $(CFLAGS) httpd/httpd.cpp -o httpd/httpd.o
