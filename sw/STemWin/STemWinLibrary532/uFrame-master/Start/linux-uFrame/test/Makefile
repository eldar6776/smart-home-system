CC = gcc
AR = ar

CFLAGS = -Wall -D_REENTRANT -D_GNU_SOURCE -fPIC -g
CFLAGS += -I../include/
#LDFLAGS = -lpthread  -luFrame -luFrameBase -L../ -L../../linux-uFrameBase/
LDFLAGS = -lpthread ../libuFrame.a ../../linux-uFrameBase/libuFrameBase.a ../../GUI/libucgui.a
LDFLAGS += -lrt

OBJS = main.o aaa.o disp.o 
	
SRCS = ${OBJS:%.o=%.c}

all: depend test

depend:
	$(CC) $(CFLAGS) -MM $(SRCS)  > .depend

-include .depend
test:$(OBJS)
	$(CC) $(CFLAGS) -o uFrame_test $^ $(LDFLAGS)

clean:
	-rm -f *.o .depend  uFrame_test
#	cd ./test && $(MAKE) clean
