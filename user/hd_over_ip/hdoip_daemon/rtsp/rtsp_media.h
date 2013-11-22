/*
 * rtsp_media.h
 *
 * rmsq: rtsp media server request received (C->S)
 * rmsr: rtsp media server response received (S->c)
 * rmcq: rtsp media client request received (S->C)
 * rmcr: rtsp media client response received (C->S)
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_MEDIA_H_
#define RTSP_MEDIA_H_

#include "rtsp.h"
#include "rtsp_connection.h"
#include "linked_list.h"
#include "map.h"

typedef enum {
    RTSP_STATE_NONE     = 0x0,
    RTSP_STATE_INIT     = 0x1,
    RTSP_STATE_READY    = 0x2,
    RTSP_STATE_PLAYING  = 0x4
} e_rtsp_media_state;

#define RTSP_STATE_ALL  (RTSP_STATE_INIT | RTSP_STATE_READY | RTSP_STATE_PLAYING)

typedef enum {
    HDCP_SEND_RTX,
    HDCP_VERIFY_CERT,
    HDCP_RECEIVE_RRX,
    HDCP_RECEIVE_H,
    HDCP_SEND_KS
} e_rtsp_hdcp_state;

typedef enum {
    RTSP_RESULT_IDLE = 0,
    RTSP_RESULT_READY,
    RTSP_RESULT_PLAYING,
    RTSP_RESULT_TEARDOWN,
    RTSP_RESULT_TEARDOWN_Q,
    RTSP_RESULT_PAUSE,
    RTSP_RESULT_PAUSE_Q,
    RTSP_RESULT_NO_VIDEO_IN,
    RTSP_RESULT_CONNECTION_REFUSED,
    RTSP_RESULT_SERVER_ERROR,
    RTSP_RESULT_SERVER_BUSY,
    RTSP_RESULT_SERVER_NO_VTB,
    RTSP_RESULT_SERVER_TRY_LATER,
    RTSP_RESULT_SERVER_NO_VIDEO_IN,
    RTSP_RESULT_SERVER_HDCP_ERROR,
    RTSP_RESULT_SERVER_MULTICAST
} e_rtsp_media_result;

typedef int (frtspm)(struct rtsp_media *, void *msg, struct rtsp_connection *);
typedef int (frtspl)(void* media);
typedef int (frtspe)(void* media, uint32_t event);

// Media HDCP variables
typedef struct t_rtsp_media_hdcp {
	int			hdcp_state;		// HDCP state
    char		km[33];			// masterkey
    char 		rrx[19];		// random number from receiver
    char		rn[17];			// random number from locality check
    char 		kd[65];			//
    char    	rtx[17];		// rtx (random number)
    uint32_t	repeater;		// repeater (true or false)
}t_rtsp_media_hdcp;

// Media session
typedef struct rtsp_media {
    void*   creator;            // Pointer to client or server thread creates this media (t_rtsp_server / t_rtsp_client)
    void*   top;                // UNUSED
    struct rtsp_media* owner;   // Pointer to origin media (video, audio_board, audio_emb or box_sys)
    t_node* children;           // UNUSED
    char*   name;               // "audio_board", "audio_emb", "video" or "" (box_sys)
    char    sessionid[20];      // Session string (ID)
    int     state;              // Media state (INIT, READY, PLAYING)
    int		hdcp_state;			// HDCP state
    t_rtsp_media_hdcp hdcp_var; // HDCP variables (state, keys, etc.)
    int     result;             // Media status
    size_t  cookie_size;        // Size of cookie
    void*   cookie;             // Media related data
    frtspm* options;
    frtspm* describe;
    frtspm* hdcp;
    frtspm* error;              // (media*, rtsp-code, connection)
    frtspm* get_parameter;      // (c->s)
    frtspm* set_parameter;      // (c->s)
    frtspm* setup;              // (c->s) request or response
    frtspm* play;               // (c->s) request or response
    frtspm* pause;              // (c->s|s->c) rsp=0: response / else: request
    frtspm* teardown;           // (c->s|s->c) rsp=0: response / else: request
    frtspm* update;             // (s->c) (events)
    frtspl* ready;              // local
    frtspm* dosetup;            // local
    frtspm* doplay;             // local
    frtspm* doteardown;         // local
    frtspe* event;              // local (events)
    frtspm* remove;				// local
} t_rtsp_media;

typedef struct {
    uint32_t            address;        // remote ip address
    uint8_t             mac[6];         // mac address
    uint32_t            vid_port;
    uint32_t            aud_port;
} t_hdoip_ethernet;

// Media cookie
typedef struct {
    t_hdoip_ethernet    remote;
} t_multicast_cookie;

int rtsp_media_check_request(const t_map_set* method, t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);

// rtsp media server
int rmsq_options(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmsq_describe(t_rtsp_media *media, void *msg, t_rtsp_connection *rsp);
int rmsq_get_parameter(t_rtsp_media *media, void *msg, t_rtsp_connection *rsp);
int rmsq_set_parameter(t_rtsp_media *media, void *msg, t_rtsp_connection *rsp);
int rmsq_setup(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmsq_play(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmsq_teardown(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmsq_pause(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmsq_hdcp(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);

int rmsr_pause(t_rtsp_media* media, void* msg);

// rtsp media client
int rmcq_options(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmcq_update(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmcq_pause(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmcq_teardown(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);

int rmcr_setup(t_rtsp_media* media, void* msg);
int rmcr_play(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp);
int rmcr_teardown(t_rtsp_media* media, void* msg);

// rtsp media state manipulation (local)
int rtsp_media_ready(t_rtsp_media* media);
int rtsp_media_setup(t_rtsp_media* media);
int rtsp_media_play(t_rtsp_media* media);
int rtsp_media_event(t_rtsp_media* media, uint32_t event);

// RTSP DESCRIBE
int rtsp_handle_describe_generic(t_rtsp_media *media, t_rtsp_req_describe *data,
                                 t_rtsp_connection *con);

static inline bool rtsp_media_sinit(t_rtsp_media* media)
{
    return (media->state <= RTSP_STATE_INIT);
}

static inline bool rtsp_media_sready(t_rtsp_media* media)
{
    return (media->state == RTSP_STATE_READY);
}

static inline bool rtsp_media_splaying(t_rtsp_media* media)
{
    return (media->state == RTSP_STATE_PLAYING);
}

static inline bool rtsp_media_active(t_rtsp_media* media)
{
    return (media->creator);
}

#endif /* RTSP_MEDIA_H_ */
