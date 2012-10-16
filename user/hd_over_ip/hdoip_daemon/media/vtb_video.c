/*
 * vtb_video.c
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */

#include "hdoipd.h"
#include "hdoipd_osd.h"
#include "hoi_drv_user.h"
#include "vtb_video.h"
#include "hdoipd_fsm.h"
#include "box_sys.h"
#include "edid_merge.h"
#include "rscp_server.h"
#include "hdcp.h"
#include "multicast.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define TICK_TIMEOUT_MULTICAST          (hdoipd.eth_timeout * 2)
#define TICK_TIMEOUT_UNICAST            (hdoipd.eth_timeout)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

int vtb_video_hdcp(t_rscp_media* media, t_rscp_req_hdcp* m, t_rscp_connection* rsp)
{
	int ret;

	t_multicast_cookie* cookie = media->cookie;
    ret = hdcp_ske_s(media, m, rsp);

    cookie->timeout = 0;
    cookie->alive_ping = 1;

    return ret;
}

int vtb_video_setup(t_rscp_media* media, t_rscp_req_setup* m, t_rscp_connection* rsp)
{
    int hdcp;

    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_video_setup");

    set_multicast_enable(reg_test("multicast_en", "true"));

    media->result = RSCP_RESULT_READY;

    if (!hdoipd_state(HOID_VTB)) {
        // currently no video input active
        report(" ? not in state vtb");
        rscp_err_no_vtb(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // get own MAC address
    if (net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac) != RSCP_SUCCESS) {
        report(" ? net_get_local_hwaddr failed");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // get own IP address
    if (net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address) != RSCP_SUCCESS) {
        report(" ? net_get_local_addr failed");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // get MAC address of next in rout
    if (net_get_remote_hwaddr(hdoipd.listener.sockfd, "eth0", rsp->address, (uint8_t*)&cookie->remote.mac) != RSCP_SUCCESS) {
        report(" ? net_get_remote_hwaddr failed");
        rscp_err_retry(rsp);
        return RSCP_REQUEST_ERROR;
    }

    if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_VIDEO) == CLIENT_NOT_AVAILABLE)) {
        // reserve resource
        hdoipd_set_vtb_state(VTB_VID_IDLE);
    }

    cookie->timeout = 0;
    cookie->alive_ping = 1;

    //check if hdcp is forced by HDMI, user or client (over RSCP)
    if (reg_test("hdcp-force", "true") || hdoipd_rsc(RSC_VIDEO_IN_HDCP) || (m->hdcp.hdcp_on==1)) {
    	m->hdcp.hdcp_on = 1;
    	hdoipd.hdcp.enc_state = HDCP_ENABLED;
    }

    cookie->remote.address = rsp->address;
    cookie->remote.vid_port = PORT_RANGE_START(m->transport.client_port);
    m->transport.server_port = PORT_RANGE(hdoipd.local.vid_port, hdoipd.local.vid_port);
    m->transport.multicast = get_multicast_enable();

    rscp_response_setup(rsp, &m->transport, media->sessionid, &m->hdcp); //respond message to setup message

    REPORT_RTX("TX", hdoipd.local, "->", cookie->remote, vid);

    return RSCP_SUCCESS;
}

int vtb_video_play(t_rscp_media* media, t_rscp_req_play* m, t_rscp_connection* rsp)
{
    int n, hdcp;
    t_video_timing timing;
    t_rscp_rtp_format fmt;
    hdoip_eth_params eth;
    char dst_mac[6];
    uint32_t bandwidth;
    uint32_t chroma;

    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_video_play");

    media->result = RSCP_RESULT_PLAYING;

    if (!hdoipd_state(HOID_VTB)) {
        // we don't have the resource reserved
        report(" ? require state VTB");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    if (!hdoipd_rsc(RSC_VIDEO_IN)) {
        // currently no video input active
        report(" ? require active video input");
        media->result = RSCP_RESULT_NO_VIDEO_IN;
        rscp_err_no_source(rsp);
        //hdoipd_set_vtb_state(VTB_VID_OFF);
        hdoipd_set_vtb_state(VTB_VID_IDLE);
        return RSCP_REQUEST_ERROR;
    }

    //check if Video HDCP status is unchanged, else return ERROR
    report("TEST HDCP status before start video play!");
    if  (hdoipd.hdcp.enc_state && !(get_hdcp_status() & HDCP_ETO_VIDEO_EN)){
    	if (hdoipd.hdcp.ske_executed){
    	    hoi_drv_hdcp(hdoipd.hdcp.keys); //write keys to kernel
    	    hoi_drv_hdcp_viden_eto();
    	    report(INFO "Video encryption enabled (eto)!");
    	}
    	else {
    	    report(" ? Video HDCP status has changed after SKE");
    	    rscp_err_hdcp(rsp);
    	    return RSCP_REQUEST_ERROR;
    	}
    }

    // test for valid resolution

    // start ethernet output
    if(get_multicast_enable()) {
        eth.ipv4_dst_ip = inet_addr(reg_get("multicast_group"));
        convert_ip_to_multicast_mac(inet_addr(reg_get("multicast_group")), dst_mac);
        report(INFO "sending multicast to : %s", reg_get("multicast_group"));
    }
    else {
        eth.ipv4_dst_ip = cookie->remote.address;
        for(n=0;n<6;n++) dst_mac[n] = cookie->remote.mac[n];
        report(INFO "sending unicast to : %s", reg_get("remote-uri"));
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
    if (hoi_drv_info(&timing, &fmt.value, 0)) {
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    hoi_drv_get_mtime(&fmt.rtptime);

    bandwidth = (uint32_t)reg_get_int("bandwidth");             // bandwidth in byte/s 
    chroma    = (uint32_t)reg_get_int("chroma-bandwidth");      // percent of chroma bandwidth (0 .. 100)


    // use 4 ADV212 if 1080i and bandwidth >= 50Mbit/s
    if ((timing.width == 1920) && (timing.height == 540) && (bandwidth >= (uint32_t)((uint64_t)(25+25*chroma/100)*(1048576)/8))) {
        fmt.value = 4;
    }

    // limit bandwidth to 120Mbit/s for 1080i
    if ((timing.width == 1920) && (timing.height == 540) && (bandwidth > (uint32_t)((uint64_t)(60+60*chroma/100)*(1048576)/8))) {
        bandwidth = (uint32_t)((uint64_t)(60+60*chroma/100)*(1048576)/8);
    }

    // limit bandwidth to 60Mbit/s for 576i and 480i
    if ((timing.width == 720) && ((timing.height == 240) || (timing.height == 288)) && (bandwidth > (uint32_t)((uint64_t)(30+30*chroma/100)*(1048576)/8))) {
        bandwidth = (uint32_t)((uint64_t)(30+30*chroma/100)*(1048576)/8);
    }

    // send timing
    rscp_response_play(rsp, media->sessionid, &fmt, &timing);

    if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_VIDEO) == CLIENT_NOT_AVAILABLE)) {
        // activate vsi
#ifdef VID_IN_PATH
        if (hoi_drv_vsi(fmt.compress, chroma, 0, bandwidth, &eth, &timing, &fmt.value)) {
            return RSCP_REQUEST_ERROR;
        }
#endif
        // We are streaming Video now...
        hdoipd_set_vtb_state(VTB_VIDEO);
    }

    add_client_to_vtb(MEDIA_IS_VIDEO, cookie->remote.address);

    osd_permanent(false);

    return RSCP_SUCCESS;
}

int vtb_video_teardown(t_rscp_media* media, t_rscp_req_teardown UNUSED *m, t_rscp_connection* rsp)
{

    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_video_teardown");

    media->result = RSCP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_VIDEO|VTB_VID_IDLE)) {
        if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_VIDEO) == CLIENT_AVAILABLE_ONLY_ONE)) {
#ifdef VID_IN_PATH
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

    multicast_remove_edid(cookie->remote.address);
    remove_client_from_vtb(MEDIA_IS_VIDEO, cookie->remote.address);

    hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
    osd_permanent(true);
    osd_printf("vtb.video connection stopped...\n");

    return RSCP_SUCCESS;
}

void vtb_video_pause(t_rscp_media *media)
{
    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_video_pause");

    media->result = RSCP_RESULT_PAUSE_Q;

    if (hdoipd_tstate(VTB_VIDEO)) {
        if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_VIDEO) == CLIENT_AVAILABLE_ONLY_ONE)) {
#ifdef VID_IN_PATH
            hdoipd_hw_reset(DRV_RST_VID_IN);
#endif
            hdoipd_set_vtb_state(VTB_VID_IDLE);

            hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
            osd_permanent(true);
            osd_printf("Video input lost...");

        }
    }

    remove_client_from_vtb(MEDIA_IS_VIDEO, cookie->remote.address);
}

int vtb_video_ext_pause(t_rscp_media* media, void* UNUSED m, t_rscp_connection* rsp)
{
    vtb_video_pause(media);
    rscp_response_pause(rsp, media->sessionid);

    return RSCP_SUCCESS;
}

int vtb_video_update(t_rscp_media *media, t_rscp_req_update *m, t_rscp_connection *rsp)
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
    uint32_t timeout;

    switch (event) {
        case EVENT_VIDEO_IN_ON:
        	report(INFO "EVENT VIDEO IN ON");
            if (rscp_media_splaying(media)) {
                vtb_video_pause(media);
                rscp_server_update(media, EVENT_VIDEO_IN_OFF);
            }
            rscp_server_update(media, EVENT_VIDEO_IN_ON);
            return RSCP_PAUSE;
        break;
        case EVENT_VIDEO_IN_OFF:
        	report(INFO "EVENT VIDEO IN OFF");
            if (rscp_media_splaying(media)) {
                rscp_server_update(media, EVENT_VIDEO_IN_OFF);
                rscp_listener_add_kill(&hdoipd.listener, media);
                hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
                osd_permanent(true);
                osd_printf("vtb.video no input...\n");

            }
        break;
        case EVENT_TICK:
            if (cookie->alive_ping) {
                cookie->alive_ping--;
            } else {
                cookie->alive_ping = TICK_SEND_ALIVE;
                // send tick we are alive (until something like rtcp is used)
          //      if (hdoipd_tstate(VTB_VIDEO)) { // only if video stream = active
                rscp_server_update(media, EVENT_TICK);
          //      }
            }

            if (get_multicast_enable()) {
                timeout = TICK_TIMEOUT_MULTICAST;
            } else {
                timeout = TICK_TIMEOUT_UNICAST;
            }

            if (cookie->timeout <= timeout) {
                cookie->timeout++;

            } else {
                report(INFO "vtb_video_event: timeout");
                // timeout -> kill connection
                // server cannot kill itself -> add to kill list
                // (will be executed after all events are processed)
                cookie->timeout = 0;
                rscp_listener_add_kill(&hdoipd.listener, media);
                hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
                osd_permanent(true);
                osd_printf("vtb.video connection lost...\n");
            }
        break;
    }

    return RSCP_SUCCESS;
}

t_rscp_media vtb_video = {
    .name = "video",
    .owner = 0,
    .cookie = 0,
    .hdcp = (frscpm*)vtb_video_hdcp,
    .cookie_size = sizeof(t_multicast_cookie),
    .setup = (frscpm*)vtb_video_setup,
    .play = (frscpm*)vtb_video_play,
    .pause = (frscpm*)vtb_video_ext_pause,
    .teardown = (frscpm*)vtb_video_teardown,
    .update = (frscpm*)vtb_video_update,
    .event = (frscpe*)vtb_video_event
};
