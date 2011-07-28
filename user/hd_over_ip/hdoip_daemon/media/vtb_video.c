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
#include "edid_merge.h"
#include "rscp_server.h"
#include "hdcp.h"
#include "multicast.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


#define EDID_PATH_VIDEO_IN              "/tmp/edid_vid_in"
#define EDID_PREAMBLE                   "/tmp/edid_"

#define TICK_TIMEOUT                    (hdoipd.eth_timeout)
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
    int n, ret;

    t_multicast_cookie* cookie = media->cookie;
    t_edid edid1, edid2;
    bool merge = false;
    char buf[255];

    report(VTB_METHOD "vtb_video_setup");

    set_multicast_enable(reg_test("multicast_en", "true"));

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

    // E-EDID handling
    sprintf(buf, "%s%08x",EDID_PREAMBLE, rsp->address);
    ret = edid_read_file(&edid1, buf);
    if(ret != -1) {
        merge = false;
        if(ret == -2) { // No E-EDID exist from host
            edid_write_file((t_edid *)m->edid.edid, buf);
            report(INFO "no E-EDID stored, save remote E-EDID");
            merge = true;
        } else {
            edid_read_file(&edid1, buf);
            if(memcmp(&edid1, (void*)m->edid.edid, 256 * sizeof(uint8_t)) != 0) {
                edid_write_file((t_edid *)m->edid.edid, buf);
                report(INFO "remote E-EDID changed, save remote E-EDID");
                merge = true;
            } else {
                report(INFO "remote E-EDID didn't changed");
            }
        }

        if(merge) { // new or changed E-EDID from remote received
            ret = edid_read_file(&edid1, EDID_PATH_VIDEO_IN);
            if(ret != -1) {
                if((ret == -2) || !get_multicast_enable()) { // No E-EDID for video exists or unicast -> write edid without merge
                    report(INFO "E-EDID loaded");
                    edid_write_file((t_edid *)m->edid.edid, EDID_PATH_VIDEO_IN);
                    hoi_drv_wredid((t_edid *)m->edid.edid);
                    hdoipd_clr_rsc(RSC_VIDEO_IN);
                    hdoipd_clr_rsc(RSC_AUDIO0_IN);
                } else { // Merge E-EDIDs
                    memcpy(&edid2, &edid1, 256 * sizeof(uint8_t));
                    edid_merge(&edid1, (t_edid *)m->edid.edid);
                    if(memcmp(&edid1, &edid2, 256 * sizeof(uint8_t)) != 0) { // if video in E-EDID changed
                        report(INFO "loaded E-EDID after merging");
                        edid_write_file(&edid1, EDID_PATH_VIDEO_IN);
                        hoi_drv_wredid(&edid1);
                        hdoipd_clr_rsc(RSC_VIDEO_IN);
                        hdoipd_clr_rsc(RSC_AUDIO0_IN);
                    } else {
                        report(INFO "Output E-EDID didn't changed");
                    }
                }
            } else {
                perrno("vtb_video_setup() failed");
            }
        }
    } else {
        perrno("vtb_video_setup() failed");
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
    int n;
    t_video_timing timing;
    t_rscp_rtp_format fmt;
    hdoip_eth_params eth;
    char dst_mac[6];

    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_video_play");

    media->result = RSCP_RESULT_PLAYING;

    if (!hdoipd_state(HOID_VTB)) {
        // we don't have the resource reserved
        report(" ? require state VTB");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

   /* if((!get_multicast_enable()) || (check_client_availability == CLIENT_NOT_AVAILABLE)) {
        if (!hdoipd_tstate(VTB_VID_IDLE)) {
            // we don't have the resource reserved
            report(" ? require state VTB_IDLE");
            rscp_err_server(rsp);
            return RSCP_REQUEST_ERROR;
        }
    }*/

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
    if ((n = hoi_drv_info(&timing, &fmt.value))) {
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    hoi_drv_get_mtime(&fmt.rtptime);

    // send timing
    rscp_response_play(rsp, media->sessionid, &fmt, &timing);

    if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_VIDEO) == CLIENT_NOT_AVAILABLE)) {
        // activate vsi
#ifdef VID_IN_PATH
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

    remove_client_from_vtb(MEDIA_IS_VIDEO, cookie->remote.address);

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
        }
    }

    remove_client_from_vtb(MEDIA_IS_VIDEO, cookie->remote.address);
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
    .hdcp = (frscpm*)vtb_video_hdcp,
    .cookie_size = sizeof(t_multicast_cookie),
    .setup = (frscpm*)vtb_video_setup,
    .play = (frscpm*)vtb_video_play,
    .teardown = (frscpm*)vtb_video_teardown,
    .update = (frscpm*)vtb_video_update,
    .event = (frscpe*)vtb_video_event
};
