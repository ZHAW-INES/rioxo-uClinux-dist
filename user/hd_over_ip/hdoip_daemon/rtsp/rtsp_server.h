/*
 * rtsp_server.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_SERVER_H_
#define RTSP_SERVER_H_

#include "bstmap.h"
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
    bool                kill;           // flag to kill the actual server thread by rtsp_listener
    t_bstmap*           media;          // associated media-controls
    t_bstmap*           sessions;       // sessions and their media-control
    size_t              media_session_count; // number of media-controls with an active session
    bool                open;           // whether the server is open or not
    t_rtsp_timeout      timeout;
} t_rtsp_server;

extern const t_map_set rtsp_srv_methods[];

int rtsp_server_add_media(t_rtsp_server* handle, t_rtsp_media* media);
t_rtsp_media* rtsp_server_get_media_by_name(t_rtsp_server* handle, char* name);
t_rtsp_media* rtsp_server_get_media_by_session(t_rtsp_server* handle, char* sessionid);
void rtsp_server_remove_media(t_rtsp_server* handle, t_rtsp_media* media, bool remove_from_list);

t_rtsp_server* rtsp_server_create(int fd, uint32_t addr);
void rtsp_server_close(t_rtsp_server* handle);
void rtsp_server_teardown(t_rtsp_server* handle);
void rtsp_server_event(t_rtsp_server* handle, uint32_t event);

int rtsp_server_thread(t_rtsp_server* handle);

void rtsp_server_update_media(t_rtsp_media* media, uint32_t event);

int rtsp_server_handle_setup(t_rtsp_server* handle, t_rtsp_edid *edid, t_rtsp_media *media);

#endif /* RTSP_SERVER_H_ */
