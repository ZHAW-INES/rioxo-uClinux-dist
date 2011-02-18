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
#include "rscp_command.h"
#include "rscp_error.h"
#include "string.h"
#include "debug.h"

int rmsq_option(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void UNUSED *owner)
{
    return media->option(media, msg, rsp);
}

int rmsq_setup(t_rscp_media* _media, void* msg, t_rscp_connection* rsp, void* owner)
{
    t_rscp_media* media = _media;
    int ret = RSCP_SERVER_ERROR;

    // start new session?
    if (!_media->owner) {
        media = malloc(sizeof(t_rscp_media));
        memcpy(media, _media, sizeof(t_rscp_media));
        media->owner = _media;
        rscp_listener_create_sessionid(owner, media->sessionid);
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
            rscp_listener_add_session(owner, media);
        } else {
            free(media);
        }
    }

    // request properly handled
    if (ret == RSCP_REQUEST_ERROR) {
        ret = RSCP_SUCCESS;
    }

    return ret;
}

int rmsq_play(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void UNUSED *owner)
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

int rmsq_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void* owner)
{
    int ret = RSCP_SERVER_ERROR;

    // needs to be a session
    if (!media->owner) return RSCP_CLIENT_ERROR;

    if (media->teardown) ret = media->teardown(media, msg, rsp);
    media->state = RSCP_INIT;
    rscp_listener_remove_session(owner, media);
    free(media);
    return ret;
}

/** Teardown server
 *
 * Note: the listener(owner) is locked when this function is called
 * Sessions are removed by the caller
 *
 */
int rmsr_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void UNUSED *owner)
{
    int ret = RSCP_SUCCESS;

    // needs to be a session
    if (!media->owner) return RSCP_CLIENT_ERROR;

    if (media->teardown) ret = media->teardown(media, 0, rsp);
    media->state = RSCP_INIT;
    return ret;
}

int rmsq_hello(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void UNUSED *owner)
{
    int ret = RSCP_SUCCESS;
    if (media->hello) ret = media->hello(media, msg, rsp);
    return ret;
}

// rscp media server
int rmsq_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void UNUSED *owner)
{
    int ret = RSCP_SUCCESS;
    if (media->state == RSCP_PLAYING) {
        if (media->pause_q) ret = media->pause_q(media, msg, rsp);
        if (!ret) media->state = RSCP_READY;
    }
    return ret;
}

// rscp media server request
int rmsq_update(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void UNUSED *owner)
{
    int ret = RSCP_SUCCESS;
    if (media->update) ret = media->update(media, msg, rsp);
    if (ret) media->state = RSCP_READY;
    return ret;
}


// rscp media server response
int rmsr_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->state == RSCP_PLAYING) {
        if (media->pause_r) ret = media->pause_r(media, msg, rsp);
        if (!ret) media->state = RSCP_READY;
    }
    return ret;
}

// rscp media client
int rmcq_update(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void UNUSED *owner)
{
    int ret = RSCP_SUCCESS;
    if (media->update) ret = media->update(media, msg, rsp);
    if (ret) media->state = RSCP_READY;
    return ret;
}

// rscp media client
int rmcq_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void UNUSED *owner)
{
    int ret = RSCP_SUCCESS;
    if (media->state == RSCP_PLAYING) {
        if (media->pause_q) ret = media->pause_q(media, msg, rsp);
        if (!ret) media->state = RSCP_READY;
    }
    return ret;
}

// rscp media client
int rmcr_pause(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;
    if (media->state == RSCP_PLAYING) {
        if (media->pause_r) ret = media->pause_r(media, msg, rsp);
        if (!ret) media->state = RSCP_READY;
    }
    return ret;
}

// rscp media client
int rmcr_option(t_rscp_media UNUSED *media, void UNUSED *msg, t_rscp_connection UNUSED *rsp)
{
    int ret = RSCP_SUCCESS;
    return ret;
}

int rmcr_setup(t_rscp_media* media, void* msg, t_rscp_connection* rsp)
{
    int ret = RSCP_SUCCESS;

    ret = media->setup(media, msg, rsp);

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
            ret = media->play(media, msg, rsp);
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
    if (media->teardown) ret = media->teardown(media, 0, rsp);
    media->state = RSCP_INIT;
    return ret;
}

int rmcq_teardown(t_rscp_media* media, void* msg, t_rscp_connection* rsp, void UNUSED *owner)
{
    int ret = RSCP_SUCCESS;
    if (media->teardown) ret = media->teardown(media, msg, rsp);
    media->state = RSCP_INIT;
    return ret;
}

int rscp_media_force_ready(t_rscp_media* media)
{
    if (media->state == RSCP_PLAYING) {
        media->state = RSCP_READY;
    }
    return RSCP_SUCCESS;
}

int rscp_media_force_init(t_rscp_media* media)
{
    media->state = RSCP_INIT;
    return RSCP_SUCCESS;
}

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
        if (media->event) ret = media->event(media, event);
    }
    return ret;
}
