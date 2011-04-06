/*
 * rscp_server.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RSCP_SERVER_H_
#define RSCP_SERVER_H_

#include "map.h"
#include "rscp.h"
#include "rscp_net.h"
#include "rscp_error.h"
#include "rscp_media.h"
#include "rscp_connection.h"

typedef struct {
    int                 nr;
    t_node*             idx;            //!< node containing this connection
    void*               owner;
    t_rscp_connection   con;
} t_rscp_server;

extern const t_map_set srv_method[];

t_rscp_server* rscp_server_create(int fd, uint32_t addr);
void rscp_server_free(t_rscp_server* server);

int rscp_server_thread(t_rscp_server* handle);

void rscp_server_close(t_rscp_media* media);
void rscp_server_teardown(t_rscp_media* media);
void rscp_server_update(t_rscp_media* media, uint32_t event);
void rscp_server_pause(t_rscp_media* media);

#endif /* RSCP_SERVER_H_ */
