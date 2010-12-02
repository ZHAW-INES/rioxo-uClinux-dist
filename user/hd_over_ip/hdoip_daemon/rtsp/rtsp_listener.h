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

#include "linked_list.h"

#define RTSP_BACKLOG    (20)

typedef struct {
    pthread_t th;               //!< Server thread (accepts connection)
    t_node* cons;               //!< Connections

    pthread_mutex_t mutex_run;  //!< protects mutex/run
    pthread_mutex_t mutex;      //!< protects connections
    bool run;                   //!< run flag

    int port;
    int sockfd;
} t_rtsp_listener;


int rtsp_listener_start(t_rtsp_listener* handle, int port);
int rtsp_listener_close(t_rtsp_listener* handle);

#endif /* RTSP_LISTENER_H_ */
