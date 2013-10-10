/*
 * vtb_audio_emb.c
 *
 *  Created on: 06.01.2011
 *      Author: alda
 */

#include "box_sys.h"
#include "box_sys_vtb.h"
#include "hdcp.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hoi_drv_user.h"
#include "multicast.h"
#include "rtsp_command.h"
#include "rtsp_server.h"
#include "vtb_audio_emb.h"

#define TICK_TIMEOUT_MULTICAST          (hdoipd.eth_timeout * 2)
#define TICK_TIMEOUT_UNICAST            (hdoipd.eth_timeout)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

int vtb_audio_emb_describe(t_rtsp_media *media, void *_data, t_rtsp_connection *con)
{
    t_rtsp_req_describe *data = _data;
    int ret;

    if (!data)
        return -1;

    ret = rtsp_handle_describe_generic(media, data, con);
    if (ret)
        return ret;

    /* Add media specific content */
/*
    msgprintf(con, "m=audio 0 RTP/AVP 97\r\n");
    msgprintf(con, "a=control:/device/1/audio/1/\r\n");
    msgprintf(con, "a=fmtp:97 line_out/head_out; line_in/mic; bit=16; fs=48000\r\n");
    msgprintf(con, "m=audio 0 RTP/AVP 97\r\n");
    msgprintf(con, "a=control:/device/1/audio/2/\r\n");
    msgprintf(con, "a=fmtp:97 video-audio; fs=48000\r\n");             //TODO*/

    rtsp_send(con);

    return 0;
}

int vtb_audio_emb_hdcp(t_rtsp_media* media, t_rtsp_req_hdcp* m, t_rtsp_connection* rsp)
{
	int ret;

    ret = hdcp_ske_s(media, m, rsp);
    ((t_rtsp_server*)media->creator)->timeout.timeout = 0;
    return ret;
}

int vtb_audio_emb_setup(t_rtsp_media* media, t_rtsp_req_setup* m, t_rtsp_connection* rsp)
{
    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_audio_emb_setup");

    multicast_set_enabled(reg_test("multicast_en", "true"));

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

    if (!multicast_get_enabled() && hdoipd_tstate(VTB_AUDIO_EMB) ) {
        report(ERROR "already streaming audio embedded");
        rtsp_response_error(rsp, RTSP_STATUS_INTERNAL_SERVER_ERROR, "Already Streaming");
        return RTSP_REQUEST_ERROR;
    }

    // only set the EDID if it has been passed as a header value
    if (rtsp_server_handle_setup((t_rtsp_server*)media->creator, &(m->edid), media) !=0) {
        report(ERROR "setting up audio input failed");
        rtsp_err_no_source(rsp);
//        unlock("audio_setup");
        return RTSP_REQUEST_ERROR;
    }

    // reserve resource
    if ((!multicast_get_enabled()) || (multicast_client_check_availability(MEDIA_IS_AUDIO) == CLIENT_NOT_AVAILABLE)) {
        hdoipd_set_vtb_state(VTB_AUD_EMB_IDLE);
    }

    //check if hdcp is forced by HDMI, user or client (over RTSP)
    if (reg_test("hdcp-force", "true") || hdoipd_rsc(RSC_VIDEO_IN_HDCP) || (m->hdcp.hdcp_on==1)) {
    	m->hdcp.hdcp_on = 1;
    	hdoipd.hdcp.enc_state = HDCP_ENABLED;
    }

    cookie->remote.address = rsp->address;
    cookie->remote.aud_port = PORT_RANGE_START(m->transport.client_port);
    m->transport.server_port = PORT_RANGE(hdoipd.local.aud_port, hdoipd.local.aud_port);
    m->transport.multicast = multicast_get_enabled();

    rtsp_response_setup(rsp, &m->transport, media->sessionid, media->name, &m->hdcp);

    REPORT_RTX("TX", hdoipd.local, "->", cookie->remote, aud);

    media->result = RTSP_RESULT_READY;
    ((t_rtsp_server*)media->creator)->timeout.timeout = 0;

    return RTSP_SUCCESS;
}

int vtb_audio_emb_play(t_rtsp_media* media, t_rtsp_req_play UNUSED *m, t_rtsp_connection* rsp)
{
    int n;
    t_rtsp_rtp_format fmt;
    struct hdoip_eth_params eth;
    struct hdoip_aud_params aud;
    char dst_mac[6];
    t_fec_setting fec;
    char *fec_setting;
    uint16_t audio_channel_status[12];

    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_audio_emb_play");

    if (!hdoipd_state(HOID_VTB)) {
        // we don't have the resource reserved
        report(" ? require state VTB");
        rtsp_err_server(rsp);
        return RTSP_REQUEST_ERROR;
    }

    if (!hdoipd_rsc(RSC_AUDIO_EMB_IN)) {
        // currently no audio input active
        report(" ? require active audio input");
        rtsp_err_no_source(rsp);
        hdoipd_set_vtb_state(VTB_AUD_EMB_IDLE);
        return RTSP_REQUEST_ERROR;
    }

    if (!multicast_get_enabled() && hdoipd_tstate(VTB_AUDIO_EMB) ) {
        report(ERROR "already streaming audio embedded");
        rtsp_response_error(rsp, RTSP_STATUS_INTERNAL_SERVER_ERROR, "Already Streaming");
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

    // network settings
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
    eth.ipv4_ttl = reg_get_int("eth-ttl");
    eth.packet_size = 512;
    eth.rtp_ssrc = 0;
    for(n=0;n<6;n++) eth.src_mac[n] = hdoipd.local.mac[n];
    for(n=0;n<6;n++) eth.dst_mac[n] = dst_mac[n];
    eth.udp_dst_port = cookie->remote.aud_port;
    eth.udp_src_port = hdoipd.local.aud_port;

    // asi parameters (TODO: restrict audio channel based on registry)
    // read parameters from gateware
    t_hoi_msg_info *nfo;
    hoi_drv_info_all(&nfo);
    memcpy(&aud, &(nfo->aud_params[AUD_STREAM_NR_EMBEDDED]), sizeof(struct hdoip_aud_params));

    fmt.compress = aud.sample_width;
    fmt.value = aud.fs;
    fmt.value2 = (uint32_t) aud.ch_map;
    hoi_drv_get_mtime(&fmt.rtptime);

    if ( ((aud.sample_width < 8) || (aud.sample_width > 32)) ||
         ((aud.fs < 32000) || (aud.fs > 192000)) ||
         ((aud.ch_map < 0x0001) || (aud.ch_map > 0x00FF)) ) {
          rtsp_err_def_source(rsp);
          return RTSP_REQUEST_ERROR;
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

    // get audio channel status
    hoi_drv_acs(audio_channel_status);

    // send timing
    rtsp_response_play(rsp, media->sessionid, &fmt, 0, reg_test("traffic-shaping", "true"), audio_channel_status);

    if ((!multicast_get_enabled()) || (multicast_client_check_availability(MEDIA_IS_AUDIO) == CLIENT_NOT_AVAILABLE)) {
        // activate asi
#ifdef AUD_IN_PATH
        hoi_drv_asi(AUD_STREAM_NR_EMBEDDED, &eth, &aud, &fec);
        report(INFO "\naudio embedded streaming started(fs = %d Hz, bitwidth = %d Bit, ch_map = 0x%02X)", aud.fs, aud.sample_width, aud.ch_map);
#endif

        // We are streaming Audio now...
        hdoipd_set_vtb_state(VTB_AUDIO_EMB);
    }

    media->result = RTSP_RESULT_PLAYING;
    multicast_client_add(MEDIA_IS_AUDIO, (t_rtsp_server*)media->creator);

    return RTSP_SUCCESS;
}

int vtb_audio_emb_teardown(t_rtsp_media* media, t_rtsp_req_teardown UNUSED *m, t_rtsp_connection* rsp)
{
    t_rtsp_server* server = (t_rtsp_server*)media->creator;    

    report(VTB_METHOD "vtb_audio_emb_teardown");

    media->result = RTSP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_AUD_EMB_MASK)) {
        if ((!multicast_get_enabled()) || (multicast_client_check_availability(MEDIA_IS_AUDIO) == CLIENT_AVAILABLE_ONLY_ONE)) {
#ifdef AUD_IN_PATH
            hdoipd_hw_reset(DRV_RST_AUD_EMB_IN);
#endif
            hdoipd_set_vtb_state(VTB_AUD_EMB_OFF);
        }
    }

    if (rsp) {
        rtsp_response_teardown(rsp);
    }

    server->timeout.timeout = 0;
    report(VTB_METHOD "vtb_audio_teardown: remove media");
    multicast_client_remove(MEDIA_IS_AUDIO, server);

    return RTSP_SUCCESS;
}

void vtb_audio_emb_pause(t_rtsp_media *media)
{
    report(VTB_METHOD "vtb_audio_emb_pause");

    media->result = RTSP_RESULT_PAUSE_Q;

    if (hdoipd_tstate(VTB_AUDIO_EMB)) {
        if ((!multicast_get_enabled()) || (multicast_client_check_availability(MEDIA_IS_AUDIO) == CLIENT_AVAILABLE_ONLY_ONE)) {
#ifdef AUD_IN_PATH
            hdoipd_hw_reset(DRV_RST_AUD_EMB_IN);
#endif
            hdoipd_set_vtb_state(VTB_AUD_EMB_IDLE);
        }
    }

    multicast_client_remove(MEDIA_IS_AUDIO, (t_rtsp_server*)media->creator);
}

int vtb_audio_emb_ext_pause(t_rtsp_media* media, void* UNUSED m, t_rtsp_connection* rsp)
{
    vtb_audio_emb_pause(media);
    rtsp_response_pause(rsp, media->sessionid);

    return RTSP_SUCCESS;
}

int vtb_audio_emb_event(t_rtsp_media *media, uint32_t event)
{
    int ret = RTSP_SUCCESS;

    if (rtsp_media_sinit(media))
        return RTSP_WRONG_STATE;

    switch (event) {
        case EVENT_AUDIO_IN0_ON:
            if (rtsp_media_splaying(media)) {
                vtb_audio_emb_pause(media);
                rtsp_server_update_media(media, EVENT_AUDIO_IN0_OFF);
                ret = RTSP_PAUSE;
            }
            rtsp_server_update_media(media, EVENT_AUDIO_IN0_ON);
        break;
        case EVENT_AUDIO_IN0_OFF:
            if (rtsp_media_splaying(media)) {
                vtb_audio_emb_pause(media);
                rtsp_server_update_media(media, EVENT_AUDIO_IN0_OFF);
                ret = RTSP_PAUSE;
            }
        break;
    }
    return ret;
}

t_rtsp_media vtb_audio_emb = {
    .name = "audio_emb",
    .owner = 0,
    .cookie = 0,
    .options = (frtspm*)box_sys_options,
    .describe = vtb_audio_emb_describe,
    .get_parameter = (frtspm*)box_sys_vtb_get_parameter,
    .hdcp = (frtspm*)vtb_audio_emb_hdcp,
    .cookie_size = sizeof(t_multicast_cookie),
    .setup = (frtspm*)vtb_audio_emb_setup,
    .play = (frtspm*)vtb_audio_emb_play,
    .pause = (frtspm*)vtb_audio_emb_ext_pause,
    .teardown = (frtspm*)vtb_audio_emb_teardown,
    .event = (frtspe*)vtb_audio_emb_event
};
