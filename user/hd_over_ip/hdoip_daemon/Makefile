EXEC = hdoipd
OBJS = hdoipd_main.o

CFLAGS += -I.
CFLAGS += -Idriver
OBJS   +=   driver/hoi_drv_user.o
CFLAGS += -Ifsm
OBJS   +=   fsm/hdoipd_fsm.o
CFLAGS += -Iimage
OBJS   +=   image/hoi_image.o
OBJS   +=   image/hoi_res.o

# we need the ioctl defines from hdoip_types.h
CFLAGS += -I../../../vendors/InES/eva/modules/hdoip_core/driver
CFLAGS += -std=gnu99 

LDLIBS += -lpthread -lrt

include ../Makefile.common
