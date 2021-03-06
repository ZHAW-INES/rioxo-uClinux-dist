/*
 * usb_media.c
 *
 *  Created on: 21.03.2012
 *      Author: buan
 */

#include <stdint.h>

#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hoi_drv_user.h"
#include "rtsp_client.h"
#include "rtsp_command.h"
#include "rtsp_parse_header.h"
#include "rtsp_server.h"
#include "rtsp_string.h"
#include "usb_media.h"
#include "vrb_video.h"
#include "box_sys.h"
#include "box_sys_vtb.h"

#define TICK_TIMEOUT                    (hdoipd.eth_timeout * 2)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

static char usb_host_ip[50];

int usb_describe(t_rtsp_media *media, void *_data, t_rtsp_connection *con)
{
    t_rtsp_req_describe *data = _data;

    if (!data)
        return -1;

    rtsp_handle_describe_generic(media, data, con);

    /* Add media specific content */

    /* TODO: List all available media-controls */

    rtsp_send(con);

    return 0;
}

int usb_setup(t_rtsp_media UNUSED *media, t_rtsp_req_setup* m, t_rtsp_connection* rsp)
{
    report(VRB_METHOD "usb_setup");

    strncpy(usb_host_ip, m->transport.destination_str, 50);

    //TODO: set port in usbip
    m->transport.port = htons(3242);

    rtsp_response_usb_setup(rsp, &m->transport, media->sessionid);

    media->result = RTSP_RESULT_READY;

    return RTSP_SUCCESS;
}

int usb_play(t_rtsp_media UNUSED *media, t_rtsp_req_play* m, t_rtsp_connection* rsp)
{
    report(VRB_METHOD "usb_play");

    if (strcmp(m->usb.mouse,    "")) {
        usb_attach_device(&hdoipd.usb_devices, usb_host_ip, m->usb.mouse, "mouse");
    }
    if (strcmp(m->usb.keyboard, "")) {
        usb_attach_device(&hdoipd.usb_devices, usb_host_ip, m->usb.keyboard, "keyboard");
    }
    if (strcmp(m->usb.storage,  "")) {
        usb_attach_device(&hdoipd.usb_devices, usb_host_ip, m->usb.storage, "storage");
    }

    rtsp_response_usb_play(rsp, media->sessionid);

    media->result = RTSP_RESULT_PLAYING;

    return RTSP_SUCCESS;
}

int usb_teardown(t_rtsp_media *media UNUSED, t_rtsp_req_teardown* m UNUSED, t_rtsp_connection* rsp)
{
    report(VRB_METHOD "usb_teardown");

    usb_detach_device(0);
    usb_detach_device(1);
    
    if (rsp) {
        rtsp_response_usb_teardown(rsp);
    }

    media->result = RTSP_RESULT_TEARDOWN;

    return RTSP_SUCCESS;
}

int usb_dosetup(t_rtsp_media *media, t_rtsp_usb* m UNUSED, void* rsp UNUSED)
{
    t_rtsp_client *client = media->creator;
    t_rtsp_transport transport;
    t_rtsp_header_common common;
    u_rtsp_header buf;
    int ret;

    report(VTB_METHOD "usb_dosetup");

    rtsp_default_transport(&transport);

    if (!client) { report(ERROR "usb_dosetup: no client"); return RTSP_NULL_POINTER; }

    transport.multicast = false;
    transport.port = htons(3242);
 
    if (reg_test("system-dhcp", "true")) {
        strncpy(transport.destination_str, reg_get("system-hostname"), 49);
    } else {
        strncpy(transport.destination_str, reg_get("system-ip"), 49);
    }

    // send USB SETUP
    rtsp_request_usb_setup(&client->con, client->uri, &transport);

    // response
    rtsp_default_response_setup((void*)&buf);
    ret = rtsp_parse_response(&client->con, tab_response_setup, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    if (ret == RTSP_SUCCESS) {

        strcpy(media->sessionid, common.session);

        switch (media->state) {
            case RTSP_STATE_INIT:
            case RTSP_STATE_READY:
                media->state = RTSP_STATE_READY;
            break;
            case RTSP_STATE_PLAYING:
                media->state = RTSP_STATE_PLAYING;
            break;
        }
    } else {
        report(ERROR "USB Setup Response ERROR");
    }

    media->result = RTSP_RESULT_READY;

    return ret;
}

int usb_doplay(t_rtsp_media *media, t_rtsp_usb* m, void* rsp UNUSED)
{
    t_rtsp_client *client = media->creator;
    u_rtsp_header buf;
    int n, ret;

    report(VTB_METHOD "usb_doplay");

    if (!client) return RTSP_NULL_POINTER;

    // send USB PLAY
    rtsp_request_usb_play(&client->con, client->uri, media->sessionid, m->mouse, m->keyboard, m->storage);

    // response
    rtsp_default_response_play((void*)&buf);
    n = rtsp_parse_response(&client->con, tab_response_play, (void*)&buf, 0, CFG_RSP_TIMEOUT);

    if (n == RTSP_SUCCESS) {
        ret = RTSP_SUCCESS;
    } else {
        report(ERROR "USB Play Response ERROR");
        ret = RTSP_CLIENT_ERROR;
    }

    switch (media->state) {
        case RTSP_STATE_INIT:
            // TODO: send error message
        break;
        case RTSP_STATE_READY:
        case RTSP_STATE_PLAYING:
            media->state = RTSP_STATE_PLAYING;
        break;
    }

    media->result = RTSP_RESULT_PLAYING;

    return ret;
}

int usb_doteardown(t_rtsp_media *media, t_rtsp_usb *m UNUSED, void *rsp UNUSED)
{
    t_rtsp_client *client = media->creator;
    u_rtsp_header buf;
    int n, ret;

    report(VTB_METHOD "usb_doteardown");

    if (!client) return RTSP_NULL_POINTER;

    // send USB TEARDOWN
    rtsp_request_usb_teardown(&client->con, client->uri, media->sessionid);

    // response
    rtsp_default_response_teardown((void*)&buf);
    n = rtsp_parse_response(&client->con, tab_response_teardown, (void*)&buf, 0, CFG_RSP_TIMEOUT);

    if (n == RTSP_SUCCESS) {
        ret = RTSP_SUCCESS;
    } else {
        report(ERROR "USB Teardown Response ERROR");
        ret = RTSP_CLIENT_ERROR;
    }

    rtsp_client_close(client, false);

    // TODO: correct handling of mouse and keyboard and storage teardown separate
    // also in rmsq teardown for device side
    media->state = RTSP_STATE_INIT;

    media->result = RTSP_RESULT_TEARDOWN;

    return ret;
}


t_rtsp_media usb_media = {
    .name = "usb",
    .owner = 0,
    .cookie = 0,
    .options = (frtspm*)box_sys_options,
    .describe = usb_describe,
    .get_parameter = (frtspm*)box_sys_vtb_get_parameter,
    .setup = (frtspm*)usb_setup,
    .play = (frtspm*)usb_play,
    .teardown = (frtspm*)usb_teardown,
    .dosetup = (frtspm*)usb_dosetup,
    .doplay = (frtspm*)usb_doplay,
    .doteardown = (frtspm*)usb_doteardown
};
