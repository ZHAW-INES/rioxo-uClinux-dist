
hdoip_objs  = hdoip_cli.o hdoip_lib.o
hdoip_objs += hdoip_cli_cmds.o

EXEC = hdoip
OBJS = $(hdoip_objs)

CFLAGS += -I../hdoip_daemon/
CFLAGS += -I../../../vendors/InES/eva/modules/hdoip_core/hal/vid
CFLAGS += -std=c99

include ../Makefile.common
