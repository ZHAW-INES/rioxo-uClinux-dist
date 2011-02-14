/*
 * rscp_media.h
 *
 * rmsq: rscp media server request (C->S)
 * rmsr: rscp media server response (S->c)
 * rmcq: rscp media client request (S->C)
 * rmcr: rscp media client response (C->S)
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RSCP_MEDIA_H_
#define RSCP_MEDIA_H_

#include "rscp.h"
#include "rscp_connection.h"
#include "linked_list.h"

typedef enum {
    RSCP_INIT,
    RSCP_READY,
    RSCP_PLAYING
} e_rscp_media_state;

typedef enum {
    RSCP_RESULT_IDLE = 0,
    RSCP_RESULT_READY,
    RSCP_RESULT_PLAYING,
    RSCP_RESULT_TEARDOWN,
    RSCP_RESULT_TEARDOWN_Q,
    RSCP_RESULT_PAUSE,
    RSCP_RESULT_PAUSE_Q,
    RSCP_RESULT_NO_VIDEO_IN,
    RSCP_RESULT_CONNECTION_REFUSED,
    RSCP_RESULT_SERVER_ERROR,
    RSCP_RESULT_SERVER_BUSY,
    RSCP_RESULT_SERVER_NO_VTB,
    RSCP_RESULT_SERVER_TRY_LATER,
    RSCP_RESULT_SERVER_NO_VIDEO_IN
} e_rscp_media_result;

//
typedef int (frscpo)(void* media, void* msg, void* rsp, void* owner);
typedef int (frscpm)(void* media, void* msg, void* rsp);
typedef int (frscpl)(void* media);
typedef int (frscpe)(void* media, uint32_t event);

// Media session
typedef struct t_rscp_media {
    void*   creator;            //!<
    struct t_rscp_media* owner; //!< Owner Media
    t_node* children;           //!< Child Medias
    char*   name;
    char    sessionid[20];      //!< Session string
    int     state;              //!< Media state (INIT, READY, PLAYING)
    int     result;
    void*   cookie;             //!< Media related data
    frscpm* error;              //!< (media*, rscp-code, connection)
    frscpm* option;
    frscpm* setup;              //!< (c->s) request or response
    frscpm* play;               //!< (c->s) request or response
    frscpm* pause_q;            //!< (c->s|s->c) always request
    frscpm* pause_r;            //!< (c->s|s->c) always response
    frscpm* teardown_q;         //!< (c->s|s->c) always request
    frscpm* teardown_r;         //!< (c->s|s->c) always response
    frscpm* hello;              //!< (s->c)
    frscpm* update;             //!< (s->c) (events)
    frscpl* ready;              //!< local
    frscpl* dosetup;            //!< local
    frscpl* doplay;             //!< local
    frscpe* event;              //!< local (events)
} t_rscp_media;

// rscp media server
int rmsq_option(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner);
int rmsq_setup(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner);
int rmsq_play(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner);
int rmsq_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner);
int rmsq_hello(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner);
int rmsq_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner);
int rmsr_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner);
int rmsr_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmsr_update(t_rscp_media* media, void* msg, t_rscp_connection* rsp);

// rscp media client
int rmcq_update(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner);
int rmcq_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner);
int rmcr_option(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmcr_setup(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmcr_play(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmcr_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmcq_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner);

// rscp media local
int rscp_media_ready(t_rscp_media* media);
int rscp_media_setup(t_rscp_media* media);
int rscp_media_play(t_rscp_media* media);
int rscp_media_event(t_rscp_media* media, uint32_t event);

// state changes...
int rscp_media_force_ready(t_rscp_media* media);


static inline bool rscp_media_sinit(t_rscp_media* media)
{
    return (media->state == RSCP_INIT);
}

static inline bool rscp_media_sready(t_rscp_media* media)
{
    return (media->state == RSCP_READY);
}

static inline bool rscp_media_splaying(t_rscp_media* media)
{
    return (media->state == RSCP_PLAYING);
}

static inline bool rscp_media_active(t_rscp_media* media)
{
    return (media->creator);
}

#endif /* RSCP_MEDIA_H_ */
