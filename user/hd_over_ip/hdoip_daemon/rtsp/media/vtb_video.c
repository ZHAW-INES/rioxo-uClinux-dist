/*
 * vtb_video.c
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include "box_sys.h"
#include "box_sys_vtb.h"
#include "edid_merge.h"
#include "hdcp.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hdoipd_osd.h"
#include "hoi_drv_user.h"
#include "multicast.h"
#include "rtsp_command.h"
#include "rtsp_server.h"
#include "vtb_video.h"

#define TICK_TIMEOUT_MULTICAST          (hdoipd.eth_timeout * 2)
#define TICK_TIMEOUT_UNICAST            (hdoipd.eth_timeout)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

int vtb_video_describe(t_rtsp_media *media, void *_data, t_rtsp_connection *con)
{
    t_rtsp_req_describe *data = _data;
    int ret;
    const char *s, *compress;

    if (!data)
        return -1;

    ret = rtsp_handle_describe_generic(media, data, con);
    if (ret)
        return ret;

    /* Add media specific content */

    s = reg_get("compress");
    if (strcmp(s, "jp2k") == 0)
        compress = "JPEG2000";
    else
        compress = "Plain";

    msgprintf(con, "m=video 0 RTP/AVP 96\r\n");
    msgprintf(con, "a=control:/device/video RTSP/1.0/\r\n");
    msgprintf(con, "a=rtpmap:96 %s/1000000\r\n", compress);
    msgprintf(con, "a=fmtp:96 img_size=1920*1080; adv_div=4; FPS=60\r\n");

    rtsp_send(con);

    return 0;
}

int vtb_video_hdcp(t_rtsp_media* media, t_rtsp_req_hdcp* m, t_rtsp_connection* rsp)
{
	int ret;

    ret = hdcp_ske_s(media, m, rsp);

    ((t_rtsp_server*)media->creator)->timeout.timeout = 0;

    return ret;
}

int vtb_video_setup(t_rtsp_media* media, t_rtsp_req_setup* m, t_rtsp_connection* rsp)
{
    t_multicast_cookie* cookie = media->cookie;
    int temp;

    report(VTB_METHOD "vtb_video_setup");

    media->result = RTSP_RESULT_READY;

    multicast_set_enabled(reg_test("multicast_en", "true"));

    if (!hdoipd_state(HOID_VTB)) {
        // currently no video input active
        report(" ? not in state vtb");
        rtsp_err_no_vtb(rsp);
        return RTSP_REQUEST_ERROR;
    }

    // get own MAC address
    if (net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac) != RTSP_SUCCESS) {
        report(" ? net_get_local_hwaddr failed");
        rtsp_err_server(rsp);
        return RTSP_REQUEST_ERROR;
    }

    // get own IP address
    if (net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address) != RTSP_SUCCESS) {
        report(" ? net_get_local_addr failed");
        rtsp_err_server(rsp);
        return RTSP_REQUEST_ERROR;
    }

    // get MAC address of next in rout
    if (net_get_remote_hwaddr(hdoipd.listener.sockfd, "eth0", rsp->address, (uint8_t*)&cookie->remote.mac) != RTSP_SUCCESS) {
        report(" ? net_get_remote_hwaddr failed");
        rtsp_err_retry(rsp);
        return RTSP_REQUEST_ERROR;
    }

    if (!multicast_get_enabled() && hdoipd_tstate(VTB_VIDEO)) {
        report(ERROR "already streaming video");
        rtsp_response_error(rsp, RTSP_STATUS_SERVICE_UNAVAILABLE, "Already Streaming");
        return RTSP_REQUEST_ERROR;
    }

    // only set the EDID if it has been passed as a header value
    // also takes care of multicast setup    

    temp = rtsp_server_handle_setup((t_rtsp_server*)media->creator, &(m->edid), media);
    if (temp != 0) {
        report(ERROR "setting up video input failed : %d", temp);
        rtsp_err_no_source(rsp);
        return RTSP_REQUEST_ERROR;
    }


    if ((!multicast_get_enabled()) || (multicast_client_check_availability(MEDIA_IS_VIDEO) == CLIENT_NOT_AVAILABLE)) {
        // reserve resource
        hdoipd_set_vtb_state(VTB_VID_IDLE);
    }

    ((t_rtsp_server*)media->creator)->timeout.timeout = 0;

    //check if hdcp is forced by HDMI, user or client (over RTSP)
    if (reg_test("hdcp-force", "true") || hdoipd_rsc(RSC_VIDEO_IN_HDCP) || (m->hdcp.hdcp_on==1)) {
    	m->hdcp.hdcp_on = 1;
    	hdoipd.hdcp.enc_state = HDCP_ENABLED;
    }

    cookie->remote.address = rsp->address;
    // parse "client_port" or "port" in the "Transport" header
    // if neither is present, fall back to the local port
    cookie->remote.vid_port = PORT_RANGE_START(m->transport.client_port);
    if (cookie->remote.vid_port <= 0) {
        cookie->remote.vid_port = PORT_RANGE_START(m->transport.port);
        if (cookie->remote.vid_port <= 0) {
            cookie->remote.vid_port = PORT_RANGE_START(hdoipd.local.vid_port);
        }
    }
    m->transport.server_port = PORT_RANGE(hdoipd.local.vid_port, hdoipd.local.vid_port);
    m->transport.multicast = multicast_get_enabled();

    rtsp_response_setup(rsp, &m->transport, media->sessionid, media->name, &m->hdcp); //respond message to setup message

    REPORT_RTX("TX", hdoipd.local, "->", cookie->remote, vid);

    return RTSP_SUCCESS;
}

int vtb_video_play(t_rtsp_media* media, t_rtsp_req_play* m, t_rtsp_connection* rsp)
{
    int n;
    t_video_timing timing;
    t_rtsp_rtp_format fmt;
    struct hdoip_eth_params eth;
    char dst_mac[6];
    uint32_t bandwidth;
    uint32_t chroma;
    uint32_t traffic_shaping;
    t_fec_setting fec;
    char *fec_setting;

    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_video_play");

    if (!hdoipd_state(HOID_VTB)) {
        // we don't have the resource reserved
        report(" ? require state VTB");
        rtsp_err_server(rsp);
        return RTSP_REQUEST_ERROR;
    }

    if (!hdoipd_rsc(RSC_VIDEO_IN)) {
        // currently no video input active
        report(" ? require active video input");
        media->result = RTSP_RESULT_NO_VIDEO_IN;
        rtsp_err_no_source(rsp);
        //hdoipd_set_vtb_state(VTB_VID_OFF);
        hdoipd_set_vtb_state(VTB_VID_IDLE);
        return RTSP_REQUEST_ERROR;
    }

    if (!multicast_get_enabled() && hdoipd_tstate(VTB_VIDEO)) {
        report(ERROR "already streaming video");
        rtsp_response_error(rsp, RTSP_STATUS_SERVICE_UNAVAILABLE, "Already Streaming");
        return RTSP_REQUEST_ERROR;
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
    	    rtsp_err_hdcp(rsp);
    	    return RTSP_REQUEST_ERROR;
    	}
    }

    // test for valid resolution

    // start ethernet output
    if(multicast_get_enabled()) {
        eth.ipv4_dst_ip = inet_addr(reg_get("multicast_group"));
        convert_ip_to_multicast_mac(inet_addr(reg_get("multicast_group")), dst_mac);
        report(INFO "sending multicast to : %s", reg_get("multicast_group"));
    }
    else {
        eth.ipv4_dst_ip = cookie->remote.address;
        for(n=0;n<6;n++) dst_mac[n] = cookie->remote.mac[n];
        report(INFO "sending unicast to : %i.%i.%i.%i", ((cookie->remote.address) & 0xff), ((cookie->remote.address >> 8) & 0xff), ((cookie->remote.address >> 16) & 0xff), ((cookie->remote.address >> 24) & 0xff));

    }

    eth.ipv4_src_ip = hdoipd.local.address;
    eth.ipv4_tos = 0;
    eth.ipv4_ttl = 30;
    eth.packet_size = 1500-56;
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
        rtsp_err_server(rsp);
        return RTSP_REQUEST_ERROR;
    }

    hoi_drv_get_mtime(&fmt.rtptime);

    bandwidth = (uint32_t)reg_get_int("bandwidth");             // bandwidth in byte/s 
    chroma    = (uint32_t)reg_get_int("chroma-bandwidth");      // percent of chroma bandwidth (0 .. 100)

    traffic_shaping = reg_test("traffic-shaping", "true");      // enables ethernet traffic shaping of video packets

    // use 4 ADV212 if 1080i and bandwidth >= 50Mbit/s
    if ((timing.width == 1920) && (timing.height == 540) && (bandwidth >= (uint32_t)((uint64_t)(25+25*chroma/100)*(1048576)/8))) {
        fmt.value = 4;
    }

    // limit bandwidth to 120Mbit/s for 1080i
    if ((timing.width == 1920) && (timing.height == 540) && (bandwidth > (uint32_t)((uint64_t)(60+60*chroma/100)*(1048576)/8))) {
        bandwidth = (uint32_t)((uint64_t)(60+60*chroma/100)*(1048576)/8);
    }

    // limit bandwidth to 50Mbit/s for 576i and 480i
    if ((timing.width == 720) && ((timing.height == 240) || (timing.height == 243) || (timing.height == 244) || (timing.height == 288)) && (bandwidth > (uint32_t)((uint64_t)(25+25*chroma/100)*(1048576)/8))) {
        bandwidth = (uint32_t)((uint64_t)(25+25*chroma/100)*(1048576)/8);
    }

    // fec settings (convert from ascii to integer)
    fec_setting = reg_get("fec_setting");
    fec.video_enable = fec_setting[0] - 48;
    fec.video_l = fec_setting[1] - 48 + 4;
    fec.video_d = fec_setting[2] - 48 + 4;
    fec.video_interleaving = fec_setting[3] - 48;
    fec.video_col_only = fec_setting[4] - 48;
    fec.audio_enable =fec_setting[5] - 48;
    fec.audio_l = fec_setting[6] - 48 + 4;
    fec.audio_d = fec_setting[7] - 48 + 4;
    fec.audio_interleaving = fec_setting[8] - 48;
    fec.audio_col_only = fec_setting[9] - 48;

    // send timing
    rtsp_response_play(rsp, media->sessionid, &fmt, &timing);

    if ((!multicast_get_enabled()) || (multicast_client_check_availability(MEDIA_IS_VIDEO) == CLIENT_NOT_AVAILABLE)) {
        // activate vsi
#ifdef VID_IN_PATH
        if (hoi_drv_vsi(fmt.compress, chroma, 0, bandwidth, &eth, &timing, &fmt.value, traffic_shaping, &fec)) {
            return RTSP_REQUEST_ERROR;
        }
#endif
        // We are streaming Video now...
        hdoipd_set_vtb_state(VTB_VIDEO);
    }

    media->result = RTSP_RESULT_PLAYING;
    multicast_client_add(MEDIA_IS_VIDEO, (t_rtsp_server*)media->creator);

    osd_permanent(false);

    return RTSP_SUCCESS;
}

int vtb_video_teardown(t_rtsp_media* media, t_rtsp_req_teardown UNUSED *m, t_rtsp_connection* rsp)
{
    t_rtsp_server* server = (t_rtsp_server*)media->creator;

    report(VTB_METHOD "vtb_video_teardown");

    media->result = RTSP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_VIDEO|VTB_VID_IDLE)) {
        if ((!multicast_get_enabled()) || (multicast_client_check_availability(MEDIA_IS_VIDEO) == CLIENT_AVAILABLE_ONLY_ONE)) {
#ifdef VID_IN_PATH
            hdoipd_hw_reset(DRV_RST_VID_IN);
#endif
            hdoipd_set_vtb_state(VTB_VID_OFF);

            hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
            osd_permanent(true);
            osd_printf("vtb.video connection stopped...\n");
        }
    }

    if (rsp) {
        rtsp_response_teardown(rsp);
    }

    server->timeout.timeout = 0;   
    multicast_edid_remove(server);
    multicast_client_remove(MEDIA_IS_VIDEO, server);

    return RTSP_SUCCESS;
}

void vtb_video_pause(t_rtsp_media *media)
{
    report(VTB_METHOD "vtb_video_pause");

    media->result = RTSP_RESULT_PAUSE_Q;

    if (hdoipd_tstate(VTB_VIDEO)) {
        if ((!multicast_get_enabled()) || (multicast_client_check_availability(MEDIA_IS_VIDEO) == CLIENT_AVAILABLE_ONLY_ONE)) {
#ifdef VID_IN_PATH
            hdoipd_hw_reset(DRV_RST_VID_IN);
#endif
            hdoipd_set_vtb_state(VTB_VID_IDLE);

            hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
            osd_permanent(true);
            osd_printf("Video input lost...");

        }
    }

    multicast_client_remove(MEDIA_IS_VIDEO, (t_rtsp_server*)media->creator);
}

int vtb_video_ext_pause(t_rtsp_media *media, void *m UNUSED, t_rtsp_connection *rsp)
{
    vtb_video_pause(media);
    rtsp_response_pause(rsp, media->sessionid);

    return RTSP_SUCCESS;
}

int vtb_video_event(t_rtsp_media *media, uint32_t event)
{
    if (rtsp_media_sinit(media))
        return RTSP_WRONG_STATE;

    switch (event) {
        case EVENT_VIDEO_IN_ON:
        	report(INFO "EVENT VIDEO IN ON");
            if (rtsp_media_splaying(media)) {
                vtb_video_pause(media);
     //           rtsp_server_update_media(media, EVENT_VIDEO_IN_OFF);
            }
            rtsp_server_update_media(media, EVENT_VIDEO_IN_ON);
            return RTSP_PAUSE;
            break;
        case EVENT_VIDEO_IN_OFF:
        	report(INFO "EVENT VIDEO IN OFF");
            if (rtsp_media_splaying(media)) {
                rtsp_server_update_media(media, EVENT_VIDEO_IN_OFF);
                hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
                osd_permanent(true);
                osd_printf("vtb.video no input...\n");

            }
            break;
    }

    return RTSP_SUCCESS;
}

t_rtsp_media vtb_video = {
    .name = "video",
    .owner = 0,
    .cookie = 0,
    .options = (frtspm*)box_sys_options,
    .describe = vtb_video_describe,
    .get_parameter = (frtspm*)box_sys_vtb_get_parameter,
    .hdcp = (frtspm*)vtb_video_hdcp,
    .cookie_size = sizeof(t_multicast_cookie),
    .setup = (frtspm*)vtb_video_setup,
    .play = (frtspm*)vtb_video_play,
    .pause = (frtspm*)vtb_video_ext_pause,
    .teardown = (frtspm*)vtb_video_teardown,
    .event = (frtspe*)vtb_video_event
};
