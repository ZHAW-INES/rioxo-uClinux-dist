/*
 * vrb_audio_board.c
 *
 *  Created on: 07.01.2011
 *      Author: alda
 */

#include "hoi_drv_user.h"
#include "hdoipd.h"
#include "box_sys.h"
#include "vrb_audio_board.h"
#include "hdoipd_osd.h"
#include "hdoipd_fsm.h"
#include "hdcp.h"
#include "multicast.h"

#define PROCESSING_DELAY_CORRECTION     (6000)
#define TICK_TIMEOUT                    (hdoipd.eth_timeout)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

static struct {
    t_hdoip_eth         remote;
    int                 timeout;
    int                 alive_ping;
    uint32_t            dst_ip;
    bool                multicast_en;
} vrb;

int vrb_audio_board_setup(t_rscp_media *media, t_rscp_rsp_setup* m, t_rscp_connection* rsp)
{
    int n;
    t_rscp_client* client = media->creator;
    report(VRB_METHOD "vrb_audio_board_setup");

    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac)) != RSCP_SUCCESS) {
        return n;
    }

    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RSCP_SUCCESS) {
        return n;
    }

    vrb.remote.address = client->con.address;
    vrb.remote.aud_port = PORT_RANGE_START(m->transport.server_port);
    hdoipd.local.aud_port = PORT_RANGE_START(m->transport.client_port);

    // get MAC address of next in rout
    if ((n = net_get_remote_hwaddr(hdoipd.listener.sockfd, "eth0", vrb.remote.address, (uint8_t*)&vrb.remote.mac)) != RSCP_SUCCESS) {
        report(" ? net_get_remote_hwaddr failed");
        rscp_err_retry(rsp);
        return RSCP_REQUEST_ERROR;
    }

    vrb.multicast_en = m->transport.multicast;

    if (vrb.multicast_en) {
        vrb.dst_ip = m->transport.multicast_group;
        join_multicast_group(vrb.dst_ip);
    }
    else {
        vrb.dst_ip = hdoipd.local.address;
    }

    REPORT_RTX("RX", hdoipd.local, "<-", vrb.remote, aud);

    // do not output HDCP encrypted content on SDI
    if ((m->hdcp.hdcp_on == 1) && (hdoipd.drivers & DRV_GS2972)) {
        report(INFO "encrypted audio output on SDI is not allowed");
        osd_permanent(true);
        osd_printf("encrypted audio output on SDI is not allowed\n");
        rscp_client_set_teardown(client);
        return RSCP_REQUEST_ERROR;
    }

    /*start hdcp session key exchange if necessary */
    report("Check if HDCP is necessary and start ske");
    hdoipd.hdcp.enc_state = m->hdcp.hdcp_on;

    if ((m->hdcp.hdcp_on == 1) && !(hdoipd.hdcp.ske_executed)){
		if (rscp_client_hdcp(client) != RSCP_SUCCESS){
			report(" ? Session key exchange failed");
			//rscp_err_hdcp(rsp);  DONT CALL THIS AT VRB!!!!!!!!!!!!!!!
			return RSCP_REQUEST_ERROR;
		}
    }

#ifdef ETI_PATH
    hoi_drv_eti(vrb.dst_ip, 0, vrb.remote.address, 0, 0, hdoipd.local.aud_port);
#endif

    hdoipd_set_vtb_state(VTB_AUD_BOARD_IDLE);

    media->result = RSCP_RESULT_READY;
    vrb.alive_ping = 1;
    vrb.timeout = 0;

    return RSCP_SUCCESS;
}

int vrb_audio_board_play(t_rscp_media *media, t_rscp_rsp_play* m, t_rscp_connection UNUSED *rsp)
{
    uint32_t compress = 0;
    report(VRB_METHOD "vrb_audio_board_play");
    char stream_type[20];
    int volume, mic_boost;
    char *aud_source, *mic_boost_str;
    uint32_t test_rtptime;
    t_fec_setting fec;
    struct hdoip_eth_params eth;
    struct hdoip_aud_params aud;
    char dst_mac[6];
    int n;

    eth.ipv4_dst_ip = vrb.remote.address;
    report(INFO "sending unicast to : %s", reg_get("remote-uri"));

    eth.ipv4_src_ip = hdoipd.local.address;
    eth.ipv4_tos = 0;
    eth.ipv4_ttl = 30;
    eth.packet_size = 512;
    eth.rtp_ssrc = 1;
    for(n=0;n<6;n++) eth.src_mac[n] = hdoipd.local.mac[n];
    for(n=0;n<6;n++) eth.dst_mac[n] = vrb.remote.mac[n];
    eth.udp_dst_port = hdoipd.local.aud_port;
    eth.udp_src_port = hdoipd.local.aud_port;

    media->result = RSCP_RESULT_PLAYING;

    hoi_drv_set_stime(1 /*slave_nr*/, m->format.rtptime+PROCESSING_DELAY_CORRECTION-21000); //TODO: set slave timer correctly

    if (reg_test("mode-sync", "streamsync")) {
        compress |= DRV_STREAM_SYNC;
        hdoipd_set_rsc(RSC_AUDIO_IF_BOARD_SYNC);
    }

    volume = atoi(reg_get("audio-hpgain"));
    volume = volume*79/100 - 73;

    hoi_drv_aic23b_dac(AIC23B_ENABLE, volume, m->format.value, m->format.compress, m->format.value2);

    // define own audio parameters
    aud.fs = AUD_FS;
    aud.sample_width = AUD_SAMPLE_WIDTH;
    aud.ch_map = AUD_CH_MAP; // stereo

    // activate audio interface board
    volume = atoi(reg_get("audio-linegain"));                           //get the line in gain
    volume = volume*31/100;
    report(INFO "volume = %u", volume);

    fec.video_enable = 0;
    fec.video_l = 0;
    fec.video_d = 0;
    fec.video_interleaving = 0;
    fec.video_col_only = 0;
    fec.audio_enable = 0;
    fec.audio_l = 0;
    fec.audio_d = 0;
    fec.audio_interleaving = 0;
    fec.audio_col_only = 0;

    aud_source = reg_get("audio-source");                               //get the audio source 
    report(aud_source);
    
    mic_boost_str = reg_get("audio-mic-boost");
    if (mic_boost_str[0] == '0') mic_boost = AIC23B_ADC_MICBOOST_0DB;    //just to choose between 0 and 20dB mic-boost
    else                        mic_boost = AIC23B_ADC_MICBOOST_20DB;

    if (strcmp(aud_source, "mic"))
        hoi_drv_aic23b_adc(AIC23B_ENABLE, AIC23B_ADC_SRC_LINE, volume, mic_boost, &aud);
    else
        hoi_drv_aic23b_adc(AIC23B_ENABLE, AIC23B_ADC_SRC_MIC, volume, mic_boost, &aud);

#ifdef AUD_OUT_PATH
    hoi_drv_aso(AUD_STREAM_NR_IF_BOARD, m->format.value, m->format.compress, m->format.value2, reg_get_int("network-delay"),reg_get_int("av-delay"), compress, 1);
    report(INFO "\naudio if_board streaming started(fs = %d Hz, bitwidth = %d Bit, channel_map = 0x%x)", m->format.value, m->format.compress, m->format.value2);
#endif

    hoi_drv_asi(AUD_STREAM_NR_IF_BOARD, &eth, &aud, &fec);

    hdoipd_set_vtb_state(VTB_AUDIO_BOARD);

    struct in_addr a1; a1.s_addr = vrb.remote.address;
    osd_printf("Streaming unicast Audio %ikHz %i channel from %s\n", m->format.value, aud_chmap2cnt(m->format.value2), inet_ntoa(a1));

    hoi_drv_set_led_status(SDI_OUT_WITH_AUDIO);

    return RSCP_SUCCESS;
}

int vrb_audio_board_teardown(t_rscp_media *media, t_rscp_req_teardown UNUSED *m, t_rscp_connection *rsp)
{
    report(VRB_METHOD "vrb_audio_board_teardown");

    media->result = RSCP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_AUDIO_BOARD|VTB_AUD_BOARD_IDLE)) {
#ifdef AUD_OUT_PATH
        hdoipd_hw_reset(DRV_RST_AUD_BOARD_OUT);
        hdoipd_hw_reset(DRV_RST_AUD_BOARD_IN);
#endif
        hdoipd_clr_rsc(RSC_AUDIO_IF_BOARD_SYNC);
        hdoipd_set_vtb_state(VTB_AUD_BOARD_OFF);
    }

    if (vrb.multicast_en) {
        leave_multicast_group(vrb.dst_ip);
    }

    osd_printf("vrb-audio board connection lost...\n");

    vrb.alive_ping = 1;
    vrb.timeout = 0;

    hoi_drv_set_led_status(SDI_OUT_OFF);

    return RSCP_SUCCESS;
}

int vrb_audio_board_error(t_rscp_media *media, intptr_t m, t_rscp_connection* rsp)
{
	t_rscp_client *client = media->creator;

    if(rsp) {
        report(" ? client failed (%d): %d - %s", m, rsp->ecode, rsp->ereason);
        osd_printf("vrb-audio (board) streaming could not be established: %d - %s\n", rsp->ecode, rsp->ereason);
        switch(rsp->ecode) {
            case 300:   media->result = RSCP_RESULT_SERVER_TRY_LATER;
                        break;
            case 404:   media->result = RSCP_RESULT_SERVER_BUSY;
                        break;
            case 405:   media->result = RSCP_RESULT_SERVER_NO_VTB;
                        break;
            case 406:   media->result = RSCP_RESULT_SERVER_NO_VIDEO_IN;
                        break;
            case 408:   media->result = RSCP_RESULT_SERVER_HDCP_ERROR;
            			rscp_client_set_teardown(client);
	    				// start sending alive packets
            	        if(hdoipd.auto_stream) {
                            alive_check_start_vrb_alive();
            	        }
                        break;
            case 409:   media->result = RSCP_RESULT_SERVER_MULTICAST;
                        break;
            case 400:
            default:    media->result = RSCP_RESULT_SERVER_ERROR;
                        break;
        }
    } else {
        osd_printf("vrb-audio (board) streaming could not be established: connection refused\n");
        media->result = RSCP_RESULT_CONNECTION_REFUSED;
    }
    return RSCP_SUCCESS;
}


void vrb_audio_board_pause(t_rscp_media *media)
{
    report(INFO "vrb_audio_board_pause");
    media->result = RSCP_RESULT_PAUSE_Q;
    report(VRB_METHOD "vrb_audio_board_pause");

    if (hdoipd_tstate(VTB_AUDIO_BOARD)) {

#ifdef AUD_OUT_PATH
        hdoipd_hw_reset(DRV_RST_AUD_BOARD_IN);
        hdoipd_hw_reset(DRV_RST_AUD_BOARD_OUT);
#endif
#ifdef ETI_PATH
        hoi_drv_eti(vrb.dst_ip, 0, vrb.remote.address, 0, hdoipd.local.aud_port, 0);
#endif
        hdoipd_clr_rsc(RSC_AUDIO_IF_BOARD_SYNC);
        hdoipd_set_vtb_state(VTB_AUD_BOARD_IDLE);
    }

    hoi_drv_set_led_status(SDI_OUT_OFF);
}

int vrb_audio_board_ext_pause(t_rscp_media* media, void* UNUSED m, t_rscp_connection* rsp)
{
    vrb_audio_board_pause(media);

    return RSCP_SUCCESS;
}

int vrb_audio_board_update(t_rscp_media *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
{
    switch (m->event) {

        case EVENT_TICK:
            // reset timeout
            vrb.timeout = 0;
        break;

        case EVENT_AUDIO_IN1_ON:
            // No multicast for now...simply stop before starting new
            if (!rscp_media_splaying(media)) {
                vrb_audio_board_pause(media);
                // restart
                rscp_client_set_play(media->creator);
                return RSCP_PAUSE;
            }
        break;
        case EVENT_AUDIO_IN1_OFF:
            vrb_audio_board_pause(media);
            osd_printf("vtb.audio (board) stopped streaming - no audio input\n");
            report(ERROR "vtb.audio (board) stopped streaming - no audio input");
            return RSCP_PAUSE;
        break;
    }

    return RSCP_SUCCESS;
}

int vrb_audio_board_ready(t_rscp_media UNUSED *media)
{
    //if (hdoipd_tstate(VTB_AUD_MASK)) {
    //    // not ready ->
    //    return RSCP_ERRORNO;
    //}
    if (!hdoipd_rsc(RSC_VIDEO_SINK)) {
        // no video sink
        return RSCP_ERRORNO;
    }
    return RSCP_SUCCESS;
}

int vrb_audio_board_dosetup(t_rscp_media *media, void* UNUSED m, void* UNUSED rsp)
{
    int port;
    t_rscp_transport transport;
    t_rscp_client *client = media->creator;
    t_rscp_hdcp hdcp;

    hdcp.hdcp_on = reg_test("hdcp-force", "true");

    report(VRB_METHOD "vrb_audio_board_dosetup");

    if (!client) return RSCP_NULL_POINTER;

    rscp_default_transport(&transport); // TODO: daemon transport configuration
    port = reg_get_int("audio-port-board");
    transport.client_port = PORT_RANGE(htons(port), htons(port+1));

    return rscp_client_setup(client, &transport, 0, &hdcp);
}

int vrb_audio_board_doplay(t_rscp_media *media, void* UNUSED m, void* UNUSED rsp)
{
    t_rscp_rtp_format fmt;
    t_rscp_client *client = media->creator;

    report(VRB_METHOD "vrb_audio_board_doplay");

    if (!client) return RSCP_NULL_POINTER;

    memset(&fmt, 0, sizeof(fmt));
    hoi_drv_get_mtime(&fmt.rtptime);

    return rscp_client_play(client, &fmt);
}

int vrb_audio_board_event(t_rscp_media *media, uint32_t event)
{
    t_rscp_client *client = media->creator;

    switch (event) {
        case EVENT_TICK:
            if (vrb.alive_ping) {
                vrb.alive_ping--;
            } else {
                vrb.alive_ping = TICK_SEND_ALIVE;
                // send tick we are alive (until something like rtcp is used)
         //       if (hdoipd_tstate(VTB_AUDIO)) { // only if audio stream = active
                rscp_client_update(client, EVENT_TICK);
         //       }
            }
            if (vrb.timeout <= TICK_TIMEOUT) {
                vrb.timeout++;
            } else {
                report(INFO "vrb_audio_board_event: timeout");
                // timeout -> kill connection
                vrb.timeout = 0;
                rscp_client_set_kill(client);
                osd_printf("vrb-audio connection lost...\n");
            }
        break;
    }

    return RSCP_SUCCESS;
}

t_rscp_media vrb_audio_board = {
    .name = "audio_board",
    .owner = 0,
    .cookie = 0,
    .setup = (frscpm*)vrb_audio_board_setup,
    .play = (frscpm*)vrb_audio_board_play,
    .pause = (frscpm*)vrb_audio_board_ext_pause,
    .teardown = (frscpm*)vrb_audio_board_teardown,
    .error = (frscpm*)vrb_audio_board_error,
    .update = (frscpm*)vrb_audio_board_update,

    .ready = RSCP_SUCCESS,              //directely ready for streaming!
    .dosetup = (frscpm*)vrb_audio_board_dosetup,
    .doplay = (frscpm*)vrb_audio_board_doplay,
    .event = (frscpe*)vrb_audio_board_event
};
