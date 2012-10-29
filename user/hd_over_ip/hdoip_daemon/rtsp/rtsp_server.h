/*
 * rtsp_server.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_SERVER_H_
#define RTSP_SERVER_H_

#include "map.h"
#include "rtsp.h"
#include "rtsp_net.h"
#include "rtsp_error.h"
#include "rtsp_media.h"
#include "rtsp_connection.h"

typedef struct {
    int                 nr;             // number of this server thread
    t_node*             idx;            // node containing this connection
    void*               owner;          // pointer to listener that creates server thread (t_rtsp_listener)
    t_rtsp_connection   con;            // TCP connection of this thread
    bool				kill;           // flag to kill the actual server thread by rtsp_listener
    t_rtsp_media*		media;          // associated media
} t_rtsp_server;

extern const t_map_set srv_method[];

t_rtsp_server* rtsp_server_create(int fd, uint32_t addr);

int rtsp_server_thread(t_rtsp_server* handle);

void rtsp_server_close(t_rtsp_media* media);
void rtsp_server_teardown(t_rtsp_media* media);
void rtsp_server_update(t_rtsp_media* media, uint32_t event);
void rtsp_server_pause(t_rtsp_media* media);

#endif /* RTSP_SERVER_H_ */
