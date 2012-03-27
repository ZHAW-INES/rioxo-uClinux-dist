/*
 * rscp_media.h
 *
 * rmsq: rscp media server request received (C->S)
 * rmsr: rscp media server response received (S->c)
 * rmcq: rscp media client request received (S->C)
 * rmcr: rscp media client response received (C->S)
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
    HDCP_SEND_RTX,
    HDCP_VERIFY_CERT,
    HDCP_RECEIVE_RRX,
    HDCP_RECEIVE_H,
    HDCP_SEND_KS
} e_rscp_hdcp_state;

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
    RSCP_RESULT_SERVER_NO_VIDEO_IN,
    RSCP_RESULT_SERVER_HDCP_ERROR
} e_rscp_media_result;

typedef int (frscpm)(void* media, void* msg, void* rsp);
typedef int (frscpl)(void* media);
typedef int (frscpe)(void* media, uint32_t event);

// Media HDCP variables
typedef struct t_rscp_media_hdcp {
	int			hdcp_state;		// HDCP state
    char		km[33];			// masterkey
    char 		rrx[19];		// random number from receiver
    char		rn[17];			// random number from locality check
    char 		kd[65];			//
    char    	rtx[17];		// rtx (random number)
    uint32_t	repeater;		// repeater (true or false)
}t_rscp_media_hdcp;

// Media session
typedef struct t_rscp_media {
    void*   creator;            // Pointer to client or server thread creates this media (t_rscp_server / t_rscp_client)
    void*   top;                // UNUSED
    struct t_rscp_media* owner; // Pointer to origin media (video, audio or box_sys)
    t_node* children;           // UNUSED
    char*   name;               // "audio", "video" or "" (box_sys)
    char    sessionid[20];      // Session string (ID)
    int     state;              // Media state (INIT, READY, PLAYING)
    int		hdcp_state;			// HDCP state
    t_rscp_media_hdcp hdcp_var; // HDCP variables (state, keys, etc.)
    int     result;             // Media status
    size_t  cookie_size;        // Size of cookie
    void*   cookie;             // Media related data
    frscpm* hdcp;
    frscpm* error;              // (media*, rscp-code, connection)
    frscpm* setup;              // (c->s) request or response
    frscpm* play;               // (c->s) request or response
    frscpm* pause;              // (c->s|s->c) rsp=0: response / else: request
    frscpm* teardown;           // (c->s|s->c) rsp=0: response / else: request
    frscpm* hello;              // (s->c)
    frscpm* update;             // (s->c) (events)
    frscpl* ready;              // local
    frscpm* dosetup;            // local
    frscpm* doplay;             // local
    frscpm* doteardown;         // local
    frscpe* event;              // local (events)
} t_rscp_media;

typedef struct {
    uint32_t            address;        // remote ip address
    uint8_t             mac[6];         // mac address
    uint32_t            vid_port;
    uint32_t            aud_port;
} t_hdoip_ethernet;

// Media cookie
typedef struct {
    int                 timeout;
    int                 alive_ping;
    t_hdoip_ethernet    remote;
} t_multicast_cookie;


// rscp media server
int rmsq_setup(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmsq_play(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmsq_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmsq_hello(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmsq_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmsq_update(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmsq_hdcp(t_rscp_media* media, void* msg, t_rscp_connection* rsp);

int rmsr_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmsr_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp);



// rscp media client
int rmcq_update(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmcq_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmcq_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp);

int rmcr_setup(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmcr_play(t_rscp_media* media, void* msg, t_rscp_connection* rsp);
int rmcr_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp);

// rscp media state manipulation (local)
int rscp_media_ready(t_rscp_media* media);
int rscp_media_setup(t_rscp_media* media);
int rscp_media_play(t_rscp_media* media);
int rscp_media_event(t_rscp_media* media, uint32_t event);

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
