/*
 * rtsp_listener.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_LISTENER_H_
#define RTSP_LISTENER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <pthread.h>

#include "bstmap.h"
#include "linked_list.h"
#include "rtsp_media.h"
#include "rtsp_server.h"

#define RTSP_BACKLOG    (20)

typedef struct {
    int nr;                     // number of this listener threads
    pthread_t th;               // structure of this thread
    t_node* cons;               // list of TCP connections (rtsp server threads). 1 entry for each client and media. (t_rtsp_server)
    t_bstmap* media;            // map of local medias. E.g. 1x video, 1x audio, 1x box_sys (origin media)
    t_bstmap* sessions;         // map of active medias associated with this listener
    t_node* kills;              // list of media (TCP connection) listener has to kill (remove from session list and rtsp_server_close())

    pthread_mutex_t mutex;      // protects connections
    bool run;                   // run flag if listener thread is running

    int port;                   // TCP port listener is listening
    int sockfd;                 // TCP socket
} t_rtsp_listener;


int rtsp_listener_start(t_rtsp_listener* handle, int port);
int rtsp_listener_close(t_rtsp_listener* handle);

// active media in server
int rtsp_listener_add_media(t_rtsp_listener* handle, t_rtsp_media* media);
int rtsp_listener_add_kill(t_rtsp_listener* handle, t_rtsp_server* server);
t_rtsp_media* rtsp_listener_get_media(t_rtsp_listener* handle, char* name);
void rtsp_listener_free_media(t_rtsp_listener* handle);

// session management
int rtsp_listener_add_session(t_rtsp_listener* handle, char* id, void* owner);
int rtsp_listener_remove_session(t_rtsp_listener* handle, char* id);
void* rtsp_listener_get_session(t_rtsp_listener* handle, char* id);
void rtsp_listener_create_sessionid(t_rtsp_listener* handle, char* id);

// S->C communication
void rtsp_listener_event(t_rtsp_listener* handle, uint32_t event);
void rtsp_listener_close_all(t_rtsp_listener* handle);
void rtsp_listener_teardown_all(t_rtsp_listener* handle);
void rtsp_listener_session_traverse(t_rtsp_listener* handle, void (*f)(char*, char*, void*), void* d);

void rtsp_listener_close_connection(t_rtsp_listener* handle, uint32_t remote_address, bool teardown);

#endif /* RTSP_LISTENER_H_ */
