/*
 * rscp_listener.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RSCP_LISTENER_H_
#define RSCP_LISTENER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <pthread.h>

#include "bstmap.h"
#include "linked_list.h"
#include "rscp_media.h"

#define RSCP_BACKLOG    (20)

typedef struct {
    int nr;
    pthread_t th;               //!< Server thread (accepts connection)
    t_node* cons;               //!< Connections
    t_bstmap* media;            //!< media associated with this listener
    t_bstmap* sessions;         //!< active sessions associated with this listener
    t_node* kills;              //!< media to be killed

    pthread_mutex_t mutex;      //!< protects connections
    bool run;                   //!< run flag

    int port;
    int sockfd;
} t_rscp_listener;


int rscp_listener_start(t_rscp_listener* handle, int port);
int rscp_listener_close(t_rscp_listener* handle);

// active media in server
int rscp_listener_add_media(t_rscp_listener* handle, t_rscp_media* media);
int rscp_listener_add_kill(t_rscp_listener* handle, t_rscp_media* media);
t_rscp_media* rscp_listener_get_media(t_rscp_listener* handle, char* name);
void rscp_listener_free_media(t_rscp_listener* handle);

// session management
int rscp_listener_add_session(t_rscp_listener* handle, t_rscp_media* media);
int rscp_listener_remove_session(t_rscp_listener* handle, t_rscp_media* media);
t_rscp_media* rscp_listener_get_session(t_rscp_listener* handle, char* id);
void rscp_listener_create_sessionid(t_rscp_listener* handle, char* id);

// S->C communication
void rscp_listener_close_all(t_rscp_listener* handle);
void rscp_listener_event(t_rscp_listener* handle, uint32_t event);

void rscp_listener_teardown_all(t_rscp_listener* handle);
void rscp_listener_update_all(t_rscp_listener* handle);
void rscp_listener_pause_all(t_rscp_listener* handle);

void rscp_listener_teardown(t_rscp_listener* handle, t_rscp_media* filter);
void rscp_listener_update(t_rscp_listener* handle, t_rscp_media* filter);
void rscp_listener_pause(t_rscp_listener* handle, t_rscp_media* filter);
void rscp_listener_session_traverse(t_rscp_listener* handle, void (*f)(char*, char*, void*), void* d);

#endif /* RSCP_LISTENER_H_ */
