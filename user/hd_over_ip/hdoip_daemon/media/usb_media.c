/*
 * usb_media.c
 *
 *  Created on: 21.03.2012
 *      Author: buan
 */

#include <stdint.h>
#include "hoi_drv_user.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "usb_media.h"
#include "rscp_string.h"
#include "vrb_video.h"
#include "rscp_parse_header.h"

#define TICK_TIMEOUT                    (hdoipd.eth_timeout * 2)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

//TODO: do not use global variables
char        usb_host_ip[50];
uint32_t    usb_host_port;
int         timer;
int         alive_ping;

int usb_setup(t_rscp_media UNUSED *media, t_rscp_req_setup* m, t_rscp_connection* rsp)
{
    report(VRB_METHOD "usb_setup");
    
    strncpy(usb_host_ip, m->transport.usb_host_ip, 49);
    usb_host_port = m->transport.usb_host_port;

    //TODO: set port in usbip
    m->transport.usb_host_port = htons(3240);

    rscp_response_usb_setup(rsp, &m->transport, media->sessionid);

    media->result = RSCP_RESULT_READY;

    return RSCP_SUCCESS;
}

int usb_play(t_rscp_media UNUSED *media, t_rscp_req_play* m, t_rscp_connection* rsp)
{
    int i;

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

    rscp_response_usb_play(rsp, media->sessionid);

    media->result = RSCP_RESULT_PLAYING;

    return RSCP_SUCCESS;
}

int usb_teardown(t_rscp_media UNUSED *media, t_rscp_req_teardown* m, t_rscp_connection* rsp)
{
    report(VRB_METHOD "usb_teardown");

    usb_detach_device(&hdoipd.usb_devices, 0);
    usb_detach_device(&hdoipd.usb_devices, 1);
    
    if (rsp) {
        rscp_response_usb_teardown(rsp, media->sessionid);
    }

    media->result = RSCP_RESULT_TEARDOWN;

    return RSCP_SUCCESS;
}

int usb_dosetup(t_rscp_media *media, t_rscp_usb* UNUSED m, void* UNUSED rsp)
{
    t_rscp_client *client = media->creator;
    t_rscp_transport transport;
    t_rscp_header_common common;
    u_rscp_header buf;
    int ret;

    report(VTB_METHOD "usb_dosetup");

    rscp_default_transport(&transport);

    if (!client) { report(ERROR "usb_dosetup: no client"); return RSCP_NULL_POINTER; }

    transport.multicast = false;
    transport.usb_host_port = htons(3242);
 
    if (reg_test("system-dhcp", "true")) {
        strncpy(transport.usb_host_ip, reg_get("system-hostname"), 49);
    } else {
        strncpy(transport.usb_host_ip, reg_get("system-ip"), 49);
    }

    // send USB SETUP
    rscp_request_usb_setup(&client->con, client->uri, &transport, client->media->sessionid);

    // response
    rscp_default_response_setup((void*)&buf);
    ret = rscp_parse_response(&client->con, tab_response_setup, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    if (ret == RSCP_SUCCESS) {

        strcpy(client->media->sessionid, common.session);

        switch (media->state) {
            case RSCP_INIT:
            case RSCP_READY:
                media->state = RSCP_READY;
            break;
            case RSCP_PLAYING:
                media->state = RSCP_PLAYING;
            break;
        }
    } else {
        report(ERROR "USB Setup Response ERROR");
    }

    media->result = RSCP_RESULT_READY;

    return ret;
}

int usb_doplay(t_rscp_media *media, t_rscp_usb* m, void* UNUSED rsp)
{
    t_rscp_client *client = media->creator;
    u_rscp_header buf;
    int n, ret;

    report(VTB_METHOD "usb_doplay");

    if (!client) return RSCP_NULL_POINTER;

    // send USB PLAY
    rscp_request_usb_play(&client->con, client->uri, client->media->sessionid, m->mouse, m->keyboard, m->storage);

    // response
    rscp_default_response_play((void*)&buf);
    n = rscp_parse_response(&client->con, tab_response_play, (void*)&buf, 0, CFG_RSP_TIMEOUT);

    if (n == RSCP_SUCCESS) {
        ret = RSCP_SUCCESS;
    } else {
        report(ERROR "USB Play Response ERROR");
        ret = RSCP_CLIENT_ERROR;
    }

    switch (media->state) {
        case RSCP_INIT:
            // TODO: send error message
        break;
        case RSCP_READY:
        case RSCP_PLAYING:
            media->state = RSCP_PLAYING;
        break;
    }

    media->result = RSCP_RESULT_PLAYING;

    return ret;
}

int usb_doteardown(t_rscp_media *media, t_rscp_usb* UNUSED m, void* UNUSED rsp)
{
    t_rscp_client *client = media->creator;
    u_rscp_header buf;
    int n, ret;

    report(VTB_METHOD "usb_doteardown");

    if (!client) return RSCP_NULL_POINTER;

    // send USB TEARDOWN
    rscp_request_usb_teardown(&client->con, client->uri, client->media->sessionid);

    // response
    rscp_default_response_teardown((void*)&buf);
    n = rscp_parse_response(&client->con, tab_response_teardown, (void*)&buf, 0, CFG_RSP_TIMEOUT);

    if (n == RSCP_SUCCESS) {
        ret = RSCP_SUCCESS;
    } else {
        report(ERROR "USB Teardown Response ERROR");
        ret = RSCP_CLIENT_ERROR;
    }

    rscp_client_close(client);

    // TODO: correct handling of mouse and keyboard and storage teardown separate
    // also in rmsq teardown for device side
    media->state = RSCP_INIT;

    media->result = RSCP_RESULT_TEARDOWN;

    return ret;
}

int usb_update(t_rscp_media UNUSED *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
{
    if (m->event == EVENT_TICK) {
        // reset timeout
        timer = 0;
    }

    return RSCP_SUCCESS;
}

int usb_event(t_rscp_media *media, uint32_t event)
{
    t_rscp_client *client = media->creator;

    if (!client) { report(ERROR "usb_event: no client") return RSCP_CLIENT_ERROR; }

    if (event == EVENT_TICK) {
        if (alive_ping) {
            alive_ping--;
        } else {
            alive_ping = TICK_SEND_ALIVE;
            // send tick we are alive (until something like rtcp is used)
            if reg_test("mode-start", "vtb") {
                rscp_server_update(media, EVENT_TICK);
            } else {
               rscp_client_update(client, EVENT_TICK);
            }
        }

        if (timer <= TICK_TIMEOUT) {
            timer++;
        } else {
           report(INFO "usb_event: timeout");
            // timeout -> kill connection
            // server cannot kill itself -> add to kill list
            // (will be executed after all events are processed)
            timer = 0;

            if reg_test("mode-start", "vtb") {
                rscp_listener_add_kill(&hdoipd.listener, media);
            } else {
                rscp_client_set_kill(client);
            }
        }
    }

    return RSCP_SUCCESS;
}


t_rscp_media usb_media = {
    .name = "usb",
    .owner = 0,
    .cookie = 0,
    .setup = (frscpm*)usb_setup,
    .play = (frscpm*)usb_play,
    .teardown = (frscpm*)usb_teardown,
    .dosetup = (frscpm*)usb_dosetup,
    .doplay = (frscpm*)usb_doplay,
    .doteardown = (frscpm*)usb_doteardown,
    .update = (frscpm*)usb_update,
    .event = (frscpe*)usb_event
};
