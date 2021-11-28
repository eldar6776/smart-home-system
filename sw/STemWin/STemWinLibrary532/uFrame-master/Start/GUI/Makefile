#SUBDIRS=AntiAlias ConvertMono Font LCDDriver MemDev Widget ConvertColor Core JPEG MultiLayer WM

CC = gcc
CFLAGS = -I./Core -I../Config/ -I./WM/ -I./Widget
CFLAGS += -Wall -g
CFLAGS += -DGUI_DEBUG_LEVEL=7

LIBSRCS=
LIBSRCS+=$(wildcard AntiAlias/*.c)
LIBSRCS+=$(wildcard ConvertMono/*.c)
LIBSRCS+=$(wildcard Font/*.c)
LIBSRCS+=$(wildcard LCDDriver/*.c)
LIBSRCS+=$(wildcard MemDev/*.c)
LIBSRCS+=$(wildcard ConvertColor/*.c)
LIBSRCS+=$(wildcard Core/*.c)
LIBSRCS+=$(wildcard JPEG/*.c)
LIBSRCS+=$(wildcard MultiLayer/*.c)
LIBSRCS+=$(wildcard WM/*.c)
LIBSRCS+=$(wildcard Widget/*.c)

LIBOBJS:=$(LIBSRCS:%.c=%.o)

all:
	make guilib

.PHONY:guilib

guilib:$(LIBOBJS)
	@ar -rc libucgui.a $(LIBOBJS)


.PHONY:clean    

clean:
	@rm -f libucgui.a 
	@rm -f $(LIBOBJS)
