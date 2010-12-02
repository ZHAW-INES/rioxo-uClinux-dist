/*
 * rtsp_media.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */
#include "rtsp_media.h"
#include "rtsp_command.h"
#include "rtsp_error.h"
#include "string.h"

int rms_option(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    return media->option(media, msg, rsp);
}

int rms_setup(t_rtsp_media* _media, void* msg, t_rtsp_connection* rsp)
{
    t_rtsp_media* media = _media;
    int ret;

    // start new session?
    if (!_media->owner) {
        media = malloc(sizeof(t_rtsp_media));
        memcpy(media, _media, sizeof(t_rtsp_media));
        media->owner = _media;
        snprintf(media->sessionid, 20, "%d", rand());
    }

    ret = media->setup(media, msg, rsp);

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
            media->session = list_add(_media->session, media);
        } else {
            free(media);
        }
    }

    return ret;
}

int rms_play(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret;

    // needs to be a session
    if (!media->owner) return RTSP_CLIENT_ERROR;

    switch (media->state) {
        case RTSP_INIT:
            rtsp_response_error(rsp, RTSP_SC_METHOD_NVIS, "Method not Valid in this State");
            ret = RTSP_SC_METHOD_NVIS;
        break;
        case RTSP_READY:
        case RTSP_PLAYING:
            ret = media->play(media, msg, rsp);
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

int rms_teardown(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    // needs to be a session
    if (!media->owner) return RTSP_CLIENT_ERROR;

    int ret = media->teardown(media, msg, rsp);
    media->state = RTSP_INIT;
    list_remove(media->session);
    free(media);
    return ret;
}

// rtsp media client
int rmc_option(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
    return ret;
}

int rmc_setup(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
printf("rmc_setup\n");
    ret = media->setup(media, msg, rsp);

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

int rmc_play(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;

    ret = media->setup(media, msg, rsp);

    switch (media->state) {
        case RTSP_INIT:
            rtsp_response_error(rsp, RTSP_SC_METHOD_NVIS, "Method not Valid in this State");
            ret = RTSP_SC_METHOD_NVIS;
        break;
        case RTSP_READY:
        case RTSP_PLAYING:
            ret = media->play(media, msg, rsp);
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

int rmc_teardown(t_rtsp_media* media, void* msg, t_rtsp_connection* rsp)
{
    int ret = RTSP_SUCCESS;
    ret = media->teardown(media, msg, rsp);
    media->state = RTSP_INIT;
    return ret;
}

static int rtsp_media_compare(t_rtsp_media* media, char* session)
{
    return strcmp(media->sessionid, session);
}

t_rtsp_media* rtsp_media_get(t_rtsp_media* media, char* session)
{
    return list_find(media->session, rtsp_media_compare, session);
}

