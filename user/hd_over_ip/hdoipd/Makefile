# Makefile for hdoipd
#
# Copyright (c) 2010, Zurich University of Applied Sciences
#                     Institute of Embedded Systems

P = hdoipd

PWD := $(shell pwd)
INCLUDES  = -I$(PWD)/include
DEFINES	  =
CFLAGS   += -W -Wall -Wextra -Wstrict-prototypes -Wsign-compare -Wshadow \
	    -Wchar-subscripts -Wmissing-declarations -Wmissing-prototypes \
	    -Wpointer-arith -Wcast-align $(INCLUDES) $(DEFINES)
LDFLASG  +=

DEBUG = false
ifeq ($(strip $(DEBUG)),true)
	CFLAGS += -g -DDEBUG
endif

HDOIP_OBJS	:= hdoipd.o
# All objects, for dependecy rules
ALL_OBJS	:= $(HDOIP_OBJS)

all: $(P)
hdoipd: $(HDOIP_OBJS)
	@echo LD $@
	@$(CC) $(LDFLAGS) -o $@ $^

-include $(ALL_OBJS:.o=.o.dep)

%.o: %.c %.h
	@echo CC $@
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) -MM $(CFLAGS) $< > $@.dep

%.o: %.c
	@echo CC $@
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) -MM $(CFLAGS) $< > $@.dep

romfs:
	$(ROMFSINST) /bin/hdoipd

cscope:
	@echo CSCOPE
	@cscope -R -b

clean:
	@rm -f *.o *.dep *.gdb $(P)

mrproper: clean
	@rm -f cscope.out
