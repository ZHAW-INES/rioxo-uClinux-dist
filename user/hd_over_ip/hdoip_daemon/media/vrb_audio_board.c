/*
 * vrb_audio_board.c
 *
 *  Created on: 07.01.2011
 *      Author: alda
 */

#include "box_sys.h"
#include "hdcp.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hdoipd_osd.h"
#include "hoi_drv_user.h"
#include "multicast.h"
#include "rtsp_client.h"
#include "rtsp_error.h"
#include "rtsp_net.h"
#include "rtsp_command.h"
#include "vrb_audio.h"

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


int vrb_audio_board_describe(t_rtsp_media *media, void *_data, t_rtsp_connection *con)
{
    t_rtsp_req_describe *data = _data;

    if (!data)
        return -1;

    rtsp_handle_describe_generic(media, data, con);

    /* Add media specific content */

    /* TODO: List all available media controls */

    rtsp_send(con);

    return 0;
}

int vrb_audio_board_setup(t_rtsp_media *media, t_rtsp_rsp_setup* m, t_rtsp_connection* rsp)
{
    int n;
    t_rtsp_client* client = media->creator;
    report(VRB_METHOD "vrb_audio_board_setup");

    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac)) != RTSP_SUCCESS) {
        return n;
    }

    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RTSP_SUCCESS) {
        return n;
    }

    vrb.remote.address = client->con.address;
    vrb.remote.aud_port = PORT_RANGE_START(m->transport.server_port);
    hdoipd.local.aud_port = PORT_RANGE_START(m->transport.client_port);

    // get MAC address of next in rout
    if ((n = net_get_remote_hwaddr(hdoipd.listener.sockfd, "eth0", vrb.remote.address, (uint8_t*)&vrb.remote.mac)) != RTSP_SUCCESS) {
        report(" ? net_get_remote_hwaddr failed");
        rtsp_err_server(rsp);
        return RTSP_REQUEST_ERROR;
    }

    vrb.multicast_en = m->transport.multicast;

    if (vrb.multicast_en) {
        vrb.dst_ip = m->transport.destination;
        multicast_group_join(vrb.dst_ip);
    }
    else {
        vrb.dst_ip = hdoipd.local.address;
    }

    REPORT_RTX("RX", hdoipd.local, "<-", vrb.remote, aud);

#ifdef ETI_PATH
    hoi_drv_eti(vrb.dst_ip, 0, vrb.remote.address, 0, 0, htons(reg_get_int("audio-port-board")));
#endif

    hdoipd_set_vtb_state(VTB_AUD_BOARD_IDLE);

    media->result = RTSP_RESULT_READY;
    vrb.alive_ping = 1;
    vrb.timeout = 0;

    return RTSP_SUCCESS;
}

int vrb_audio_board_play(t_rtsp_media *media, t_rtsp_rsp_play* m, t_rtsp_connection UNUSED *rsp)
{
    uint32_t compress = 0;
    report(VRB_METHOD "vrb_audio_board_play");
    char stream_type[20];
    int volume, mic_boost;
    char *aud_source, *mic_boost_str;
    t_fec_setting fec;
    struct hdoip_eth_params eth;
    struct hdoip_aud_params aud;
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

    media->result = RTSP_RESULT_PLAYING;

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
    hoi_drv_aso(AUD_STREAM_NR_IF_BOARD, m->format.value, m->format.compress, m->format.value2, reg_get_int("network-delay"),reg_get_int("av-delay"), compress, 1 /*vrb: i2s control active*/);
    report(INFO "\naudio if_board streaming started(fs = %d Hz, bitwidth = %d Bit, channel_map = 0x%x)", m->format.value, m->format.compress, m->format.value2);
#endif

    hoi_drv_asi(AUD_STREAM_NR_IF_BOARD, &eth, &aud, &fec);

    hdoipd_set_vtb_state(VTB_AUDIO_BOARD);

    struct in_addr a1; a1.s_addr = vrb.remote.address;
    osd_printf("Streaming unicast Audio %ikHz %i channel from %s\n", m->format.value, aud_chmap2cnt(m->format.value2), inet_ntoa(a1));

    hoi_drv_set_led_status(SDI_OUT_WITH_AUDIO);

    return RTSP_SUCCESS;
}

int vrb_audio_board_teardown(t_rtsp_media *media, t_rtsp_req_teardown UNUSED *m, t_rtsp_connection *rsp)
{
    report(VRB_METHOD "vrb_audio_board_teardown");

    media->result = RTSP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_AUDIO_BOARD|VTB_AUD_BOARD_IDLE)) {
#ifdef AUD_OUT_PATH
        hdoipd_hw_reset(DRV_RST_AUD_BOARD_OUT);
        hdoipd_hw_reset(DRV_RST_AUD_BOARD_IN);
#endif
        hdoipd_clr_rsc(RSC_AUDIO_IF_BOARD_SYNC);
        hdoipd_set_vtb_state(VTB_AUD_BOARD_OFF);
    }

    if (vrb.multicast_en) {
        multicast_group_leave(vrb.dst_ip);
    }

    osd_printf("vrb-audio board connection lost...\n");

    vrb.alive_ping = 1;
    vrb.timeout = 0;

    hoi_drv_set_led_status(SDI_OUT_OFF);

    return RTSP_SUCCESS;
}

int vrb_audio_board_error(t_rtsp_media *media, intptr_t m, t_rtsp_connection* rsp)
{
	t_rtsp_client *client = media->creator;

    if(rsp) {
        report(" ? client failed (%d): %d - %s", m, rsp->ecode, rsp->ereason);
        osd_printf("vrb-audio (board) streaming could not be established: %d - %s\n", rsp->ecode, rsp->ereason);
        switch(rsp->ecode) {
            case 300:   media->result = RTSP_RESULT_SERVER_TRY_LATER;
                        break;
            case 404:   media->result = RTSP_RESULT_SERVER_BUSY;
                        break;
            case 405:   media->result = RTSP_RESULT_SERVER_NO_VTB;
                        break;
            case 406:   media->result = RTSP_RESULT_SERVER_NO_VIDEO_IN;
                        break;
            case 408:   media->result = RTSP_RESULT_SERVER_HDCP_ERROR;
            			rtsp_client_set_teardown(client);
                        break;
            case 409:   media->result = RTSP_RESULT_SERVER_MULTICAST;
                        break;
            case RTSP_STATUS_INTERNAL_SERVER_ERROR:
            default:    media->result = RTSP_RESULT_SERVER_ERROR;
                        break;
        }
    } else {
        osd_printf("vrb-audio (board) streaming could not be established: connection refused\n");
        media->result = RTSP_RESULT_CONNECTION_REFUSED;
    }
    return RTSP_SUCCESS;
}


void vrb_audio_board_pause(t_rtsp_media *media)
{
    report(INFO "vrb_audio_board_pause");
    media->result = RTSP_RESULT_PAUSE_Q;
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

int vrb_audio_board_ext_pause(t_rtsp_media* media, void* UNUSED m, t_rtsp_connection* rsp)
{
    vrb_audio_board_pause(media);

    return RTSP_SUCCESS;
}

int vrb_audio_board_update(t_rtsp_media *media, t_rtsp_req_update *m, t_rtsp_connection UNUSED *rsp)
{
    switch (m->event) {

        case EVENT_TICK:
            // reset timeout
            vrb.timeout = 0;
        break;

        case EVENT_AUDIO_IN1_ON:
            // No multicast for now...simply stop before starting new
            if (!rtsp_media_splaying(media)) {
                vrb_audio_board_pause(media);
                // restart
                rtsp_client_set_play(media->creator);
                return RTSP_PAUSE;
            }
        break;
        case EVENT_AUDIO_IN1_OFF:
            vrb_audio_board_pause(media);
            osd_printf("vtb.audio (board) stopped streaming - no audio input\n");
            report(ERROR "vtb.audio (board) stopped streaming - no audio input");
            return RTSP_PAUSE;
        break;
    }

    return RTSP_SUCCESS;
}

int vrb_audio_board_ready(t_rtsp_media UNUSED *media)
{
    if (!hdoipd_rsc(RSC_VIDEO_SINK)) {
        //if NO video sink
        return RTSP_ERRORNO;
    }
    return RTSP_SUCCESS;
}

int vrb_audio_board_dosetup(t_rtsp_media *media, void* UNUSED m, void* UNUSED rsp)
{
    int port;
    t_rtsp_transport transport;
    t_rtsp_client *client = media->creator;
    t_rtsp_hdcp hdcp;

    hdcp.hdcp_on = 0; // no HDCP

    report(VRB_METHOD "vrb_audio_board_dosetup");

    if (!client) return RTSP_NULL_POINTER;

    rtsp_default_transport(&transport); // TODO: daemon transport configuration
    port = reg_get_int("audio-port-board");
    transport.client_port = PORT_RANGE(htons(port), htons(port+1));

    return rtsp_client_setup(client, media, &transport, NULL, &hdcp);
}

int vrb_audio_board_doplay(t_rtsp_media *media, void* UNUSED m, void* UNUSED rsp)
{
    t_rtsp_client *client = media->creator;

    report(VRB_METHOD "vrb_audio_board_doplay");

    if (!client) return RTSP_NULL_POINTER;

    return rtsp_client_play(client, NULL, media->name);
}

int vrb_audio_board_event(t_rtsp_media *media, uint32_t event)
{
    return RTSP_SUCCESS;
}

t_rtsp_media vrb_audio_board = {
    .name = "audio_board",
    .owner = 0,
    .cookie = 0,
    .options = (frtspm*)box_sys_options,
    .describe = vrb_audio_board_describe,
    .setup = (frtspm*)vrb_audio_board_setup,
    .play = (frtspm*)vrb_audio_board_play,
    .pause = (frtspm*)vrb_audio_board_ext_pause,
    .teardown = (frtspm*)vrb_audio_board_teardown,
    .error = (frtspm*)vrb_audio_board_error,
    .update = (frtspm*)vrb_audio_board_update,

    .ready = RTSP_SUCCESS,              //directely ready for streaming!
    .dosetup = (frtspm*)vrb_audio_board_dosetup,
    .doplay = (frtspm*)vrb_audio_board_doplay,
    .event = (frtspe*)vrb_audio_board_event
};
