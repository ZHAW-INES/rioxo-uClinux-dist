/*
 * vrb_audio_emb.c
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
#include "vrb_audio_emb.h"

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

int vrb_audio_emb_describe(t_rtsp_media *media, void *_data, t_rtsp_connection *con)
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

int vrb_audio_emb_setup(t_rtsp_media *media, t_rtsp_rsp_setup* m, t_rtsp_connection* rsp)
{
    int n;
    t_rtsp_client* client = media->creator;
    report(VRB_METHOD "vrb_audio_emb_setup");

    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac)) != RTSP_SUCCESS) {
        return n;
    }

    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RTSP_SUCCESS) {
        return n;
    }

    vrb.remote.address = client->con.address;
    vrb.remote.aud_port = PORT_RANGE_START(m->transport.server_port);
    hdoipd.local.aud_port = PORT_RANGE_START(m->transport.client_port);

    vrb.multicast_en = m->transport.multicast;

    if (vrb.multicast_en) {
        vrb.dst_ip = m->transport.destination;
        multicast_group_join(vrb.dst_ip);
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
        rtsp_client_set_teardown(client);
		rtsp_client_set_kill(client);
        return RTSP_REQUEST_ERROR;
    }

    /*start hdcp session key exchange if necessary */
    report("Check if HDCP is necessary and start ske");
    hdoipd.hdcp.enc_state = m->hdcp.hdcp_on;

    if ((m->hdcp.hdcp_on == 1) && !(hdoipd.hdcp.ske_executed)){
		if (rtsp_client_hdcp(client) != RTSP_SUCCESS){
			report(" ? Session key exchange failed");
			//rtsp_err_hdcp(rsp);  DONT CALL THIS AT VRB!!!!!!!!!!!!!!!
			return RTSP_REQUEST_ERROR;
		}
    }

#ifdef ETI_PATH
    hoi_drv_eti(vrb.dst_ip, 0, vrb.remote.address, 0, hdoipd.local.aud_port, 0);
#endif

    hdoipd_set_vtb_state(VTB_AUD_EMB_IDLE);

    media->result = RTSP_RESULT_READY;
    vrb.alive_ping = 1;
    vrb.timeout = 0;

    return RTSP_SUCCESS;
}

int vrb_audio_emb_play(t_rtsp_media *media, t_rtsp_rsp_play* m, t_rtsp_connection UNUSED *rsp)
{
    uint32_t compress = 0;
    report(VRB_METHOD "vrb_audio_emb_play");
    char stream_type[20];

    // Set Stream Sync
    if (!hdoipd_rsc(RSC_VIDEO_SYNC)){            //if no video stream synced -> sync to audio stream
        hoi_drv_stsync(DRV_ST_AUDIO);            //stream sync to AUDIO
    }

    //Test if HDCP parameters were set correctly
	if (hdoipd.hdcp.enc_state && !(get_hdcp_status() & HDCP_ETI_AUDIO_EN)){
		if (hdoipd.hdcp.ske_executed){
			hoi_drv_hdcp(hdoipd.hdcp.keys); 	/* write keys to kernel */
			report(INFO "Audio encryption enabled (eti)!");
			hoi_drv_hdcp_auden_eti();
		}
		else {
			report(INFO "No valid HDCP ske executed!");
			return RTSP_ERRORNO;
		}
	}

    // set slave timer when not already synced to video or audio_embedded
    if (!hdoipd_rsc(RSC_VIDEO_SYNC) && !hdoipd_rsc(RSC_AUDIO_EMB_SYNC)) {
        hoi_drv_set_stime(0 /*slave_nr*/, m->format.rtptime+PROCESSING_DELAY_CORRECTION-21000);
    }

    if (reg_test("mode-sync", "streamsync")) {
        if (reg_test("sync-target", "audio") || (!hdoipd_rsc(RSC_VIDEO_SYNC) && !hdoipd_rsc(RSC_AUDIO_EMB_SYNC))) {
            compress |= DRV_STREAM_SYNC;
            hdoipd_set_rsc(RSC_AUDIO_EMB_SYNC);
        }
    }

#ifdef AUD_OUT_PATH
    hoi_drv_aso(AUD_STREAM_NR_EMBEDDED, m->format.value, m->format.compress, m->format.value2, reg_get_int("network-delay"),reg_get_int("av-delay"), compress, 1);
    report(INFO "\naudio embedded streaming started(fs = %d Hz, bitwidth = %d Bit, channel_map = 0x%x)", m->format.value, m->format.compress, m->format.value2);
#endif

    // set audio channel status
    hoi_drv_acs(m->acs);

    hdoipd_set_vtb_state(VTB_AUDIO_EMB);
    hdoipd_set_rsc(RSC_AUDIO_EMB_OUT);


    if (vrb.multicast_en) {
        snprintf(stream_type, 20, " Multicast ");
    } else {
        snprintf(stream_type, 20, " ");
    }

    struct in_addr a1; a1.s_addr = vrb.remote.address;
    osd_printf("Streaming%saudio from video board %ikHz %i channel from %s\n", stream_type, m->format.value, aud_chmap2cnt(m->format.value2), inet_ntoa(a1));

    hoi_drv_set_led_status(SDI_OUT_WITH_AUDIO);
    media->result = RTSP_RESULT_PLAYING;

    return RTSP_SUCCESS;
}

int vrb_audio_emb_teardown(t_rtsp_media *media, t_rtsp_req_teardown UNUSED *m, t_rtsp_connection *rsp)
{
    report(VRB_METHOD "vrb_audio_emb_teardown");

    media->result = RTSP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_AUDIO_EMB|VTB_AUD_EMB_IDLE)) {
#ifdef AUD_OUT_PATH
        hdoipd_hw_reset(DRV_RST_AUD_EMB_OUT);
#endif
        hdoipd_clr_rsc(RSC_AUDIO_EMB_OUT|RSC_AUDIO_EMB_SYNC);
        hdoipd_set_vtb_state(VTB_AUD_EMB_OFF);
    }

    if (vrb.multicast_en) {
        multicast_group_leave(vrb.dst_ip);
    }

    osd_printf("vrb-audio embedded connection lost...\n");

    vrb.alive_ping = 1;
    vrb.timeout = 0;

    hoi_drv_set_led_status(SDI_OUT_OFF);

    return RTSP_SUCCESS;
}

int vrb_audio_emb_error(t_rtsp_media *media, intptr_t m, t_rtsp_connection* rsp)
{
	t_rtsp_client *client = media->creator;

    if(rsp) {
        report(" ? client failed (%d): %d - %s", m, rsp->ecode, rsp->ereason);
        osd_printf("vrb-audio (embedded) streaming could not be established: %d - %s\n", rsp->ecode, rsp->ereason);
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
            case 409:   break;
            case RTSP_STATUS_INTERNAL_SERVER_ERROR:
            default:    media->result = RTSP_RESULT_SERVER_ERROR;
                        break;
        }
    } else {
        osd_printf("vrb-audio (embedded) streaming could not be established: connection refused\n");
        media->result = RTSP_RESULT_CONNECTION_REFUSED;
    }
    return RTSP_SUCCESS;
}

void vrb_audio_emb_pause(t_rtsp_media *media)
{
	report(INFO "vrb_audio_emb_pause");
    media->result = RTSP_RESULT_PAUSE_Q;
    report(VRB_METHOD "vrb_audio_emb_pause");

    if (hdoipd_tstate(VTB_AUDIO_EMB)) {

#ifdef AUD_OUT_PATH
        hdoipd_hw_reset(DRV_RST_AUD_EMB_OUT);
#endif
#ifdef ETI_PATH
        hoi_drv_eti(vrb.dst_ip, 0, vrb.remote.address, 0, hdoipd.local.aud_port, 0);
#endif
        hdoipd_clr_rsc(RSC_AUDIO_EMB_OUT|RSC_AUDIO_EMB_SYNC);
        hdoipd_set_vtb_state(VTB_AUD_EMB_IDLE);
    }

    hoi_drv_set_led_status(SDI_OUT_OFF);
}

int vrb_audio_emb_ext_pause(t_rtsp_media* media, void* UNUSED m, t_rtsp_connection* rsp)
{
    vrb_audio_emb_pause(media);

    return RTSP_SUCCESS;
}

int vrb_audio_emb_update(t_rtsp_media *media, t_rtsp_req_update *m, t_rtsp_connection UNUSED *rsp)
{
    switch (m->event) {

        case EVENT_TICK:
            // reset timeout
            vrb.timeout = 0;
        break;

        case EVENT_AUDIO_IN0_ON:
            // No multicast for now...simply stop before starting new
            if (!rtsp_media_splaying(media)) {
                vrb_audio_emb_pause(media);
                // restart
                rtsp_client_set_play(media->creator);
                return RTSP_PAUSE;
            }
        break;
        case EVENT_AUDIO_IN0_OFF:
            vrb_audio_emb_pause(media);
            osd_printf("vtb.audio (embedded) stopped streaming - no audio input\n");
            report(ERROR "vtb.audio (embedded) stopped streaming - no audio input");
            return RTSP_PAUSE;
        break;
    }

    return RTSP_SUCCESS;
}

int vrb_audio_emb_ready(t_rtsp_media UNUSED *media)
{
    if (!hdoipd_rsc(RSC_VIDEO_SINK)) {
        //no video sink
        return RTSP_ERRORNO;
    }
    return RTSP_SUCCESS;
}

int vrb_audio_emb_dosetup(t_rtsp_media *media, void* UNUSED m, void* UNUSED rsp)
{
    int port;
    t_rtsp_transport transport;
    t_rtsp_client *client = media->creator;
    t_rtsp_hdcp hdcp;

    hdcp.hdcp_on = reg_test("hdcp-force", "true");

    report(VRB_METHOD "vrb_audio_emb_dosetup");

    if (!client) return RTSP_NULL_POINTER;

    rtsp_default_transport(&transport); // TODO: daemon transport configuration
    port = reg_get_int("audio-port");
    transport.client_port = PORT_RANGE(htons(port), htons(port+1));

    return rtsp_client_setup(client, media, &transport, NULL, &hdcp);
}

int vrb_audio_emb_doplay(t_rtsp_media *media, void* UNUSED m, void* UNUSED rsp)
{
    t_rtsp_client *client = media->creator;

    report(VRB_METHOD "vrb_audio_emb_doplay");

    if (!client) return RTSP_NULL_POINTER;

    return rtsp_client_play(client, NULL, media->name);
}

int vrb_audio_emb_event(t_rtsp_media *media, uint32_t event)
{
    t_rtsp_client *client = media->creator;
    int ret;

    switch (event) {
        case EVENT_VIDEO_SINK_ON:
            if (!rtsp_media_sinit(media))
                return RTSP_WRONG_STATE;

            ret = rtsp_media_setup(media);
            if (ret == RTSP_SUCCESS)
              rtsp_media_play(media);
            else {
                report(ERROR "vrb_audio_event() rtsp_media_setup failed (%d)", ret);
                rtsp_client_set_kill(client);
                return ret;
            }
            break;

        case EVENT_VIDEO_SINK_OFF:
            if (rtsp_media_sinit(media))
                return RTSP_WRONG_STATE;

            // Note: after teardown call media/client is not anymore a valid struct
            rtsp_client_set_teardown(client);
            break;
    }

    return RTSP_SUCCESS;
}

t_rtsp_media vrb_audio_emb = {
    .name = "audio_emb",
    .owner = 0,
    .cookie = 0,
    .options = (frtspm*)box_sys_options,
    .describe = vrb_audio_emb_describe,
    .setup = (frtspm*)vrb_audio_emb_setup,
    .play = (frtspm*)vrb_audio_emb_play,
    .pause = (frtspm*)vrb_audio_emb_ext_pause,
    .teardown = (frtspm*)vrb_audio_emb_teardown,
    .error = (frtspm*)vrb_audio_emb_error,
    .update = (frtspm*)vrb_audio_emb_update,

    .ready = (frtspl*)vrb_audio_emb_ready,
    .dosetup = (frtspm*)vrb_audio_emb_dosetup,
    .doplay = (frtspm*)vrb_audio_emb_doplay,
    .event = (frtspe*)vrb_audio_emb_event
};
