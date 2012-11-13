/*
 * vrb_audio.c
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

int vrb_audio_setup(t_rtsp_media *media, t_rtsp_rsp_setup* m, t_rtsp_connection* rsp)
{
    int n;
    t_rtsp_client* client = media->creator;
    report(VRB_METHOD "vrb_audio_setup");

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
        rtsp_client_set_teardown(client);
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
    // TODO: separete Audio/Video
    hoi_drv_eti(vrb.dst_ip, 0, vrb.remote.address, 0, hdoipd.local.aud_port);
#endif

    hdoipd_set_vtb_state(VTB_AUD_IDLE);

    media->result = RTSP_RESULT_READY;
    vrb.alive_ping = 1;
    vrb.timeout = 0;

    return RTSP_SUCCESS;
}


int vrb_audio_play(t_rtsp_media *media, t_rtsp_rsp_play* m, t_rtsp_connection UNUSED *rsp)
{
    uint32_t compress = 0;
    report(VRB_METHOD "vrb_audio_play");
    char stream_type[20];

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

    media->result = RTSP_RESULT_PLAYING;

    // set slave timer when not already synced
    if (!hdoipd_rsc(RSC_SYNC)) {
        hoi_drv_set_stime(m->format.rtptime+PROCESSING_DELAY_CORRECTION-21000); //TODO: set slave timer correctly
    }

    if (reg_test("mode-sync", "streamsync")) {
        if (reg_test("sync-target", "audio") || !hdoipd_rsc(RSC_SYNC)) {
            compress |= DRV_STREAM_SYNC;
            hdoipd_set_rsc(RSC_AUDIO_SYNC);
        }
    }

#ifdef AUD_OUT_PATH
    // start aso (TODO: delay = 4*vframe+networkdelay)
    uint8_t channel_select[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    hoi_drv_aso(m->format.value, 1000, m->format.compress, m->format.value2, channel_select, reg_get_int("network-delay"),reg_get_int("av-delay"), compress);
    report(INFO "\naudio streaming started(fs = %d Hz, bitwidth = %d Bit, channels = %d)", m->format.value, m->format.compress, m->format.value2);
#endif

    hdoipd_set_vtb_state(VTB_AUDIO);
    hdoipd_set_rsc(RSC_AUDIO_OUT);


    if (vrb.multicast_en) {
        snprintf(stream_type, 20, " Multicast ");
    } else {
        snprintf(stream_type, 20, " ");
    }

    struct in_addr a1; a1.s_addr = vrb.remote.address;
    osd_printf("Streaming%sAudio %ikHz %i channel from %s\n", stream_type, m->format.value, m->format.value2, inet_ntoa(a1));

    hoi_drv_set_led_status(SDI_OUT_WITH_AUDIO);

    return RTSP_SUCCESS;
}


int vrb_audio_teardown(t_rtsp_media *media, t_rtsp_req_teardown UNUSED *m, t_rtsp_connection *rsp)
{
    report(VRB_METHOD "vrb_audio_teardown");

    media->result = RTSP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_AUDIO|VTB_AUD_IDLE)) {
#ifdef AUD_OUT_PATH
        hdoipd_hw_reset(DRV_RST_AUD_OUT);
#endif
        hdoipd_clr_rsc(RSC_AUDIO_OUT|RSC_AUDIO_SYNC);
        hdoipd_set_vtb_state(VTB_AUD_OFF);
    }

    if (vrb.multicast_en) {
        leave_multicast_group(vrb.dst_ip);
    }

    osd_printf("vrb.audio connection lost...\n");

    vrb.alive_ping = 1;
    vrb.timeout = 0;

    hoi_drv_set_led_status(SDI_OUT_OFF);

    return RTSP_SUCCESS;
}

int vrb_audio_error(t_rtsp_media *media, intptr_t m, t_rtsp_connection* rsp)
{
	t_rtsp_client *client = media->creator;

    if(rsp) {
        report(" ? client failed (%d): %d - %s", m, rsp->ecode, rsp->ereason);
        osd_printf("vrb.audio streaming could not be established: %d - %s\n", rsp->ecode, rsp->ereason);
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
	    				// start sending alive packets
            	        if(hdoipd.auto_stream) {
                            alive_check_start_vrb_alive();
            	        }
                        break;
            case 400:
            default:    media->result = RTSP_RESULT_SERVER_ERROR;
                        break;
        }
    } else {
        osd_printf("vrb.audio streaming could not be established: connection refused\n");
        media->result = RTSP_RESULT_CONNECTION_REFUSED;
    }
    return RTSP_SUCCESS;
}


void vrb_audio_pause(t_rtsp_media *media)
{
	report(INFO "vrb_audio_pause");
    media->result = RTSP_RESULT_PAUSE_Q;
    report(VRB_METHOD "vrb_audio_pause");

    if (hdoipd_tstate(VTB_AUDIO)) {

#ifdef AUD_OUT_PATH
        hdoipd_hw_reset(DRV_RST_AUD_OUT);
#endif
#ifdef ETI_PATH
        hoi_drv_eti(vrb.dst_ip, 0, vrb.remote.address, 0, hdoipd.local.aud_port);
#endif
        hdoipd_clr_rsc(RSC_AUDIO_OUT|RSC_AUDIO_SYNC);
        hdoipd_set_vtb_state(VTB_AUD_IDLE);
    }

    hoi_drv_set_led_status(SDI_OUT_OFF);
}

int vrb_audio_ext_pause(t_rtsp_media* media, void* UNUSED m, t_rtsp_connection* rsp)
{
    vrb_audio_pause(media);

    return RTSP_SUCCESS;
}

int vrb_audio_update(t_rtsp_media *media, t_rtsp_req_update *m, t_rtsp_connection UNUSED *rsp)
{
    switch (m->event) {

        case EVENT_TICK:
            // reset timeout
            vrb.timeout = 0;
        break;

        case EVENT_AUDIO_IN0_ON:
            // No multicast for now...simply stop before starting new
            if (!rtsp_media_splaying(media)) {
                vrb_audio_pause(media);
                // restart
                rtsp_client_set_play(media->creator);
                return RTSP_PAUSE;
            }
        break;
        case EVENT_AUDIO_IN0_OFF:
            vrb_audio_pause(media);
            osd_printf("vtb.audio stopped streaming - no audio input\n");
            report(ERROR "vtb.audio stopped streaming - no audio input");
            return RTSP_PAUSE;
        break;
    }

    return RTSP_SUCCESS;
}

int vrb_audio_ready(t_rtsp_media UNUSED *media)
{
    //if (hdoipd_tstate(VTB_AUD_MASK)) {
    //    // not ready ->
    //    return RTSP_ERRORNO;
    //}
    if (!hdoipd_rsc(RSC_VIDEO_SINK)) {
        // no hdmi sink
        return RTSP_ERRORNO;
    }
    return RTSP_SUCCESS;
}

int vrb_audio_dosetup(t_rtsp_media *media, void* UNUSED m, void* UNUSED rsp)
{
    int port;
    t_rtsp_transport transport;
    t_rtsp_client *client = media->creator;
    t_rtsp_hdcp hdcp;

    hdcp.hdcp_on = reg_test("hdcp-force", "true");

    report(VRB_METHOD "vrb_audio_dosetup");

    if (!client) return RTSP_NULL_POINTER;

    rtsp_default_transport(&transport); // TODO: daemon transport configuration
    port = reg_get_int("audio-port");
    transport.client_port = PORT_RANGE(htons(port), htons(port+1));

    return rtsp_client_setup(client, &transport, 0, &hdcp);
}

int vrb_audio_doplay(t_rtsp_media *media, void* UNUSED m, void* UNUSED rsp)
{
    t_rtsp_client *client = media->creator;

    report(VRB_METHOD "vrb_audio_doplay");

    if (!client) return RTSP_NULL_POINTER;

    return rtsp_client_play(client, 0);
}

int vrb_audio_event(t_rtsp_media *media, uint32_t event)
{
    t_rtsp_client *client = media->creator;

    switch (event) {
        case EVENT_TICK:
            if (vrb.alive_ping) {
                vrb.alive_ping--;
            } else {
                vrb.alive_ping = TICK_SEND_ALIVE;
                // send tick we are alive (until something like rtcp is used)
         //       if (hdoipd_tstate(VTB_AUDIO)) { // only if audio stream = active
                rtsp_client_update(client, EVENT_TICK);
         //       }
            }
            if (vrb.timeout <= TICK_TIMEOUT) {
                vrb.timeout++;
            } else {
                report(INFO "vrb_audio_event: timeout");
                // timeout -> kill connection
                vrb.timeout = 0;
                rtsp_client_set_kill(client);
                osd_printf("vrb.audio connection lost...\n");
            }
        break;

        case EVENT_VIDEO_SINK_OFF:
            // Note: after teardown call media/client is not anymore a valid struct
            rtsp_client_set_teardown(client);
            client = 0;
        break;
    }

    return RTSP_SUCCESS;
}

t_rtsp_media vrb_audio = {
    .name = "audio",
    .owner = 0,
    .cookie = 0,
    .setup = (frtspm*)vrb_audio_setup,
    .play = (frtspm*)vrb_audio_play,
    .pause = (frtspm*)vrb_audio_ext_pause,
    .teardown = (frtspm*)vrb_audio_teardown,
    .error = (frtspm*)vrb_audio_error,
    .update = (frtspm*)vrb_audio_update,

    .ready = (frtspl*)vrb_audio_ready,
    .dosetup = (frtspm*)vrb_audio_dosetup,
    .doplay = (frtspm*)vrb_audio_doplay,
    .event = (frtspe*)vrb_audio_event
};

