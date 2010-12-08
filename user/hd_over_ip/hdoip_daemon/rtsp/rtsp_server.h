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
#include "rtsp_net.h"

typedef struct {
    pthread_t           th;             //!< Server Connection thread
    t_node*             idx;            //!< node containing this connection
    void*               owner;
    t_rtsp_connection   con;
    u_rtsp_request      req;            //!< request data structure
} t_rtsp_server;

extern const t_map_rec srv_media[];
extern const t_map_set srv_method[];

int rtsp_server_init(t_rtsp_server* handle, int fd, uint32_t addr);
int rtsp_server_thread(t_rtsp_server* handle);

#endif /* RTSP_SERVER_H_ */
