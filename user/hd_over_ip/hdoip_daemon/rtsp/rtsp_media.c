/*
 * rtsp_media.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 *
 * - Interface to rtsp
 * - RTSP state machine
 */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "hdoipd.h"
#include "rtsp_media.h"
#include "rtsp_listener.h"
#include "rtsp_server.h"
#include "rtsp_client.h"
#include "rtsp_command.h"
#include "rtsp_error.h"
#include "string.h"


///////////////////////////////////////////////////////////////////////////////
// received a request as a server (can answer -> rsp != 0)



int rmsq_setup(t_rtsp_media* _media, void* msg, t_rtsp_connection* rsp)
{
    t_rtsp_media *media = _media;
    t_rtsp_server *server = (t_rtsp_server*)media->creator;

    int ret = RTSP_SERVER_ERROR;

    // creator is the rtsp_server who started this connection
    // we get the rtsp_listener from it

    media->top = server->owner;
    // start new session?
    if (!_media->owner) {
        media = malloc(sizeof(t_rtsp_media));
        if (!media) {
            report(ERROR "rmsq_setup.malloc: out of memory");
            return ret;
        }
        memcpy(media, _media, sizeof(t_rtsp_media));
        media->owner = _media;
        server->media = media;

        if (media->cookie_size) {
            media->cookie = malloc(media->cookie_size);
            if (!media->cookie) {
                report(ERROR "rmsq_setup.malloc: out of memory");
                free(media);
                return ret;
            }
            memset(media->cookie, 0, media->cookie_size);
        }
        rtsp_listener_create_sessionid(media->top, media->sessionid);
    }

    if (media->setup) ret = media->setup(media, msg, rsp);

    if (ret == RTSP_SUCCESS) {
        switch (media->state) {
            case RTSP_INIT:
            case RTSP_READY:
                media->state = RTSP_READY;
            break;
            case RTSP_PLAYING:
                media->state = RTSP_PLAYING;
            break;
        }
    }

    if (media != _media) {
        if (ret == RTSP_SUCCESS) {
            rtsp_listener_add_session(media->top, media);
        } else {
            if (media->cookie_size) free(media->cookie);
            free(media);
        }
        _media->creator = 0;
    }

    // request properly handled
    if (ret == RTSP_REQUEST_ERROR) {
        ret = RTSP_SUCCESS;
    }

    return ret;
}

int rmsq_play(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SERVER_ERROR;

    // needs to be a session
    if (!media->owner) return RTSP_CLIENT_ERROR;

    switch (media->state) {
        case RTSP_INIT:
            rtsp_response_error(rsp, RTSP_SC_METHOD_NVIS, "Method not Valid in this State");
        break;
        case RTSP_READY:
        case RTSP_PLAYING:
            if (media->play) ret = media->play(media, msg, rsp);
            if (ret == RTSP_SUCCESS) {
                media->state = RTSP_PLAYING;
            } else {
                // 3xx error codes go to init
                // 4xx error codes do not change
                if ((ret >= 300) && (ret < 400)) {
                    media->state = RTSP_INIT;
                }
            }
        break;
    }

    // request properly handled
    if (ret == RTSP_REQUEST_ERROR) {
        ret = RTSP_SUCCESS;
    }

    return ret;
}


/** Teardown server request
 *
 * Note: the listener(owner) is NOT locked when this function is called
 * Sessions are removed by this function
 *
 */
int rmsq_teardown(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret;

    ret = RTSP_SERVER_ERROR;

    t_rtsp_server *server = (t_rtsp_server*)media->creator;

    // needs to be a session
    if (!media->owner) return RTSP_CLIENT_ERROR;


    if ((media->teardown) && (media->state != RTSP_INIT)) ret = media->teardown(media, msg, rsp);
    media->state = RTSP_INIT;
    if(server) {
    	server->media = 0;
    }
    if (media->cookie_size) free(media->cookie);
    rtsp_listener_remove_session(media->top, media);
    free(media);
    return ret;
}

int rmsq_hello(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
    if (media->hello) ret = media->hello(media, msg, rsp);
    return ret;
}

// call hdcp function
int rmsq_hdcp(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
    if (media->hdcp) {
    	ret = media->hdcp(media, msg, rsp);
    }
    return ret;
}

// rtsp media server
int rmsq_pause(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
    if (media->state == RTSP_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, rsp);
        if (!ret) media->state = RTSP_READY;
    }
    return ret;
}

// rtsp media server request
int rmsq_update(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
    if (media->update) ret = media->update(media, msg, rsp);
    switch (ret) {
    	case RTSP_PAUSE: media->state = RTSP_READY; ret = RTSP_SUCCESS; break;
    	case RTSP_CLOSE: media->state = RTSP_INIT; ret = RTSP_SUCCESS; break;
    }
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// received a response as a server (can not answer -> rsp = 0)

/** Teardown server
 *
 * Note: the listener(owner) is locked when this function is called
 * Sessions are removed by the caller
 *
 */
int rmsr_teardown(t_rtsp_media* media, void* msg)
{
    int ret = RTSP_SUCCESS;

    t_rtsp_server *server = (t_rtsp_server*)media->creator;
    // needs to be a session
    if (!media->owner) return RTSP_CLIENT_ERROR;

    if ((media->teardown) && (media->state != RTSP_INIT)) ret = media->teardown(media, msg, 0);
    media->state = RTSP_INIT;
    if(server) {
    	server->media = 0;
    }

    if (media->cookie_size) free(media->cookie);
    free(media);
    return ret;
}

int rmsr_pause(t_rtsp_media* media, void* msg)
{
    int ret = RTSP_SUCCESS;
    if (media->state == RTSP_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, 0);
        if (!ret) media->state = RTSP_READY;
    }
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// received a request as a client (can answer -> rsp != 0)


int rmcq_update(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
    if (media->update) ret = media->update(media, msg, rsp);
    switch (ret) {
    	case RTSP_PAUSE: media->state = RTSP_READY; ret = RTSP_SUCCESS; break;
    	case RTSP_CLOSE: media->state = RTSP_INIT; ret = RTSP_SUCCESS; break;
    }
    return ret;
}

int rmcq_pause(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
    if (media->state == RTSP_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, rsp);
        if (!ret) media->state = RTSP_READY;
    }
    return ret;
}

int rmcq_teardown(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;

    if ((media->teardown) && (media->state != RTSP_INIT)) ret = media->teardown(media, msg, rsp);
    media->state = RTSP_INIT;

    ((t_rtsp_client*)media->creator)->task = E_RTSP_CLIENT_KILL;

    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// received a response as a client (can not answer -> rsp = 0)

int rmcr_pause(t_rtsp_media* media, void* msg)
{
    int ret = RTSP_SUCCESS;
    if (media->state == RTSP_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, 0);
        if (!ret) media->state = RTSP_READY;
    }
    return ret;
}

int rmcr_setup(t_rtsp_media* media, void* msg)
{
    int ret = RTSP_SUCCESS;

    ret = media->setup(media, msg, 0);

    if (ret == RTSP_SUCCESS) {
        switch (media->state) {
            case RTSP_INIT:
            case RTSP_READY:
                media->state = RTSP_READY;
            break;
            case RTSP_PLAYING:
                media->state = RTSP_PLAYING;
            break;
        }
    }

    return ret;
}

int rmcr_play(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;

    switch (media->state) {
        case RTSP_INIT:
            rtsp_response_error(rsp, RTSP_SC_METHOD_NVIS, "Method not Valid in this State");
        break;
        case RTSP_READY:
        case RTSP_PLAYING:
            ret = media->play(media, msg, 0);
            if (ret == RTSP_SUCCESS) {
                media->state = RTSP_PLAYING;
            } else {
                // 3xx error codes go to init
                // 4xx error codes do not change
                if ((ret >= 300) && (ret < 400)) {
                    media->state = RTSP_INIT;
                }
            }
        break;
    }

    return ret;
}

int rmcr_teardown(t_rtsp_media* media, void* msg)
{
    int ret = RTSP_SUCCESS;

    if ((media->teardown) && (media->state != RTSP_INIT)) ret = media->teardown(media, msg, 0);
    media->state = RTSP_INIT;
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// state manipulation (local)

int rtsp_media_ready(t_rtsp_media* media)
{
    int ret = RTSP_SUCCESS;
    if (media) {
        if (media->ready) ret = media->ready(media);
    }
    return ret;
}

int rtsp_media_setup(t_rtsp_media* media)
{
    int ret = RTSP_NULL_POINTER;
    if (media && media->creator) {
        if (media->state != RTSP_PLAYING) {
            if (media->dosetup) ret = media->dosetup(media, 0, 0);
        } else {
            ret = RTSP_WRONG_STATE;
        }
    }
    return ret;
}

int rtsp_media_play(t_rtsp_media* media)
{
    int ret = RTSP_NULL_POINTER;
    t_rtsp_client *client = media->creator;

    if (media && media->creator) {
        if (media->state == RTSP_READY) {
            if (media->doplay) ret = media->doplay(media, 0, 0);
            //check if kill / teardown is set because of HDCP error
            if(client->task & E_RTSP_CLIENT_KILL) {
            	report(INFO "KILL CLIENT");
            	rtsp_client_close(client);
            }
            else if (client->task & E_RTSP_CLIENT_TEARDOWN) {
            	report(INFO "TEARDOWN CLIENT");
            	rtsp_client_teardown(client);
            }
            //if(((t_rtsp_client*)media->creator)->task & E_RTSP_CLIENT_KILL) rtsp_client_close((t_rtsp_client*)media->creator);
            //else if (((t_rtsp_client*)media->creator)->task & E_RTSP_CLIENT_KILL) rtsp_client_teardown((t_rtsp_client*)media->creator);
        } else {
            ret = RTSP_WRONG_STATE;
        }
    }
    return ret;
}

int rtsp_media_event(t_rtsp_media* media, uint32_t event)
{
    int ret = RTSP_NULL_POINTER;

    if (media && media->creator) {
    	if (media->state != RTSP_INIT) {
			if (media->event) ret = media->event(media, event);
			switch (ret) {
				case RTSP_PAUSE: media->state = RTSP_READY; ret = RTSP_SUCCESS; break;
				case RTSP_CLOSE: media->state = RTSP_INIT; ret = RTSP_SUCCESS; break;
			}
    	}
    }
    return ret;
}
