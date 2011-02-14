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

#define PROCESSING_DELAY_CORRECTION (6000)

struct {
    t_hdoip_eth         remote;
} vrb;

int vrb_audio_setup(t_rscp_media UNUSED *media, t_rscp_rsp_setup* m, t_rscp_connection* rsp)
{
    int n;
    report("vrb_audio_setup");

    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac)) != RSCP_SUCCESS) {
        return n;
    }

    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RSCP_SUCCESS) {
        return n;
    }

    vrb.remote.address = rsp->address;
    vrb.remote.aud_port = PORT_RANGE_START(m->transport.server_port);
    hdoipd.local.aud_port = PORT_RANGE_START(m->transport.client_port);

    REPORT_RTX("RX", hdoipd.local, "<-", vrb.remote, aud);

#ifdef ETI_PATH
    // TODO: separete Audio/Video
    hoi_drv_eti(hdoipd.local.address, vrb.remote.address,
            0, hdoipd.local.aud_port);
#endif

    hdoipd_set_tstate(VTB_AUD_IDLE);

    return RSCP_SUCCESS;
}


int vrb_audio_play(t_rscp_media UNUSED *media, t_rscp_rsp_play* m, t_rscp_connection UNUSED *rsp)
{
    uint32_t compress = 0;
    report("vrb_audio_play");

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
    hoi_drv_aso(m->format.value, 1000, m->format.compress&0xff, 2, channel_select, 20);
#endif

    hdoipd_set_tstate(VTB_AUDIO);
    hdoipd_set_rsc(RSC_AUDIO_OUT);

    struct in_addr a1; a1.s_addr = vrb.remote.address;
    osd_printf("Streaming Audio from %s\n", inet_ntoa(a1));

    return RSCP_SUCCESS;
}


int vrb_audio_teardown(t_rscp_media UNUSED *media, t_rscp_rsp_teardown UNUSED *m, t_rscp_connection UNUSED *rsp)
{
    report("vrb_audio_teardown");

    if (hdoipd_tstate(VTB_AUD_MASK)) {
#ifdef AUD_OUT_PATH
        hoi_drv_reset(DRV_RST_AUD_OUT);
#endif
    }

    hdoipd_clr_rsc(RSC_AUDIO_OUT|RSC_AUDIO_SYNC);
    hdoipd_set_tstate(VTB_AUD_OFF);

    return RSCP_SUCCESS;
}

int vrb_audio_teardown_q(t_rscp_media UNUSED *media, t_rscp_req_teardown UNUSED *m, t_rscp_connection UNUSED *rsp)
{
    report("vrb_audio_teardown_q");

    if (hdoipd_tstate(VTB_AUD_MASK)) {
#ifdef AUD_OUT_PATH
        hoi_drv_reset(DRV_RST_AUD_OUT);
#endif
    }

    hdoipd_clr_rsc(RSC_AUDIO_OUT|RSC_AUDIO_SYNC);
    hdoipd_set_tstate(VTB_AUD_OFF);

    // activate hello listener
    char *s = reg_get("remote-uri");
    box_sys_set_remote(s);

    osd_printf("audio remote off...");

    return RSCP_SUCCESS;
}

int vrb_audio_error(t_rscp_media UNUSED *media, intptr_t m, t_rscp_connection* rsp)
{
    report(" ? client failed (%d): %d - %s", m, rsp->ecode, rsp->ereason);
    osd_printf("Audio Streaming could not be established: %d - %s\n", rsp->ecode, rsp->ereason);
    return RSCP_SUCCESS;
}


void vrb_audio_pause(t_rscp_media *media)
{
    report("vrb_audio_pause");

    if (hdoipd_tstate(VTB_AUDIO)) {
#ifdef AUD_OUT_PATH
        // reset input
        hoi_drv_reset(DRV_RST_AUD_OUT);
#endif
#ifdef ETI_PATH
        // restart Ethernet input
        hoi_drv_eti(hdoipd.local.address, vrb.remote.address,
                0, hdoipd.local.aud_port);
#endif
        hdoipd_clr_rsc(RSC_AUDIO_OUT|RSC_AUDIO_SYNC);
        hdoipd_set_tstate(VTB_AUD_IDLE);
    }

    // goto ready without further communication
    rscp_media_force_ready(media);
}


int vrb_audio_update(t_rscp_media *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
{
    report("vrb_audio_update(%x)", m->event);

    switch (m->event) {

        case EVENT_AUDIO_IN0_ON:
            // No multicast for now...simply stop before starting new
            if (rscp_media_splaying(media)) {
                vrb_audio_pause(media);
            }

            // restart
            unlock("vrb_audio_update");
                hdoipd_launch(hdoipd_start_vrb, media, 250, 3, 1000);
            lock("vrb_audio_update");
        break;

        case EVENT_AUDIO_IN0_OFF:
            vrb_audio_pause(media);
            osd_permanent(true);
            osd_printf("vtb.audio stopped streaming...");
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
    return RSCP_SUCCESS;
}

int vrb_audio_dosetup(t_rscp_media *media)
{
    int port;
    t_rscp_transport transport;
    t_rscp_client *client = media->creator;

    if (!client) return RSCP_NULL_POINTER;

    rscp_default_transport(&transport); // TODO: daemon transport configuration
    port = reg_get_int("audio-port");
    transport.client_port = PORT_RANGE(htons(port), htons(port+1));

    return rscp_client_setup(client, &transport, 0);
}

int vrb_audio_doplay(t_rscp_media *media)
{
    t_rscp_client *client = media->creator;

    if (!client) return RSCP_NULL_POINTER;

    return rscp_client_play(client, 0);
}

int vrb_audio_event(t_rscp_media *media, uint32_t event)
{
    t_rscp_client *client = media->creator;

    switch (event) {
        case EVENT_VIDEO_SINK_OFF:
            rscp_client_teardown(client);
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
    .teardown_r = (frscpm*)vrb_audio_teardown,
    .teardown_q = (frscpm*)vrb_audio_teardown_q,
    .error = (frscpm*)vrb_audio_error,
    .update = (frscpm*)vrb_audio_update,

    .ready = (frscpl*)vrb_audio_ready,
    .dosetup = (frscpl*)vrb_audio_dosetup,
    .doplay = (frscpl*)vrb_audio_doplay,
    .event = (frscpe*)vrb_audio_event
};

