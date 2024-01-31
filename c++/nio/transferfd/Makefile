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
#-Wno-implicit-fallthrough \
#-fstack-protector-all \
#-Wno-tautological-compare \
#-Wno-invalid-source-encoding \
#-Wno-extended-offsetof
#-Wcast-align

LDFLAGS = -lacl_all -ldl -lz -lpthread

# For Darwin
ifeq ($(findstring Darwin, $(OSNAME)), Darwin)
	CFLAGS += -Wno-invalid-source-encoding \
		  -Wno-invalid-offsetof
	ldflags += -liconv
endif

all: dispatch server

dispatch: dispatch.o
	g++ dispatch.o $(LDFLAGS) -o dispatch
dispatch.o: dispatch.cpp
	g++ $(CFLAGS) dispatch.cpp -o dispatch.o

server: server.o
	g++ server.o $(LDFLAGS) -o server
server.o: server.cpp
	g++ $(CFLAGS) server.cpp -o server.o

clean:
	rm -f *.o server dispatch
