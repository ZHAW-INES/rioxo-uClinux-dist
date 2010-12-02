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
CFLAGS += -Irtsp
OBJS   +=   rtsp/linked_list.o
OBJS   +=   rtsp/map.o
OBJS   +=   rtsp/rtsp_net.o
OBJS   +=   rtsp/rtsp_client.o
OBJS   +=   rtsp/rtsp_command.o
OBJS   +=   rtsp/rtsp_connection.o
OBJS   +=   rtsp/rtsp_default.o
OBJS   +=   rtsp/rtsp_listener.o
OBJS   +=   rtsp/rtsp_media.o
OBJS   +=   rtsp/rtsp_parse_header.o
OBJS   +=   rtsp/rtsp_parse_table.o
OBJS   +=   rtsp/rtsp_server_media.o
OBJS   +=   rtsp/rtsp_server.o
OBJS   +=   rtsp/rtsp_string.o
CFLAGS += -Iosd
OBJS   +=   osd/hdoipd_osd.o
CFLAGS += -Imedia
OBJS   +=   media/vrb_video.o
OBJS   +=   media/vtb_video.o


# we need the ioctl defines from hdoip_types.h
CFLAGS += -I../../../vendors/InES/eva/modules/hdoip_core/driver
CFLAGS += -std=gnu99 

LDLIBS += -lpthread -lrt

include ../Makefile.common
