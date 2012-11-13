/*
 * vtb_audio.c
 *
 *  Created on: 06.01.2011
 *      Author: alda
 */

#include "box_sys.h"
#include "hdcp.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hoi_drv_user.h"
#include "multicast.h"
#include "rtsp_command.h"
#include "rtsp_server.h"
#include "vtb_audio.h"

#define TICK_TIMEOUT_MULTICAST          (hdoipd.eth_timeout * 2)
#define TICK_TIMEOUT_UNICAST            (hdoipd.eth_timeout)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

int vtb_audio_hdcp(t_rtsp_media* media, t_rtsp_req_hdcp* m, t_rtsp_connection* rsp)
{
	int ret;

	t_multicast_cookie* cookie = media->cookie;
    ret = hdcp_ske_s(media, m, rsp);

    cookie->timeout = 0;
    cookie->alive_ping = 1;

    return ret;
}

int vtb_audio_setup(t_rtsp_media* media, t_rtsp_req_setup* m, t_rtsp_connection* rsp)
{
    int hdcp;
    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_audio_setup");

    set_multicast_enable(reg_test("multicast_en", "true"));

    if (!hdoipd_state(HOID_VTB)) {
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

    // reserve resource
    if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_NOT_AVAILABLE)) {
        hdoipd_set_vtb_state(VTB_AUD_IDLE);
    }

    //check if hdcp is forced by HDMI, user or client (over RTSP)
    if (reg_test("hdcp-force", "true") || hdoipd_rsc(RSC_VIDEO_IN_HDCP) || (m->hdcp.hdcp_on==1)) {
    	m->hdcp.hdcp_on = 1;
    	hdoipd.hdcp.enc_state = HDCP_ENABLED;
    }

    cookie->remote.address = rsp->address;
    cookie->remote.aud_port = PORT_RANGE_START(m->transport.client_port);
    m->transport.server_port = PORT_RANGE(hdoipd.local.aud_port, hdoipd.local.aud_port);
    m->transport.multicast = get_multicast_enable();

    rtsp_response_setup(rsp, &m->transport, media->sessionid, &m->hdcp);

    REPORT_RTX("TX", hdoipd.local, "->", cookie->remote, aud);

    media->result = RTSP_RESULT_READY;

    cookie->timeout = 0;
    cookie->alive_ping = 1;

    return RTSP_SUCCESS;
}

int vtb_audio_play(t_rtsp_media* media, t_rtsp_req_play UNUSED *m, t_rtsp_connection* rsp)
{
    int n, hdcp;
    t_rtsp_rtp_format fmt;
    hdoip_eth_params eth;
    char dst_mac[6];

    t_multicast_cookie* cookie = media->cookie;

    media->result = RTSP_RESULT_PLAYING;

    report(VTB_METHOD "vtb_audio_play");

    if (!hdoipd_state(HOID_VTB)) {
        // we don't have the resource reserved
        report(" ? require state VTB");
        rtsp_err_server(rsp);
        return RTSP_REQUEST_ERROR;
    }

    if (!hdoipd_rsc(RSC_AUDIO_IN)) {
        // currently no audio input active
        report(" ? require active audio input");
        rtsp_err_no_source(rsp);
        //hdoipd_set_vtb_state(VTB_AUD_OFF);
        hdoipd_set_vtb_state(VTB_AUD_IDLE);
        return RTSP_REQUEST_ERROR;
    }

    //check if audio HDCP status is unchanged, else return ERROR
    report("TEST HDCP status before start audio play!");
    if (hdoipd.hdcp.enc_state && !(get_hdcp_status() & HDCP_ETO_AUDIO_EN)){
    	if (hdoipd.hdcp.ske_executed){
    		hoi_drv_hdcp(hdoipd.hdcp.keys); //write keys to kernel
    		hoi_drv_hdcp_auden_eto();
    		report(INFO "Audio encryption enabled (eto)!");
    	}
    	else {
    		report(" ? Audio HDCP status has changed after SKE");
    		rtsp_err_hdcp(rsp);
    		return RTSP_REQUEST_ERROR;
    	}
    }

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
    eth.packet_size = 512;
    eth.rtp_ssrc = 0;
    for(n=0;n<6;n++) eth.src_mac[n] = hdoipd.local.mac[n];
    for(n=0;n<6;n++) eth.dst_mac[n] = dst_mac[n];
    eth.udp_dst_port = cookie->remote.aud_port;
    eth.udp_src_port = hdoipd.local.aud_port;

    // asi parameters (TODO: restrict audio channel based on registry)
    t_hoi_msg_info *nfo;
    hoi_drv_info_all(&nfo);
    fmt.compress = nfo->audio_width[0];
    fmt.value = nfo->audio_fs[0];
    fmt.value2 = nfo->audio_cnt[0];
    hoi_drv_get_mtime(&fmt.rtptime);

    if ( ((nfo->audio_width[0]<8) || (nfo->audio_width[0]>32)) ||
         ((nfo->audio_fs[0]<32000) || (nfo->audio_fs[0]>192000)) ||
         ((nfo->audio_cnt[0]<1) || (nfo->audio_cnt[0]>8)) ) {
          rtsp_err_def_source(rsp);
          return RTSP_REQUEST_ERROR;
    }

    // send timing
    rtsp_response_play(rsp, media->sessionid, &fmt, 0);

    if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_NOT_AVAILABLE)) {
        // activate asi
#ifdef AUD_IN_PATH
        uint8_t channel_select[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        hoi_drv_asi(0, &eth, nfo->audio_fs[0], nfo->audio_width[0], nfo->audio_cnt[0], channel_select);
        report(INFO "\naudio streaming started(fs = %d Hz, bitwidth = %d Bit)", nfo->audio_fs[0], nfo->audio_width[0]);
#endif
        // We are streaming Audio now...
        hdoipd_set_vtb_state(VTB_AUDIO);
    }

    add_client_to_vtb(MEDIA_IS_AUDIO, cookie->remote.address);

    return RTSP_SUCCESS;
}

int vtb_audio_teardown(t_rtsp_media* media, t_rtsp_req_teardown UNUSED *m, t_rtsp_connection* rsp)
{
    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_audio_teardown");

    media->result = RTSP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_AUD_MASK)) {
        if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_AVAILABLE_ONLY_ONE)) {
#ifdef AUD_IN_PATH
            hdoipd_hw_reset(DRV_RST_AUD_IN);
#endif
            hdoipd_set_vtb_state(VTB_AUD_OFF);
        }
    }

    if (rsp) {
        rtsp_response_teardown(rsp, media->sessionid);
    }

    cookie->timeout = 0;
    cookie->alive_ping = 1;

    remove_client_from_vtb(MEDIA_IS_AUDIO, cookie->remote.address);

    return RTSP_SUCCESS;
}

void vtb_audio_pause(t_rtsp_media *media)
{
    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_audio_pause");

    media->result = RTSP_RESULT_PAUSE_Q;

    if (hdoipd_tstate(VTB_AUDIO)) {
        if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_AVAILABLE_ONLY_ONE)) {
#ifdef AUD_IN_PATH
            hdoipd_hw_reset(DRV_RST_AUD_IN);
#endif
            hdoipd_set_vtb_state(VTB_AUD_IDLE);
        }
    }

    remove_client_from_vtb(MEDIA_IS_AUDIO, cookie->remote.address);
}

int vtb_audio_ext_pause(t_rtsp_media* media, void* UNUSED m, t_rtsp_connection* rsp)
{
    vtb_audio_pause(media);
    rtsp_response_pause(rsp, media->sessionid);

    return RTSP_SUCCESS;
}

int vtb_audio_update(t_rtsp_media UNUSED *media, t_rtsp_req_update *m, t_rtsp_connection UNUSED *rsp)
{
    t_multicast_cookie* cookie = media->cookie;

    switch (m->event) {
        case EVENT_TICK:
            // reset timeout
            cookie->timeout = 0;
        break;
    }

    return RTSP_SUCCESS;
}

int vtb_audio_event(t_rtsp_media *media, uint32_t event)
{
    t_multicast_cookie* cookie = media->cookie;
    uint32_t timeout;

    switch (event) {
        case EVENT_AUDIO_IN0_ON:
            if (rtsp_media_splaying(media)) {
                vtb_audio_pause(media);
                rtsp_server_update(media, EVENT_AUDIO_IN0_OFF);
            }
            rtsp_server_update(media, EVENT_AUDIO_IN0_ON);
            return RTSP_PAUSE;
        break;
        case EVENT_AUDIO_IN0_OFF:
            if (rtsp_media_splaying(media)) {
                rtsp_server_update(media, EVENT_AUDIO_IN0_OFF);
                rtsp_listener_add_kill(&hdoipd.listener, media);
            }
        break;
        case EVENT_TICK:
            if (cookie->alive_ping) {
                cookie->alive_ping--;
            } else {
                cookie->alive_ping = TICK_SEND_ALIVE;
                // send tick we are alive (until something like rtcp is used)
       //         if (hdoipd_tstate(VTB_AUDIO)) { // only if audio stream = active
                rtsp_server_update(media, EVENT_TICK);
       //         }
            }

            if (get_multicast_enable()) {
                timeout = TICK_TIMEOUT_MULTICAST;
            } else {
                timeout = TICK_TIMEOUT_UNICAST;
            }

            if (cookie->timeout <= timeout) {
                cookie->timeout++;
            } else {
                report(INFO "vtb_audio_event: timeout");
                // timeout -> kill connection
                // server cannot kill itself -> add to kill list
                // (will be executed after all events are processed)
                cookie->timeout = 0;
                rtsp_listener_add_kill(&hdoipd.listener, media);
            }
        break;
    }

    return RTSP_SUCCESS;
}


t_rtsp_media vtb_audio = {
    .name = "audio",
    .owner = 0,
    .cookie = 0,
    .hdcp = (frtspm*)vtb_audio_hdcp,
    .cookie_size = sizeof(t_multicast_cookie),
    .setup = (frtspm*)vtb_audio_setup,
    .play = (frtspm*)vtb_audio_play,
    .pause = (frtspm*)vtb_audio_ext_pause,
    .teardown = (frtspm*)vtb_audio_teardown,
    .update = (frtspm*)vtb_audio_update,
    .event = (frtspe*)vtb_audio_event
};
