/*
 * rscp_media.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "rscp_media.h"
#include "rscp_listener.h"
#include "rscp_server.h"
#include "rscp_client.h"
#include "rscp_command.h"
#include "rscp_error.h"
#include "string.h"
#include "debug.h"

///////////////////////////////////////////////////////////////////////////////
// received a request as a server (can answer -> rsp != 0)


int rmsq_setup(t_rscp_media* _media, void* msg, t_rscp_connection* rsp)
{
    t_rscp_media *media = _media;
    int ret = RSCP_SERVER_ERROR;

    // creator is the rscp_server who started this connection
    // we get the rscp_listener from it
    media->top = ((t_rscp_server*)media->creator)->owner;

    // start new session?
    if (!_media->owner) {
        media = malloc(sizeof(t_rscp_media));
        if (!media) {
            report(ERROR "rmsq_setup.malloc: out of memory");
            return ret;
        }
        memcpy(media, _media, sizeof(t_rscp_media));
        media->owner = _media;
        if (media->cookie_size) {
            media->cookie = malloc(media->cookie_size);
            if (!media->cookie) {
                report(ERROR "rmsq_setup.malloc: out of memory");
                free(media);
                return ret;
            }
            memset(media->cookie, 0, media->cookie_size);
        }
        rscp_listener_create_sessionid(media->top, media->sessionid);
    }

    if (media->setup) ret = media->setup(media, msg, rsp);

    if (ret == RSCP_SUCCESS) {
        switch (media->state) {
            case RSCP_INIT:
            case RSCP_READY:
                media->state = RSCP_READY;
            break;
            case RSCP_PLAYING:
                media->state = RSCP_PLAYING;
            break;
        }
    }

    if (media != _media) {
        if (ret == RSCP_SUCCESS) {
            rscp_listener_add_session(media->top, media);
        } else {
            if (media->cookie_size) free(media->cookie);
            free(media);
        }
        _media->creator = 0;
    }

    // request properly handled
    if (ret == RSCP_REQUEST_ERROR) {
        ret = RSCP_SUCCESS;
    }

    return ret;
}

int rmsq_play(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SERVER_ERROR;

    // needs to be a session
    if (!media->owner) return RSCP_CLIENT_ERROR;

    switch (media->state) {
        case RSCP_INIT:
            rscp_response_error(rsp, RSCP_SC_METHOD_NVIS, "Method not Valid in this State");
        break;
        case RSCP_READY:
        case RSCP_PLAYING:
            if (media->play) ret = media->play(media, msg, rsp);
            if (ret == RSCP_SUCCESS) {
                media->state = RSCP_PLAYING;
            } else {
                // 3xx error codes go to init
                // 4xx error codes do not change
                if ((ret >= 300) && (ret < 400)) {
                    media->state = RSCP_INIT;
                }
            }
        break;
    }

    // request properly handled
    if (ret == RSCP_REQUEST_ERROR) {
        ret = RSCP_SUCCESS;
    }

    return ret;
}

int rmsq_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SERVER_ERROR;

    // needs to be a session
    if (!media->owner) return RSCP_CLIENT_ERROR;

    if (media->teardown) ret = media->teardown(media, msg, rsp);
    media->state = RSCP_INIT;
    if (media->cookie_size) free(media->cookie);
    rscp_listener_remove_session(media->top, media);
    free(media);
    return ret;
}

int rmsq_hello(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->hello) ret = media->hello(media, msg, rsp);
    return ret;
}

// rscp media server
int rmsq_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->state == RSCP_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, rsp);
        if (!ret) media->state = RSCP_READY;
    }
    return ret;
}

// rscp media server request
int rmsq_update(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->update) ret = media->update(media, msg, rsp);
    switch (ret) {
    	case RSCP_PAUSE: media->state = RSCP_READY; ret = RSCP_SUCCESS; break;
    	case RSCP_CLOSE: media->state = RSCP_INIT; ret = RSCP_SUCCESS; break;
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
int rmsr_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;

    // needs to be a session
    if (!media->owner) return RSCP_CLIENT_ERROR;

    if (media->teardown) ret = media->teardown(media, msg, 0);
    media->state = RSCP_INIT;
    if (media->cookie_size) free(media->cookie);
    free(media);
    return ret;
}

int rmsr_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->state == RSCP_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, 0);
        if (!ret) media->state = RSCP_READY;
    }
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// received a request as a client (can answer -> rsp != 0)


int rmcq_update(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->update) ret = media->update(media, msg, rsp);
    switch (ret) {
    	case RSCP_PAUSE: media->state = RSCP_READY; ret = RSCP_SUCCESS; break;
    	case RSCP_CLOSE: media->state = RSCP_INIT; ret = RSCP_SUCCESS; break;
    }
    return ret;
}

int rmcq_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->state == RSCP_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, rsp);
        if (!ret) media->state = RSCP_READY;
    }
    return ret;
}

int rmcq_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->teardown) ret = media->teardown(media, msg, rsp);
    media->state = RSCP_INIT;

    ((t_rscp_client*)media->creator)->kill = true;

    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// received a response as a client (can not answer -> rsp = 0)


int rmcr_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->state == RSCP_PLAYING) {
        if (media->pause) ret = media->pause(media, msg, 0);
        if (!ret) media->state = RSCP_READY;
    }
    return ret;
}

int rmcr_setup(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;

    ret = media->setup(media, msg, 0);

    if (ret == RSCP_SUCCESS) {
        switch (media->state) {
            case RSCP_INIT:
            case RSCP_READY:
                media->state = RSCP_READY;
            break;
            case RSCP_PLAYING:
                media->state = RSCP_PLAYING;
            break;
        }
    }

    return ret;
}

int rmcr_play(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;

    switch (media->state) {
        case RSCP_INIT:
            rscp_response_error(rsp, RSCP_SC_METHOD_NVIS, "Method not Valid in this State");
        break;
        case RSCP_READY:
        case RSCP_PLAYING:
            ret = media->play(media, msg, 0);
            if (ret == RSCP_SUCCESS) {
                media->state = RSCP_PLAYING;
            } else {
                // 3xx error codes go to init
                // 4xx error codes do not change
                if ((ret >= 300) && (ret < 400)) {
                    media->state = RSCP_INIT;
                }
            }
        break;
    }

    return ret;
}

int rmcr_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->teardown) ret = media->teardown(media, msg, 0);
    media->state = RSCP_INIT;
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// state manipulation (local)

int rscp_media_ready(t_rscp_media* media)
{
    int ret = RSCP_SUCCESS;
    if (media) {
        if (media->ready) ret = media->ready(media);
    }
    return ret;
}

int rscp_media_setup(t_rscp_media* media)
{
    int ret = RSCP_NULL_POINTER;
    if (media) {
        if (media->dosetup) ret = media->dosetup(media);
    }
    return ret;
}

int rscp_media_play(t_rscp_media* media)
{
    int ret = RSCP_NULL_POINTER;
    if (media) {
        if (media->doplay) ret = media->doplay(media);
    }
    return ret;
}

int rscp_media_event(t_rscp_media* media, uint32_t event)
{
    int ret = RSCP_NULL_POINTER;
    if (media) {
    	if (media->state != RSCP_INIT) {
			if (media->event) ret = media->event(media, event);
			switch (ret) {
				case RSCP_PAUSE: media->state = RSCP_READY; ret = RSCP_SUCCESS; break;
				case RSCP_CLOSE: media->state = RSCP_INIT; ret = RSCP_SUCCESS; break;
			}
    	}
    }
    return ret;
}
