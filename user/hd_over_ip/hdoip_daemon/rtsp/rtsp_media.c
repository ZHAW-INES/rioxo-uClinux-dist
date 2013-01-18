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

int rtsp_media_check_request(const t_map_set *method, t_rtsp_media* media, void *msg UNUSED, t_rtsp_connection* rsp)
{
    if (method == NULL || media == NULL || rsp == NULL)
      return RTSP_SERVER_ERROR;

    // check if the request must be part of a session
    if (method->in_session && media->owner == NULL) {
        report(" ? method (%s) is only valid in a session", method->name);
        rtsp_response_error(rsp, RTSP_STATUS_SESSION_NOT_FOUND, NULL);
        return RTSP_HANDLED;
    }

    // check if the request is valid in the current state
    if ((media->state & method->states) == RTSP_STATE_NONE) {
        report(" ? method (%s) is not valid in the current state (%d) of media-control (%s)", method->name, media->state, media->name);
        rtsp_response_error(rsp, RTSP_STATUS_METHOD_NOT_VALID_IN_THIS_STATE, NULL);
        return RTSP_HANDLED;
    }

    // check if the media implementation supports this method
    int *impl = (int*)((char*)media + method->impl_offset);
    if (impl == NULL || *impl == 0) {
        report(" ? no implementation for method (%s) in media-control (%s)", method->name, media->name);
        rtsp_response_error(rsp, RTSP_STATUS_METHOD_NOT_ALLOWED, NULL);
        return RTSP_HANDLED;
    }

    return RTSP_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// received a request as a server (can answer -> rsp != 0)

int rmsq_options(t_rtsp_media *media, void *msg UNUSED, t_rtsp_connection *rsp)
{
    int ret;
    if ((ret = media->options(media, (void *) rtsp_srv_methods, rsp)) != 0)
        return ret;

    return RTSP_SUCCESS;
}

int rmsq_get_parameter(t_rtsp_media *media, void *msg, t_rtsp_connection *rsp)
{
    return media->get_parameter(media, msg, rsp);
}

int rmsq_set_parameter(t_rtsp_media *media, void *msg, t_rtsp_connection *rsp)
{
    return media->set_parameter(media, NULL, rsp);
}

int rmsq_setup(t_rtsp_media* _media, void* msg, t_rtsp_connection* rsp)
{
    t_rtsp_media *media = _media;
    // creator is the rtsp_server who started this connection
    // we get the rtsp_listener from it
    t_rtsp_server *server = (t_rtsp_server*)media->creator;
    int ret = RTSP_SERVER_ERROR;

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
        media->state = RTSP_STATE_INIT;

        // always create a new sessionid
        rtsp_listener_create_sessionid(media->top, media->sessionid);
        rtsp_server_add_media(server, media);

        if (media->cookie_size) {
            media->cookie = malloc(media->cookie_size);
            if (!media->cookie) {
                report(ERROR "rmsq_setup.malloc: out of memory");
                free(media);
                return ret;
            }
            memset(media->cookie, 0, media->cookie_size);
        }
    }

    report(INFO "rmsq_setup(): calling media's SETUP implementation");
    if (media->setup) ret = media->setup(media, msg, rsp);

    if (ret == RTSP_SUCCESS) {
        switch (media->state) {
            case RTSP_STATE_NONE:
            case RTSP_STATE_INIT:
            case RTSP_STATE_READY:
                media->state = RTSP_STATE_READY;
            break;
            case RTSP_STATE_PLAYING:
                media->state = RTSP_STATE_PLAYING;
            break;
        }
    }

    if (media != _media) {
        if (ret == RTSP_SUCCESS) {
            report(" ? RTSP Server [%d] added media (%s) for session (%s)", server->nr, media->name, media->sessionid);
            rtsp_listener_add_session(media->top, media->sessionid, server);
        } else {
            report(" ? RTSP Server [%d] failed to create media (%s)", server->nr, _media->name);
            if (media->cookie_size) free(media->cookie);
            rtsp_server_remove_media(server, media, true);
        }
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
        case RTSP_STATE_NONE:
        case RTSP_STATE_INIT:
            rtsp_response_error(rsp, RTSP_STATUS_METHOD_NOT_VALID_IN_THIS_STATE, NULL);
        break;
        case RTSP_STATE_READY:
        case RTSP_STATE_PLAYING:
            if (media->play) ret = media->play(media, msg, rsp);
            if (ret == RTSP_SUCCESS) {
                media->state = RTSP_STATE_PLAYING;
            } else {
                // 3xx error codes go to init
                // 4xx error codes do not change
                if ((ret >= 300) && (ret < 400)) {
                    media->state = RTSP_STATE_INIT;
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

    if (media->teardown && !rtsp_media_sinit(media)) ret = media->teardown(media, msg, rsp);
    media->state = RTSP_STATE_INIT;
    if (media->cookie_size) free(media->cookie);

    if (server != NULL)
      rtsp_server_remove_media(server, media, true);

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
    if (media->state == RTSP_STATE_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, rsp);
        if (!ret) media->state = RTSP_STATE_READY;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// received a response as a server (can not answer -> rsp = 0)

int rmsr_pause(t_rtsp_media* media, void* msg)
{
    int ret = RTSP_SUCCESS;
    if (media->state == RTSP_STATE_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, 0);
        if (!ret) media->state = RTSP_STATE_READY;
    }
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// received a request as a client (can answer -> rsp != 0)

int rmcq_options(t_rtsp_media *media, void *msg UNUSED, t_rtsp_connection *rsp)
{
    int ret;
    if ((ret = media->options(media, (void *)rtsp_client_methods, rsp)) != 0)
        return ret;

    return RTSP_SUCCESS;
}

int rmcq_update(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
    if (media->update) ret = media->update(media, msg, rsp);
    switch (ret) {
    	case RTSP_PAUSE: media->state = RTSP_STATE_READY; ret = RTSP_SUCCESS; break;
    	case RTSP_CLOSE: media->state = RTSP_STATE_INIT; ret = RTSP_SUCCESS; break;
    }
    return ret;
}

int rmcq_pause(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
    if (media->state == RTSP_STATE_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, rsp);
        if (!ret) media->state = RTSP_STATE_READY;
    }
    return ret;
}

int rmcq_teardown(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;

    if ((media->teardown) && !rtsp_media_sinit(media)) ret = media->teardown(media, msg, rsp);
    media->state = RTSP_STATE_INIT;

    rtsp_client_set_kill((t_rtsp_client*)media->creator);

    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// received a response as a client (can not answer -> rsp = 0)

int rmcr_pause(t_rtsp_media* media, void* msg)
{
    int ret = RTSP_SUCCESS;
    if (media->state == RTSP_STATE_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, 0);
        if (ret == RTSP_SUCCESS) media->state = RTSP_STATE_READY;
    }
    return ret;
}

int rmcr_setup(t_rtsp_media* media, void* msg)
{
    int ret = RTSP_SUCCESS;

    ret = media->setup(media, msg, 0);

    if (ret == RTSP_SUCCESS) {
        switch (media->state) {
            case RTSP_STATE_NONE:
            case RTSP_STATE_INIT:
            case RTSP_STATE_READY:
                media->state = RTSP_STATE_READY;
            break;
            case RTSP_STATE_PLAYING:
                media->state = RTSP_STATE_PLAYING;
            break;
        }
    }

    return ret;
}

int rmcr_play(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;

    switch (media->state) {
        case RTSP_STATE_NONE:
        case RTSP_STATE_INIT:
            rtsp_response_error(rsp, RTSP_STATUS_METHOD_NOT_VALID_IN_THIS_STATE, NULL);
        break;
        case RTSP_STATE_READY:
        case RTSP_STATE_PLAYING:
            ret = media->play(media, msg, 0);
            if (ret == RTSP_SUCCESS) {
                media->state = RTSP_STATE_PLAYING;
            } else {
                // 3xx error codes go to init
                // 4xx error codes do not change
                if ((ret >= 300) && (ret < 400)) {
                    media->state = RTSP_STATE_INIT;
                }
            }
        break;
    }

    return ret;
}

int rmcr_teardown(t_rtsp_media* media, void* msg)
{
    int ret = RTSP_SUCCESS;

    if (media->teardown && !rtsp_media_sinit(media)) ret = media->teardown(media, msg, 0);
    media->state = RTSP_STATE_INIT;
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
    if (media == NULL || media->creator == NULL)
        return RTSP_NULL_POINTER;

    if (media->state == RTSP_STATE_PLAYING)
      return RTSP_WRONG_STATE;

    if (media->dosetup == NULL)
      return RTSP_UNSUPPORTED_METHOD;

    return media->dosetup(media, 0, 0);
}

int rtsp_media_play(t_rtsp_media* media)
{
    int ret = RTSP_NULL_POINTER;
    t_rtsp_client *client = media->creator;

    if (media && media->creator) {
        if (media->state == RTSP_STATE_READY) {
            if (media->doplay) ret = media->doplay(media, 0, 0);
            // TODO: does this really have to be done here?
            // check if kill / teardown is set because of HDCP error
            if (client->task & E_RTSP_CLIENT_TEARDOWN) {
                report(INFO "TEARDOWN CLIENT");
                rtsp_client_teardown(client, media->name);
            }
            if(client->task & E_RTSP_CLIENT_KILL) {
            	report(INFO "KILL CLIENT");
            	rtsp_client_close(client, false);
            }
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
        if (media->event) ret = media->event(media, event);
        switch (ret) {
            case RTSP_PAUSE: media->state = RTSP_STATE_READY; ret = RTSP_SUCCESS; break;
            case RTSP_CLOSE: media->state = RTSP_STATE_INIT; ret = RTSP_SUCCESS; break;
        }
    }
    return ret;
}
