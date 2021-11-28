CC = gcc
AR = ar

CFLAGS = -Wall -D_REENTRANT -D_GNU_SOURCE -fPIC -g
CFLAGS += -DLINUX_SIM
LDFLAGS = -lpthread 

LIB = libuFrameBase
LIBSUB = libuFrameBase.a

OBJS = timer.o thread.o msg.o rtc.o
	
SRCS = ${OBJS:%.o=%.c}

all: depend $(LIB)

depend:
	$(CC) $(CFLAGS) -MM $(SRCS)  > .depend

-include .depend

$(LIB):$(OBJS)
	$(AR) -rc $@.a $(OBJS)
	$(CC) -shared -Wl,--whole-archive $(LIBSUB) -Wl,--no-whole-archive  -Wl,-soname -Wl,libuFrameBase.so -o libuFrameBase.so
	
clean:
	-rm -f *.o .depend $(LIB).*
#	cd ./test && $(MAKE) clean
