EXEC = hdoipd
OBJS = hdoipd_main.o

CFLAGS += -I.
CFLAGS += -Iedid
OBJS   +=   edid/edid.o
CFLAGS += -Iupdate
OBJS   +=   update/altremote.o
OBJS   +=   update/flash.o
OBJS   +=   update/update.o
OBJS   +=   update/util.o
CFLAGS += -Ilib
OBJS   +=   lib/linked_list.o
OBJS   +=   lib/bstree.o
OBJS   +=   lib/bstmap.o
CFLAGS += -Idriver
OBJS   +=   driver/hoi_drv_user.o
OBJS   +=   driver/hoi_drv_str.o
CFLAGS += -Ifsm
OBJS   +=   fsm/hdoipd_callback.o
OBJS   +=   fsm/hdoipd_cmd.o
OBJS   +=   fsm/hdoipd_fsm.o
OBJS   +=   fsm/hdoipd_default.o
OBJS   +=   fsm/hdoipd_task.o
CFLAGS += -Iimage
OBJS   +=   image/hoi_image.o
OBJS   +=   image/hoi_res.o
CFLAGS += -Irscp
OBJS   +=   rscp/map.o
OBJS   +=   rscp/rscp_net.o
OBJS   +=   rscp/rscp_client.o
OBJS   +=   rscp/rscp_command.o
OBJS   +=   rscp/rscp_connection.o
OBJS   +=   rscp/rscp_default.o
OBJS   +=   rscp/rscp_listener.o
OBJS   +=   rscp/rscp_media.o
OBJS   +=   rscp/rscp_parse_header.o
OBJS   +=   rscp/rscp_parse_table.o
OBJS   +=   rscp/rscp_server.o
OBJS   +=   rscp/rscp_string.o
CFLAGS += -Iosd
OBJS   +=   osd/hdoipd_osd.o
CFLAGS += -Imedia
OBJS   +=   media/box_sys.o
OBJS   +=   media/vrb_video.o
OBJS   +=   media/vtb_video.o
OBJS   +=   media/vrb_audio.o
OBJS   +=   media/vtb_audio.o
CFLAGS += -Iconfig
OBJS   +=   config/hoi_cfg.o

# we need the ioctl defines from hdoip_types.h
CFLAGS += -I../../../vendors/InES/eva/modules/hdoip_core/driver
CFLAGS += -std=gnu99

LDLIBS += -lpthread -lrt

include ../Makefile.common
