/*
 * rtsp_client.h
 *
 * rtsp client binds local media-sink to uri
 *  - SETUP/PLAY/TEARDOWN/HELLO/EVENT
 *
 * requires a <client list> to keep track of connections
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#ifndef RTSP_CLIENT_H_
#define RTSP_CLIENT_H_

#include <pthread.h>

#include "map.h"
#include "rtsp.h"
#include "rtsp_media.h"
#include "rtsp_connection.h"
#include "rtsp_default.h"

enum {
    E_RTSP_CLIENT_KILL      = 0x00000001,
    E_RTSP_CLIENT_TEARDOWN  = 0x00000002,
    E_RTSP_CLIENT_PLAY      = 0x00000004
};

typedef struct {
    int                 nr;             // number of this client thread
    pthread_t           th1, th2;       // structure of the 2 client threads
    uint32_t            task;           // defines a client task (play, teardown, kill). Executed by rtsp_client_event()
    char                uri[200];       // remote uri
    t_rtsp_connection   con;            // TCP connection (?)
    t_rtsp_connection   con1;           // Copy of con for receive (?)
    t_rtsp_connection   con2;           // Copy of con for write (?)
    t_rtsp_media        *media;         // associated media
    t_node              *idx;           // pointer to the node in the client list (hdoipd.client)
} t_rtsp_client;

extern const t_map_set rtsp_client_methods[];

extern const t_map_fnc tab_response_hdcp[];
extern const t_map_fnc tab_response_setup[];
extern const t_map_fnc tab_response_play[];
extern const t_map_fnc tab_response_pause[];
extern const t_map_fnc tab_response_teardown[];
extern const t_map_fnc tab_request_update[];


// client handling based on <client list>
t_rtsp_client* rtsp_client_open(t_node* list, t_rtsp_media *media, char* address);
void rtsp_client_force_close(t_node* list);
void rtsp_client_deactivate(t_node* list);
void rtsp_client_event(t_node* list, uint32_t event);

// client operation
int rtsp_client_close(t_rtsp_client* client);
int rtsp_client_kill(t_rtsp_client* client);
int rtsp_client_setup(t_rtsp_client* client, t_rtsp_transport* transport, t_rtsp_edid *edid, t_rtsp_hdcp *hdcp);
int rtsp_client_set_kill(t_rtsp_client* client);
int rtsp_client_set_play(t_rtsp_client* client);
int rtsp_client_play(t_rtsp_client* client, t_rtsp_rtp_format* fmt);
int rtsp_client_set_teardown(t_rtsp_client* client);
int rtsp_client_teardown(t_rtsp_client* client);
int rtsp_client_update(t_rtsp_client* client, uint32_t event);
int rtsp_client_hello(t_rtsp_client* client);
int rtsp_client_hdcp(t_rtsp_client* client);
int rtsp_client_pause(t_rtsp_client* client);

#endif /* RTSP_CLIENT_H_ */
