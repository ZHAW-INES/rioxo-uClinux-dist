/*
 * vtb_audio_board.c
 *
 *  Created on: 06.01.2011
 *      Author: alda
 */
#include "hdoipd.h"
#include "hoi_drv_user.h"
#include "vtb_audio_board.h"
#include "hdoipd_fsm.h"
#include "box_sys.h"
#include "rscp_server.h"
#include "hdcp.h"
#include "multicast.h"

#define PROCESSING_DELAY_CORRECTION     (6000)
#define TICK_TIMEOUT_MULTICAST          (hdoipd.eth_timeout * 2)
#define TICK_TIMEOUT_UNICAST            (hdoipd.eth_timeout)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

int vtb_audio_board_hdcp(t_rscp_media* media, t_rscp_req_hdcp* m, t_rscp_connection* rsp)
{
    return 0;
}

int vtb_audio_board_setup(t_rscp_media* media, t_rscp_req_setup* m, t_rscp_connection* rsp)
{
    int hdcp;
    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_audio_board_setup");

    set_multicast_enable(reg_test("multicast_en", "true"));

    if (!hdoipd_state(HOID_VTB)) {
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

    // reserve resource
    if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_NOT_AVAILABLE)) {
        hdoipd_set_vtb_state(VTB_AUD_BOARD_IDLE);
    }

    //check if hdcp is forced by HDMI, user or client (over RSCP)
    if (reg_test("hdcp-force", "true") || hdoipd_rsc(RSC_VIDEO_IN_HDCP) || (m->hdcp.hdcp_on==1)) {
    	m->hdcp.hdcp_on = 1;
    	hdoipd.hdcp.enc_state = HDCP_ENABLED;
    }

    cookie->remote.address = rsp->address;
    cookie->remote.aud_port = PORT_RANGE_START(m->transport.client_port);
    m->transport.server_port = PORT_RANGE(hdoipd.local.aud_port, hdoipd.local.aud_port);
    m->transport.multicast = get_multicast_enable();

    hoi_drv_eti(hdoipd.local.address, 0, cookie->remote.address, 0, 0, cookie->remote.aud_port);

    rscp_response_setup(rsp, &m->transport, media->sessionid, &m->hdcp);

    REPORT_RTX("TX", hdoipd.local, "->", cookie->remote, aud);

    media->result = RSCP_RESULT_READY;

    cookie->timeout = 0;
    cookie->alive_ping = 1;

    return RSCP_SUCCESS;
}

int vtb_audio_board_play(t_rscp_media* media, t_rscp_req_play UNUSED *m, t_rscp_connection* rsp)
{

    int n, hdcp;
    t_rscp_rtp_format fmt;
    struct hdoip_eth_params eth;
    struct hdoip_aud_params aud;
    char dst_mac[6];
    t_fec_setting fec;
    char *fec_setting;
    int volume, mic_boost;
    char *volume_str, *aud_source, *mic_boost_str;

    t_multicast_cookie* cookie = media->cookie;

    media->result = RSCP_RESULT_PLAYING;

    report(VTB_METHOD "vtb_audio_board_play");

    if (!hdoipd_state(HOID_VTB)) {
        // we don't have the resource reserved
        report(" ? require state VTB");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    if (!hdoipd_rsc(RSC_AUDIO_BOARD_IN)) {
        // currently no audio input active
        report(" ? require active audio input");
        rscp_err_no_source(rsp);
        hdoipd_set_vtb_state(VTB_AUD_BOARD_IDLE);
        return RSCP_REQUEST_ERROR;
    }

    // network settings
    // SETTINGS FOR BIDIRECTIONAL -> receive!!
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
    eth.rtp_ssrc = 1;
    for(n=0;n<6;n++) eth.src_mac[n] = hdoipd.local.mac[n];
    for(n=0;n<6;n++) eth.dst_mac[n] = dst_mac[n];
    eth.udp_dst_port = cookie->remote.aud_port;
    eth.udp_src_port = hdoipd.local.aud_port;

    hoi_drv_set_stime(1, m->format.rtptime+PROCESSING_DELAY_CORRECTION-21000);

    // define own audio parameters
    aud.fs = AUD_FS;
    aud.sample_width = AUD_SAMPLE_WIDTH;
    aud.ch_map = AUD_CH_MAP; // stereo
    // activate audio interface board
    volume = atoi(reg_get("audio-linegain"));                           //get the line in gain
    volume = volume*31/100;
    report(INFO "volume = %u", volume);

    aud_source = reg_get("audio-source");                               //get the audio source 
    report(aud_source);
    
    mic_boost_str = reg_get("audio-mic-boost");
    if(mic_boost_str[0] == '0') mic_boost = AIC23B_ADC_MICBOOST_0DB;    //just to choose between 0 and 20dB mic-boost
    else                        mic_boost = AIC23B_ADC_MICBOOST_20DB;

    if(strcmp(aud_source, "mic"))
        hoi_drv_aic23b_adc(AIC23B_ENABLE, AIC23B_ADC_SRC_LINE, volume, mic_boost, &aud);
    else
        hoi_drv_aic23b_adc(AIC23B_ENABLE, AIC23B_ADC_SRC_MIC, volume, mic_boost, &aud);

    fmt.compress = aud.sample_width;
    fmt.value = aud.fs;
    fmt.value2 = (uint32_t) aud.ch_map;
    hoi_drv_get_mtime(&fmt.rtptime);
    
    volume = atoi(reg_get("audio-hpgain"));
    volume = volume*79/100 - 73;
    hoi_drv_aic23b_dac(AIC23B_ENABLE, volume, fmt.value, fmt.compress, fmt.value2);

    if ( ((aud.sample_width < 8) || (aud.sample_width > 32)) ||
         ((aud.fs < 32000) || (aud.fs > 192000)) ||
         ((aud.ch_map < 0x0001) || (aud.ch_map > 0x00FF)) ) {
          rscp_err_def_source(rsp);
          return RSCP_REQUEST_ERROR;
    }

    // fec settings (convert from ascii to integer)
    fec_setting = reg_get("fec_setting");
    fec.video_enable = fec_setting[0] - 48;
    fec.video_l = fec_setting[1] - 48 + 2;
    fec.video_d = fec_setting[2] - 48 + 2;
    fec.video_interleaving = fec_setting[3] - 48;
    fec.video_col_only = fec_setting[4] - 48;
    fec.audio_enable =fec_setting[5] - 48;
    fec.audio_l = fec_setting[6] - 48 + 2;
    fec.audio_d = fec_setting[7] - 48 + 2;
    fec.audio_interleaving = fec_setting[8] - 48;
    fec.audio_col_only = fec_setting[9] - 48;

    // send timing
    rscp_response_play(rsp, media->sessionid, &fmt, 0);

    if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_NOT_AVAILABLE)) {
        // activate asi
#ifdef AUD_IN_PATH
        hoi_drv_asi(AUD_STREAM_NR_IF_BOARD, &eth, &aud, &fec);
        hoi_drv_aso(AUD_STREAM_NR_IF_BOARD, fmt.value, fmt.compress, fmt.value2, reg_get_int("network-delay"),reg_get_int("av-delay"), DRV_STREAM_SYNC, 0);
        report(INFO "\naudio in if_board streaming started(fs = %d Hz, bitwidth = %d Bit, channel_map = 0x%02X)", aud.fs, aud.sample_width, aud.ch_map);
        report(INFO "\naudio out if_board streaming started(fs = %d Hz, bitwidth = %d Bit, channel_map = 0x%x)", fmt.value, fmt.compress, fmt.value2);
#endif

        // We are streaming Audio now...
        hdoipd_set_vtb_state(VTB_AUDIO_BOARD);
    }

    add_client_to_vtb(MEDIA_IS_AUDIO, cookie->remote.address);

    return RSCP_SUCCESS;
}


// TODO: stop only the stream associated with rscp connection
int vtb_audio_board_teardown(t_rscp_media* media, t_rscp_req_teardown UNUSED *m, t_rscp_connection* rsp)
{
    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_audio_board_teardown");

    media->result = RSCP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_AUD_BOARD_MASK)) {
        if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_AVAILABLE_ONLY_ONE)) {
#ifdef AUD_IN_PATH
            hdoipd_hw_reset(DRV_RST_AUD_BOARD_IN);
            hdoipd_hw_reset(DRV_RST_AUD_BOARD_OUT);
#endif
            hdoipd_set_vtb_state(VTB_AUD_BOARD_OFF);
        }
    }

    if (rsp) {
        rscp_response_teardown(rsp, media->sessionid);
    }

    cookie->timeout = 0;
    cookie->alive_ping = 1;

    remove_client_from_vtb(MEDIA_IS_AUDIO, cookie->remote.address);

    return RSCP_SUCCESS;
}

void vtb_audio_board_pause(t_rscp_media *media)
{
    t_multicast_cookie* cookie = media->cookie;

    report(VTB_METHOD "vtb_audio_board_pause");

    media->result = RSCP_RESULT_PAUSE_Q;

    if (hdoipd_tstate(VTB_AUDIO_BOARD)) {
        if ((!get_multicast_enable()) || (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_AVAILABLE_ONLY_ONE)) {
#ifdef AUD_IN_PATH
            hdoipd_hw_reset(DRV_RST_AUD_BOARD_IN);
            hdoipd_hw_reset(DRV_RST_AUD_BOARD_OUT);
#endif
            hdoipd_set_vtb_state(VTB_AUD_BOARD_IDLE);
        }
    }
    hoi_drv_eti(hdoipd.local.address, 0, cookie->remote.address, 0, 0, hdoipd.local.aud_port);

    remove_client_from_vtb(MEDIA_IS_AUDIO, cookie->remote.address);
}

int vtb_audio_board_ext_pause(t_rscp_media* media, void* UNUSED m, t_rscp_connection* rsp)
{
    vtb_audio_board_pause(media);
    rscp_response_pause(rsp, media->sessionid);

    return RSCP_SUCCESS;
}

int vtb_audio_board_update(t_rscp_media UNUSED *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
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

int vtb_audio_board_event(t_rscp_media *media, uint32_t event)
{
    t_multicast_cookie* cookie = media->cookie;
    uint32_t timeout;

    switch (event) {
        case EVENT_AUDIO_IN1_ON:
            if (rscp_media_splaying(media)) {
                vtb_audio_board_pause(media);
                rscp_server_update(media, EVENT_AUDIO_IN1_OFF);
            }
            rscp_server_update(media, EVENT_AUDIO_IN1_ON);
            return RSCP_PAUSE;
        break;
        case EVENT_AUDIO_IN1_OFF:
            if (rscp_media_splaying(media)) {
                vtb_audio_board_pause(media);
                rscp_server_update(media, EVENT_AUDIO_IN1_OFF);
            }
            return RSCP_PAUSE;
        break;
        case EVENT_TICK:
            if (cookie->alive_ping) {
                cookie->alive_ping--;
            } else {
                cookie->alive_ping = TICK_SEND_ALIVE;
                // send tick we are alive (until something like rtcp is used)
       //         if (hdoipd_tstate(VTB_AUDIO)) { // only if audio stream = active
                rscp_server_update(media, EVENT_TICK);
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
                report(INFO "vtb_audio_board_event: timeout");
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

t_rscp_media vtb_audio_board = {
    .name = "audio_board",
    .owner = 0,
    .cookie = 0,
    .hdcp = (frscpm*)vtb_audio_board_hdcp,
    .cookie_size = sizeof(t_multicast_cookie),
    .setup = (frscpm*)vtb_audio_board_setup,
    .play = (frscpm*)vtb_audio_board_play,
    .pause = (frscpm*)vtb_audio_board_ext_pause,
    .teardown = (frscpm*)vtb_audio_board_teardown,
    .update = (frscpm*)vtb_audio_board_update,
    .event = (frscpe*)vtb_audio_board_event
};
