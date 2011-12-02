
hdoip_objs = hdoip_test.o hdt_ether.o hdt_video.o

EXEC = hdoip_test
OBJS = $(hdoip_objs)

LDLIBS = -lpthread

CFLAGS += -I../hdoip_daemon/
CFLAGS += -I../../../vendors/InES/eva/modules/hdoip_core/hal/vid
CFLAGS += -std=c99

include ../Makefile.common


