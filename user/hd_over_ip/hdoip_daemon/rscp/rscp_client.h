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

typedef struct {
    int                 nr;
    pthread_t           th1, th2;
    bool				kill;
    char                uri[200];       //!< remote resource name
    t_rscp_connection   con;
    t_rscp_connection   con1;           //!< request/response multiplexer
    t_rscp_connection   con2;
    t_rscp_media        *media;
    t_node              *idx;
} t_rscp_client;

extern const t_map_set client_method[];

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

int rscp_client_setup(t_rscp_client* client, t_rscp_transport* transport, t_rscp_edid *edid);
int rscp_client_play(t_rscp_client* client, t_rscp_rtp_format* fmt);
int rscp_client_teardown(t_rscp_client* client);
int rscp_client_update(t_rscp_client* client, uint32_t event);
int rscp_client_hello(t_rscp_client* client);

#endif /* RSCP_CLIENT_H_ */
