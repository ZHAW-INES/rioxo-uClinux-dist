/*
 * rtsp_media.h
 *
 * TODO:
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_MEDIA_H_
#define RTSP_MEDIA_H_

#include "rtsp.h"
#include "rtsp_connection.h"
#include "linked_list.h"

typedef enum {
    RTSP_INIT,
    RTSP_READY,
    RTSP_PLAYING
} e_rtsp_media_state;

//
typedef int (frtspm)(intptr_t media, intptr_t msg, intptr_t rsp);

// Media session
typedef struct t_rtsp_media {
    struct t_rtsp_media* owner; //!< Owner Media
    char    sessionid[20];      //!< Session string
    int     state;              //!< Media state (INIT, READY, PLAYING)
    void*   cookie;             //!< Media related data
    t_node* session;            //!< List of all sessions related to that media
    frtspm* option;
    frtspm* setup;
    frtspm* play;
    frtspm* teardown;
    frtspm* error;              //!< (media*, rtsp-code, connection)
} t_rtsp_media;

// rtsp media server
int rms_option(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rms_setup(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rms_play(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rms_teardown(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);

// rtsp media client
int rmc_option(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmc_setup(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmc_play(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmc_teardown(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);

//
t_rtsp_media* rtsp_media_get(t_rtsp_media* media, char* session);

#endif /* RTSP_MEDIA_H_ */
