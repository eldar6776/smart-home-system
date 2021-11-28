CC = gcc
AR = ar

CFLAGS = -O2 -Wall -D_REENTRANT -D_GNU_SOURCE -fPIC -g
CFLAGS += -I./include/ -DLINUX_SIM
LDFLAGS = -lpthread 

LIB = libuFrame
LIBSUB = libuFrame.a

OBJS = timer.o thread.o msg.o
OBJS = framework/framework.o \
	   framework/generic.o \
	   framework/message.o \
	   framework/systime.o \
	   framework/device/key.o 
	
SRCS = ${OBJS:%.o=%.c}

all: depend $(LIB)

depend:
	$(CC) $(CFLAGS) -MM $(SRCS)  > .depend

-include .depend

$(LIB):$(OBJS)
	$(AR) -rc $@.a $(OBJS)
	$(CC) -shared -Wl,--whole-archive $(LIBSUB) -Wl,--no-whole-archive  -Wl,-soname -Wl,libuFrame.so -o libuFrame.so
	
clean:
	-rm -f *.o .depend $(LIB).* $(OBJS)
#	cd ./test && $(MAKE) clean
