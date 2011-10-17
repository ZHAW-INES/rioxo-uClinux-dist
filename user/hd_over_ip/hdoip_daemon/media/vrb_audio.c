/*
 * vrb_audio.c
 *
 *  Created on: 07.01.2011
 *      Author: alda
 */

#include "hoi_drv_user.h"
#include "hdoipd.h"
#include "box_sys.h"
#include "vrb_audio.h"
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

int vrb_audio_setup(t_rscp_media *media, t_rscp_rsp_setup* m, t_rscp_connection* rsp)
{
    int n;
    t_rscp_client* client = media->creator;
    report(VRB_METHOD "vrb_audio_setup");

    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac)) != RSCP_SUCCESS) {
        return n;
    }

    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RSCP_SUCCESS) {
        return n;
    }

    vrb.remote.address = client->con.address;
    vrb.remote.aud_port = PORT_RANGE_START(m->transport.server_port);
    hdoipd.local.aud_port = PORT_RANGE_START(m->transport.client_port);

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
    if ((m->hdcp.hdcp_on == 1) && (hdoipd.drivers | DRV_GS2972)) {
        report(INFO "encrypted audio output on SDI is not allowed");
        osd_permanent(true);
        osd_printf("encrypted audio output on SDI is not allowed\n");
        rscp_client_set_teardown(client);
        hdoipd_set_task_start_vrb();
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
    // TODO: separete Audio/Video
    hoi_drv_eti(vrb.dst_ip, 0, vrb.remote.address, 0, hdoipd.local.aud_port);
#endif

    hdoipd_set_vtb_state(VTB_AUD_IDLE);

    media->result = RSCP_RESULT_READY;
    vrb.alive_ping = 1;
    vrb.timeout = 0;

    return RSCP_SUCCESS;
}


int vrb_audio_play(t_rscp_media *media, t_rscp_rsp_play* m, t_rscp_connection UNUSED *rsp)
{
    uint32_t compress = 0;
    report(VRB_METHOD "vrb_audio_play");

    //Test if HDCP parameters were set correctly
	if (hdoipd.hdcp.enc_state && !(get_hdcp_status() & HDCP_ETI_AUDIO_EN)){
		if (hdoipd.hdcp.ske_executed){
			hoi_drv_hdcp(hdoipd.hdcp.keys); 	/* write keys to kernel */
			report(INFO "Audio encryption enabled (eti)!");
			hoi_drv_hdcp_auden_eti();
		}
		else {
			report(INFO "No valid HDCP ske executed!");
			return RSCP_ERRORNO;
		}
	}

    media->result = RSCP_RESULT_PLAYING;

    // set slave timer when not already synced
    if (!hdoipd_rsc(RSC_SYNC)) {
        hoi_drv_set_stime(m->format.rtptime+PROCESSING_DELAY_CORRECTION);
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
    hoi_drv_aso(m->format.value, 1000, m->format.compress, m->format.value2, channel_select, 20, compress);
    report(INFO "\naudio streaming started(fs = %d Hz, bitwidth = %d Bit, channels = %d)", m->format.value, m->format.compress, m->format.value2);
#endif

    hdoipd_set_vtb_state(VTB_AUDIO);
    hdoipd_set_rsc(RSC_AUDIO_OUT);

    struct in_addr a1; a1.s_addr = vrb.remote.address;
    osd_printf("Streaming Audio from %s\n", inet_ntoa(a1));

    hoi_drv_set_led_status(SDI_OUT_WITH_AUDIO);

    return RSCP_SUCCESS;
}


int vrb_audio_teardown(t_rscp_media *media, t_rscp_req_teardown UNUSED *m, t_rscp_connection *rsp)
{
    report(VRB_METHOD "vrb_audio_teardown");

    media->result = RSCP_RESULT_TEARDOWN;

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

    if (rsp) {
        osd_printf("audio remote off...\n");
    }

    vrb.alive_ping = 1;
    vrb.timeout = 0;

    hoi_drv_set_led_status(SDI_OUT_OFF);

    return RSCP_SUCCESS;
}

int vrb_audio_error(t_rscp_media *media, intptr_t m, t_rscp_connection* rsp)
{
	t_rscp_client *client = media->creator;

    if(rsp) {
        report(" ? client failed (%d): %d - %s", m, rsp->ecode, rsp->ereason);
        osd_printf("vrb.audio streaming could not be established: %d - %s\n", rsp->ecode, rsp->ereason);
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
            			hdoipd_set_task_start_vrb();
                        break;
            case 400:
            default:    media->result = RSCP_RESULT_SERVER_ERROR;
                        break;
        }
    } else {
        osd_printf("vrb.audio streaming could not be established: connection refused\n");
        media->result = RSCP_RESULT_CONNECTION_REFUSED;
    }
    return RSCP_SUCCESS;
}


void vrb_audio_pause(t_rscp_media *media)
{
	report(INFO "vrb_audio_pause");
    media->result = RSCP_RESULT_PAUSE_Q;
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


int vrb_audio_update(t_rscp_media *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
{
	t_rscp_client *client = media->creator;
    switch (m->event) {

        case EVENT_TICK:
            // reset timeout
            vrb.timeout = 0;
        break;

        case EVENT_AUDIO_IN0_ON:
            // No multicast for now...simply stop before starting new
            if (!rscp_media_splaying(media)) {
                vrb_audio_pause(media);
                // restart
                rscp_client_set_play(media->creator);
                return RSCP_PAUSE;
            }
        break;
        case EVENT_AUDIO_IN0_OFF:
            vrb_audio_pause(media);
            osd_printf("vtb.audio stopped streaming - no audio input\n");
            report(ERROR "vtb.audio stopped streaming - no audio input");
            return RSCP_PAUSE;
        break;
    }

    return RSCP_SUCCESS;
}

int vrb_audio_ready(t_rscp_media UNUSED *media)
{
    if (hdoipd_tstate(VTB_AUD_MASK)) {
        // not ready ->
        return RSCP_ERRORNO;
    }
    if (!hdoipd_rsc(RSC_VIDEO_SINK)) {
        // no hdmi sink
        return RSCP_ERRORNO;
    }
    return RSCP_SUCCESS;
}

int vrb_audio_dosetup(t_rscp_media *media)
{
    int port;
    t_rscp_transport transport;
    t_rscp_client *client = media->creator;
    t_rscp_hdcp hdcp;

    hdcp.hdcp_on = reg_test("hdcp-force", "true");

    report(VRB_METHOD "vrb_audio_dosetup");

    if (!client) return RSCP_NULL_POINTER;

    rscp_default_transport(&transport); // TODO: daemon transport configuration
    port = reg_get_int("audio-port");
    transport.client_port = PORT_RANGE(htons(port), htons(port+1));

    return rscp_client_setup(client, &transport, 0, &hdcp);
}

int vrb_audio_doplay(t_rscp_media *media)
{
    t_rscp_client *client = media->creator;

    report(VRB_METHOD "vrb_audio_doplay");

    if (!client) return RSCP_NULL_POINTER;

    return rscp_client_play(client, 0);
}

int vrb_audio_event(t_rscp_media *media, uint32_t event)
{
    t_rscp_client *client = media->creator;

    switch (event) {
        case EVENT_TICK:
            if (vrb.alive_ping) {
                vrb.alive_ping--;
            } else {
                vrb.alive_ping = TICK_SEND_ALIVE;
                // send tick we are alive (until something like rtcp is used)
                rscp_client_update(client, EVENT_TICK);
            }
            if (vrb.timeout <= TICK_TIMEOUT) {
                vrb.timeout++;
            } else {
                report(INFO "vrb_audio_event: timeout");
                // timeout -> kill connection
                vrb.timeout = 0;
                rscp_client_set_kill(client);
                osd_printf("vrb.audio connection lost...\n");
            }
        break;

        case EVENT_VIDEO_SINK_OFF:
            // Note: after teardown call media/client is not anymore a valid struct
            rscp_client_set_teardown(client);
            client = 0;
        break;
    }

    return RSCP_SUCCESS;
}

t_rscp_media vrb_audio = {
    .name = "audio",
    .owner = 0,
    .cookie = 0,
    .setup = (frscpm*)vrb_audio_setup,
    .play = (frscpm*)vrb_audio_play,
    .teardown = (frscpm*)vrb_audio_teardown,
    .error = (frscpm*)vrb_audio_error,
    .update = (frscpm*)vrb_audio_update,

    .ready = (frscpl*)vrb_audio_ready,
    .dosetup = (frscpl*)vrb_audio_dosetup,
    .doplay = (frscpl*)vrb_audio_doplay,
    .event = (frscpe*)vrb_audio_event
};

