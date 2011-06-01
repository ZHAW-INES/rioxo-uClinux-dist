/*
 * vtb_video.c
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */

#include "hdoipd.h"
#include "hoi_drv_user.h"
#include "vtb_video.h"
#include "hdoipd_fsm.h"
#include "box_sys.h"
#include "edid.h"
#include "rscp_server.h"
#include "multicast.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define TICK_TIMEOUT                    (hdoipd.eth_timeout)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)


int vtb_video_setup(t_rscp_media* media, t_rscp_req_setup* m, t_rscp_connection* rsp)
{
    int n, ret;
    t_multicast_cookie* cookie = media->cookie;
    t_edid edid_old;

    report(INFO "vtb_video_setup");

    media->result = RSCP_RESULT_READY;

    if (!hdoipd_state(HOID_VTB)) {
        // currently no video input active
        report(" ? not in state vtb");
        rscp_err_no_vtb(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // test if resource is available (only on first connection)
    if((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_VIDEO) == CLIENT_NOT_AVAILABLE)) {
        if (hdoipd_tstate(VTB_VID_MASK)) {
            report(" ? vtb busy");
            rscp_err_busy(rsp);
            return RSCP_REQUEST_ERROR;
        }
    }

    // get own MAC address
    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac)) != RSCP_SUCCESS) {
        report(" ? net_get_local_hwaddr failed");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // get own IP address
    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RSCP_SUCCESS) {
        report(" ? net_get_local_addr failed");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // get MAC address of next in rout
    if ((n = net_get_remote_hwaddr(hdoipd.listener.sockfd, "eth0", rsp->address, (uint8_t*)&cookie->remote.mac)) != RSCP_SUCCESS) {
        report(" ? net_get_remote_hwaddr failed");
        rscp_err_retry(rsp);
        return RSCP_REQUEST_ERROR;
    }

    if (check_client_availability(MEDIA_IS_VIDEO) == CLIENT_NOT_AVAILABLE) {
        if (!hdoipd_tstate(VTB_VID_MASK)) {
            // TODO: dont reload when already the same, store edid in file for next boot
            //       have a list of all contributing edid source (to test if it is already included in our edid)
            //
            // video source not  in use -> update
            ret = edid_read_file(&edid_old);
            if(ret == 0) {  // old EDID read
                if(edid_compare(&edid_old, (void*)m->edid.edid) == 0) { // new EDID
                    report(" i [EDID] new E-EDID received");
                    edid_write_file((void*)m->edid.edid);
                    hoi_drv_wredid(m->edid.edid);
                    edid_report((void*)m->edid.edid);
                } else {
                    report(" i [EDID] same E-EDID");
                }
            } else if(ret == -2) { // file doesn't exist
                report(" i [EDID] no E-EDID saved");
                edid_write_file((void*)m->edid.edid);
                hoi_drv_wredid(m->edid.edid);
                edid_report((void*)m->edid.edid);
            } else {
                perrno("edid_read_file() failed");
            }
        }

        // reserve resource
        hdoipd_set_vtb_state(VTB_VID_IDLE);
    }

    cookie->timeout = 0;
    cookie->alive_ping = 1;

    cookie->remote.address = rsp->address;
    cookie->remote.vid_port = PORT_RANGE_START(m->transport.client_port);
    m->transport.server_port = PORT_RANGE(hdoipd.local.vid_port, hdoipd.local.vid_port);
    m->transport.multicast = set_multicast_enable(reg_test("multicast_en", "true"));

    rscp_response_setup(rsp, &m->transport, media->sessionid);

    REPORT_RTX("TX", hdoipd.local, "->", cookie->remote, vid);

    return RSCP_SUCCESS;
}

int vtb_video_play(t_rscp_media* media, t_rscp_req_play* m, t_rscp_connection* rsp)
{
    int n;
    t_video_timing timing;
    t_rscp_rtp_format fmt;
    hdoip_eth_params eth;
    char dst_mac[6];

    t_multicast_cookie* cookie = media->cookie;

    report(INFO "vtb_video_play");

    media->result = RSCP_RESULT_PLAYING;

    if (!hdoipd_state(HOID_VTB)) {
        // we don't have the resource reserved
        report(" ? require state VTB");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    if(!get_multicast_enable() || (check_client_availability == CLIENT_NOT_AVAILABLE)) {
        if (!hdoipd_tstate(VTB_VID_IDLE)) {
            // we don't have the resource reserved
            report(" ? require state VTB_IDLE");
            rscp_err_server(rsp);
            return RSCP_REQUEST_ERROR;
        }
    }

    if (!hdoipd_rsc(RSC_VIDEO_IN)) {
        // currently no video input active
        report(" ? require active video input");
        media->result = RSCP_RESULT_NO_VIDEO_IN;
        rscp_err_no_source(rsp);
        hdoipd_set_vtb_state(VTB_VID_OFF);
        return RSCP_REQUEST_ERROR;
    }

    // test for valid resolution

    // start ethernet output
    if(get_multicast_enable()) {
        eth.ipv4_dst_ip = inet_addr(reg_get("multicast_group"));
        convert_ip_to_multicast_mac(inet_addr(reg_get("multicast_group")), dst_mac);
    }
    else {
        eth.ipv4_dst_ip = cookie->remote.address;
        for(n=0;n<6;n++) dst_mac[n] = cookie->remote.mac[n];
    }

    eth.ipv4_src_ip = hdoipd.local.address;
    eth.ipv4_tos = 0;
    eth.ipv4_ttl = 30;
    eth.packet_size = 1500;
    eth.rtp_ssrc = 0;
    for(n=0;n<6;n++) eth.src_mac[n] = hdoipd.local.mac[n];
    for(n=0;n<6;n++) eth.dst_mac[n] = dst_mac[n];
    eth.udp_dst_port = cookie->remote.vid_port;
    eth.udp_src_port = hdoipd.local.vid_port;

    // start vsi
    fmt.compress = m->format.compress;
    fmt.value = reg_get_int("advcnt-min");

    // probe config
    if ((n = hoi_drv_info(&timing, &fmt.value))) {
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    hoi_drv_get_mtime(&fmt.rtptime);

    // send timing
    rscp_response_play(rsp, media->sessionid, &fmt, &timing);

#ifdef VID_IN_PATH
    if (check_client_availability(MEDIA_IS_VIDEO) == CLIENT_NOT_AVAILABLE) {
        // activate vsi
        if ((n = hoi_drv_vsi(fmt.compress, 0, reg_get_int("bandwidth"), &eth, &timing, &fmt.value))) {
            return RSCP_REQUEST_ERROR;
        }
#endif
        // We are streaming Video now...
        hdoipd_set_vtb_state(VTB_VIDEO);
    }

    add_client_to_vtb(MEDIA_IS_VIDEO, cookie->remote.address);

    return RSCP_SUCCESS;
}

int vtb_video_teardown(t_rscp_media* media, t_rscp_req_teardown *m, t_rscp_connection* rsp)
{
    t_multicast_cookie* cookie = media->cookie;

    report(INFO "vtb_video_teardown");

    media->result = RSCP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_VIDEO|VTB_VID_IDLE)) {
#ifdef VID_IN_PATH
        if (check_client_availability(MEDIA_IS_VIDEO) == CLIENT_AVAILABLE_ONLY_ONE) {
            hdoipd_hw_reset(DRV_RST_VID_IN);
#endif
            hdoipd_set_vtb_state(VTB_VID_OFF);
        }
    }

    if (rsp) {
        rscp_response_teardown(rsp, media->sessionid);
    }

    cookie->timeout = 0;
    cookie->alive_ping = 1;

    remove_client_from_vtb(MEDIA_IS_VIDEO, cookie->remote.address);

    return RSCP_SUCCESS;
}

void vtb_video_pause(t_rscp_media *media)
{
    t_multicast_cookie* cookie = media->cookie;

    report(INFO "vtb_video_pause");

    media->result = RSCP_RESULT_PAUSE_Q;

    if (hdoipd_tstate(VTB_VIDEO)) {
#ifdef VID_IN_PATH
        if (check_client_availability(MEDIA_IS_VIDEO) == CLIENT_AVAILABLE_ONLY_ONE) {
            hdoipd_hw_reset(DRV_RST_VID_IN);
#endif
            hdoipd_set_vtb_state(VTB_VID_IDLE);
        }
    }

    remove_client_from_vtb(MEDIA_IS_VIDEO, cookie->remote.address);
}

int vtb_video_update(t_rscp_media UNUSED *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
{
    t_multicast_cookie* cookie = media->cookie;

    switch (m->event) {
        case EVENT_TICK:
            // reset timeout
            cookie->timeout = 0;
        break;
    }

    return RSCP_SUCCESS;
}

int vtb_video_event(t_rscp_media *media, uint32_t event)
{
    t_multicast_cookie* cookie = media->cookie;

    switch (event) {
        case EVENT_VIDEO_IN_ON:
            if (rscp_media_splaying(media)) {
                vtb_video_pause(media);
                rscp_server_update(media, EVENT_VIDEO_IN_OFF);
            }
            rscp_server_update(media, EVENT_VIDEO_IN_ON);
            return RSCP_PAUSE;
        break;
        case EVENT_VIDEO_IN_OFF:
            if (rscp_media_splaying(media)) {
                vtb_video_pause(media);
                rscp_server_update(media, EVENT_VIDEO_IN_OFF);
            }
            return RSCP_PAUSE;
        break;
        case EVENT_TICK:
            if (cookie->alive_ping) {
                cookie->alive_ping--;
            } else {
                cookie->alive_ping = TICK_SEND_ALIVE;
                // send tick we are alive (until something like rtcp is used)
                rscp_server_update(media, EVENT_TICK);
            }

            if (cookie->timeout <= TICK_TIMEOUT) {
                cookie->timeout++;

            } else {
                report(INFO "vtb_video_event: timeout");
                // timeout -> kill connection
                // server cannot kill itself -> add to kill list
                // (will be executed after all events are processed)
                cookie->timeout = 0;
                rscp_listener_add_kill(&hdoipd.listener, media);
            }
        break;
    }

    return RSCP_SUCCESS;
}

t_rscp_media vtb_video = {
    .name = "video",
    .owner = 0,
    .cookie = 0,
    .cookie_size = sizeof(t_multicast_cookie),
    .setup = (frscpm*)vtb_video_setup,
    .play = (frscpm*)vtb_video_play,
    .teardown = (frscpm*)vtb_video_teardown,
    .update = (frscpm*)vtb_video_update,
    .event = (frscpe*)vtb_video_event
};
