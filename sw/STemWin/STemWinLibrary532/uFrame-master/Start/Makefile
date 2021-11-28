CC = gcc
CFLAGS = -Wall -g
CFLAGS += -I./GUI/Core -I./Config/ -I./GUI/WM/ -I./GUI/Widget -I./GUI/Core
OBJS:=test.o MicriumLogo_1bpp.o MicriumLogo.o
all:gui $(OBJS)
	$(CC) -o test $(OBJS) GUI/libucgui.a -lrt

.PHONY:clean gui
gui:
	make -C GUI
clean:
	rm -f *.o test
