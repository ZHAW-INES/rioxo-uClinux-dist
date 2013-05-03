/*
 * vrb_audio_emb.c
 *
 *  Created on: 07.01.2011
 *      Author: alda
 */

#include "hoi_drv_user.h"
#include "hdoipd.h"
#include "box_sys.h"
#include "vrb_audio_emb.h"
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

int vrb_audio_emb_setup(t_rscp_media *media, t_rscp_rsp_setup* m, t_rscp_connection* rsp)
{
    int n;
    t_rscp_client* client = media->creator;
    report(VRB_METHOD "vrb_audio_emb_setup");

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
    hoi_drv_eti(vrb.dst_ip, 0, vrb.remote.address, 0, hdoipd.local.aud_port, 0);
#endif

    hdoipd_set_vtb_state(VTB_AUD_EMB_IDLE);

    media->result = RSCP_RESULT_READY;
    vrb.alive_ping = 1;
    vrb.timeout = 0;

    return RSCP_SUCCESS;
}


int vrb_audio_emb_play(t_rscp_media *media, t_rscp_rsp_play* m, t_rscp_connection UNUSED *rsp)
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
			return RSCP_ERRORNO;
		}
	}

    media->result = RSCP_RESULT_PLAYING;

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

    return RSCP_SUCCESS;
}

int vrb_audio_emb_teardown(t_rscp_media *media, t_rscp_req_teardown UNUSED *m, t_rscp_connection *rsp)
{
    report(VRB_METHOD "vrb_audio_emb_teardown");

    media->result = RSCP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_AUDIO_EMB|VTB_AUD_EMB_IDLE)) {
#ifdef AUD_OUT_PATH
        hdoipd_hw_reset(DRV_RST_AUD_EMB_OUT);
#endif
        hdoipd_clr_rsc(RSC_AUDIO_EMB_OUT|RSC_AUDIO_EMB_SYNC);
        hdoipd_set_vtb_state(VTB_AUD_EMB_OFF);
    }

    if (vrb.multicast_en) {
        leave_multicast_group(vrb.dst_ip);
    }

    osd_printf("vrb-audio embedded connection lost...\n");

    vrb.alive_ping = 1;
    vrb.timeout = 0;

    hoi_drv_set_led_status(SDI_OUT_OFF);

    return RSCP_SUCCESS;
}

int vrb_audio_emb_error(t_rscp_media *media, intptr_t m, t_rscp_connection* rsp)
{
	t_rscp_client *client = media->creator;

    if(rsp) {
        report(" ? client failed (%d): %d - %s", m, rsp->ecode, rsp->ereason);
        osd_printf("vrb-audio (embedded) streaming could not be established: %d - %s\n", rsp->ecode, rsp->ereason);
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
            case 400:
            default:    media->result = RSCP_RESULT_SERVER_ERROR;
                        break;
        }
    } else {
        osd_printf("vrb-audio (embedded) streaming could not be established: connection refused\n");
        media->result = RSCP_RESULT_CONNECTION_REFUSED;
    }
    return RSCP_SUCCESS;
}


void vrb_audio_emb_pause(t_rscp_media *media)
{
	report(INFO "vrb_audio_emb_pause");
    media->result = RSCP_RESULT_PAUSE_Q;
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

int vrb_audio_emb_ext_pause(t_rscp_media* media, void* UNUSED m, t_rscp_connection* rsp)
{
    vrb_audio_emb_pause(media);

    return RSCP_SUCCESS;
}

int vrb_audio_emb_update(t_rscp_media *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
{
    switch (m->event) {

        case EVENT_TICK:
            // reset timeout
            vrb.timeout = 0;
        break;

        case EVENT_AUDIO_IN0_ON:
            // No multicast for now...simply stop before starting new
            if (!rscp_media_splaying(media)) {
                vrb_audio_emb_pause(media);
                // restart
                rscp_client_set_play(media->creator);
                return RSCP_PAUSE;
            }
        break;
        case EVENT_AUDIO_IN0_OFF:
            vrb_audio_emb_pause(media);
            osd_printf("vtb.audio (embedded) stopped streaming - no audio input\n");
            report(ERROR "vtb.audio (embedded) stopped streaming - no audio input");
            return RSCP_PAUSE;
        break;
    }

    return RSCP_SUCCESS;
}

int vrb_audio_emb_ready(t_rscp_media UNUSED *media)
{
    //if (hdoipd_tstate(VTB_AUD_MASK)) {
    //    // not ready ->
    //    return RSCP_ERRORNO;
    //}
    if (!hdoipd_rsc(RSC_VIDEO_SINK)) {
        //no video sink
        return RSCP_ERRORNO;
    }
    return RSCP_SUCCESS;
}

int vrb_audio_emb_dosetup(t_rscp_media *media, void* UNUSED m, void* UNUSED rsp)
{
    int port;
    t_rscp_transport transport;
    t_rscp_client *client = media->creator;
    t_rscp_hdcp hdcp;

    hdcp.hdcp_on = reg_test("hdcp-force", "true");

    report(VRB_METHOD "vrb_audio_emb_dosetup");

    if (!client) return RSCP_NULL_POINTER;

    rscp_default_transport(&transport); // TODO: daemon transport configuration
    port = reg_get_int("audio-port");
    transport.client_port = PORT_RANGE(htons(port), htons(port+1));

    return rscp_client_setup(client, &transport, 0, &hdcp);
}

int vrb_audio_emb_doplay(t_rscp_media *media, void* UNUSED m, void* UNUSED rsp)
{
    t_rscp_rtp_format fmt;
    t_rscp_client *client = media->creator;

    report(VRB_METHOD "vrb_audio_emb_doplay");

    if (!client) return RSCP_NULL_POINTER;

    memset(&fmt, 0, sizeof(fmt));
    hoi_drv_get_mtime(&fmt.rtptime);

    return rscp_client_play(client, &fmt);
}

int vrb_audio_emb_event(t_rscp_media *media, uint32_t event)
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
                report(INFO "vrb_audio_event: timeout audio embedded");
                // timeout -> kill connection
                vrb.timeout = 0;
                rscp_client_set_kill(client);
                osd_printf("vrb-audio connection lost...\n");
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

t_rscp_media vrb_audio_emb = {
    .name = "audio_emb",
    .owner = 0,
    .cookie = 0,
    .setup = (frscpm*)vrb_audio_emb_setup,
    .play = (frscpm*)vrb_audio_emb_play,
    .pause = (frscpm*)vrb_audio_emb_ext_pause,
    .teardown = (frscpm*)vrb_audio_emb_teardown,
    .error = (frscpm*)vrb_audio_emb_error,
    .update = (frscpm*)vrb_audio_emb_update,

    .ready = (frscpl*)vrb_audio_emb_ready,
    .dosetup = (frscpm*)vrb_audio_emb_dosetup,
    .doplay = (frscpm*)vrb_audio_emb_doplay,
    .event = (frscpe*)vrb_audio_emb_event
};
