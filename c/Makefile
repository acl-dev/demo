OSNAME  = $(shell uname -s)
OSTYPE  = $(shell uname -m)
ldflags = -lz -ldl -lpthread
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

BIN=bin
$(shell mkdir -p $(BIN))

all: vstring_base64

clean cl:
	@(rm -f $(BIN)/vstring_base64)
	@(rm -f */*.o)

vstring_base64: code/vstring_base64.o
	gcc code/vstring_base64.o -lacl_all $(ldflags) -o $(BIN)/vstring_base64
code/vstring_base64.o: code/vstring_base64.c
	gcc $(CFLAGS) code/vstring_base64.c -o code/vstring_base64.o
