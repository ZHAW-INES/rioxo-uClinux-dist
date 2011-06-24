/*
 * rscp_client.h
 *
 * rscp client binds local media-sink to uri
 *  - SETUP/PLAY/TEARDOWN/HELLO/EVENT
 *
 * requires a <client list> to keep track of connections
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#ifndef RSCP_CLIENT_H_
#define RSCP_CLIENT_H_

#include <pthread.h>

#include "rscp.h"
#include "map.h"
#include "rscp_error.h"
#include "rscp_media.h"
#include "rscp_connection.h"
#include "rscp_default.h"

enum {
    E_RSCP_CLIENT_KILL      = 0x00000001,
    E_RSCP_CLIENT_TEARDOWN  = 0x00000002,
    E_RSCP_CLIENT_PLAY      = 0x00000004
};

typedef struct {
    int                 nr;             // number of this client thread
    pthread_t           th1, th2;       // structure of the 2 client threads
    uint32_t            task;           // defines a client task (play, teardown, kill). Executed by rscp_client_event()
    char                uri[200];       // remote uri
    t_rscp_connection   con;            // TCP connection (?)
    t_rscp_connection   con1;           // Copy of con for receive (?)
    t_rscp_connection   con2;           // Copy of con for write (?)
    t_rscp_media        *media;         // associated media
    t_node              *idx;           // pointer to the node in the client list (hdoipd.client)
} t_rscp_client;

extern const t_map_set client_method[];

extern const t_map_fnc tab_response_hdcp[];
extern const t_map_fnc tab_response_setup[];
extern const t_map_fnc tab_response_play[];
extern const t_map_fnc tab_response_teardown[];
extern const t_map_fnc tab_request_update[];


// client handling based on <client list>
t_rscp_client* rscp_client_open(t_node* list, t_rscp_media *media, char* address);
void rscp_client_force_close(t_node* list);
void rscp_client_deactivate(t_node* list);
void rscp_client_event(t_node* list, uint32_t event);

// client operation
int rscp_client_close(t_rscp_client* client);
int rscp_client_kill(t_rscp_client* client);
int rscp_client_setup(t_rscp_client* client, t_rscp_transport* transport, t_rscp_edid *edid, t_rscp_hdcp *hdcp);
int rscp_client_set_kill(t_rscp_client* client);
int rscp_client_set_play(t_rscp_client* client);
int rscp_client_play(t_rscp_client* client, t_rscp_rtp_format* fmt);
int rscp_client_set_teardown(t_rscp_client* client);
int rscp_client_teardown(t_rscp_client* client);
int rscp_client_update(t_rscp_client* client, uint32_t event);
int rscp_client_hello(t_rscp_client* client);
int rscp_client_hdcp(t_rscp_client* client);



#endif /* RSCP_CLIENT_H_ */
