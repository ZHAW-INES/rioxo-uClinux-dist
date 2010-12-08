/*
 * rtsp_client.h
 *
 * rtsp client binds local media-sink to uri
 *  - SETUP/PLAY/TEARDOWN
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#ifndef RTSP_CLIENT_H_
#define RTSP_CLIENT_H_

#include "rtsp.h"
#include "map.h"
#include "rtsp_error.h"
#include "rtsp_media.h"
#include "rtsp_connection.h"
#include "rtsp_default.h"

typedef struct {
    char                uri[200];       //!< remote resource name
    t_rtsp_connection   con;
    u_rtsp_response     rsp;            //!< request data structure
    t_rtsp_media        *media;
} t_rtsp_client;

extern const t_map_fnc tab_response_setup[];
extern const t_map_fnc tab_response_play[];
extern const t_map_fnc tab_response_teardown[];

int rtsp_client_open(t_rtsp_client* client, t_rtsp_media *media, char* uri);
int rtsp_client_close(t_rtsp_client* client);

// returns response code
int rtsp_client_setup(t_rtsp_client* client, t_rtsp_transport* transport);
int rtsp_client_play(t_rtsp_client* client, t_rtsp_rtp_format* fmt);
int rtsp_client_teardown(t_rtsp_client* client);

#endif /* RTSP_CLIENT_H_ */
