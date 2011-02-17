/*
 * vtb_audio.c
 *
 *  Created on: 06.01.2011
 *      Author: alda
 */
#include "hdoipd.h"
#include "hoi_drv_user.h"
#include "vtb_audio.h"
#include "hdoipd_fsm.h"
#include "box_sys.h"
#include "rscp_server.h"

static struct {
    t_hdoip_eth         remote;
} vtb;


int vtb_audio_setup(t_rscp_media* media, t_rscp_req_setup* m, t_rscp_connection* rsp)
{
    int n;

    report("vtb_audio_setup");

    if (!hdoipd_state(HOID_VTB)) {
        // currently no video input active
        report(" ? not in state vtb");
        rscp_err_no_vtb(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // test if resource is available
    // currently no multicast support -> video must be UNUSED
    if (hdoipd_tstate(VTB_AUD_MASK)) {
        // currently busy -> no multicast supported yet
        report(" ? vtb busy");
        rscp_err_busy(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // get own MAC address
    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac)) != RSCP_SUCCESS) {
        report(" ? net_get_local_hwaddr failed");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // get own IP address
    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RSCP_SUCCESS) {
        report(" ? net_get_local_addr failed");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // get MAC address of next in rout
    if ((n = net_get_remote_hwaddr(hdoipd.listener.sockfd, "eth0", rsp->address, (uint8_t*)&vtb.remote.mac)) != RSCP_SUCCESS) {
        report(" ? net_get_remote_hwaddr failed");
        rscp_err_retry(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // reserve resource
    hdoipd_set_tstate(VTB_AUD_IDLE);

    vtb.remote.address = rsp->address;
    vtb.remote.vid_port = PORT_RANGE_START(m->transport.client_port);
    m->transport.server_port = PORT_RANGE(hdoipd.local.vid_port, hdoipd.local.vid_port);
    rscp_response_setup(rsp, &m->transport, media->sessionid);

    REPORT_RTX("TX", hdoipd.local, "->", vtb.remote, aud);

    return RSCP_SUCCESS;
}

int vtb_audio_play(t_rscp_media* media, t_rscp_req_play UNUSED *m, t_rscp_connection* rsp)
{
    int n;
    t_rscp_rtp_format fmt;
    hdoip_eth_params eth;

    report("vtb_audio_play");

    if (!hdoipd_state(HOID_VTB)) {
        // we don't have the resource reserved
        report(" ? require state VTB");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // no multicast currently -> must be idle
    if (!hdoipd_tstate(VTB_AUD_IDLE)) {
        // we don't have the resource reserved
        report(" ? require state VTB_AUD_IDLE");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    if (!hdoipd_rsc(RSC_AUDIO_IN)) {
        // currently no audio input active
        report(" ? require active audio input");
        rscp_err_no_source(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // start ethernet output
    eth.ipv4_dst_ip = vtb.remote.address;
    eth.ipv4_src_ip = hdoipd.local.address;
    eth.ipv4_tos = 0;
    eth.ipv4_ttl = 30;
    eth.packet_size = 512;
    eth.rtp_ssrc = 0;
    for(n=0;n<6;n++) eth.src_mac[n] = hdoipd.local.mac[n];
    for(n=0;n<6;n++) eth.dst_mac[n] = vtb.remote.mac[n];
    eth.udp_dst_port = vtb.remote.vid_port;
    eth.udp_src_port = hdoipd.local.vid_port;

    // asi parameters (TODO: restrict audio channel based on registry)
    t_hoi_msg_info *nfo;
    hoi_drv_info_all(&nfo);
    fmt.compress = nfo->audio_width[0];
    fmt.value = nfo->audio_fs[0];
    hoi_drv_get_mtime(&fmt.rtptime);

    // send timing
    rscp_response_play(rsp, media->sessionid, &fmt, 0);

#ifdef AUD_IN_PATH
    // activate asi
    uint8_t channel_select[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    hoi_drv_asi(0, &eth, nfo->audio_fs[0], nfo->audio_width[0], 2, channel_select);
    report(INFO "audio streaming started");
#endif

    // We are streaming Audio now...
    hdoipd_set_tstate(VTB_AUDIO);

    return RSCP_SUCCESS;
}

int vtb_audio_teardown(t_rscp_media* media, t_rscp_req_teardown UNUSED *m, t_rscp_connection* rsp)
{
    report("vtb_audio_teardown");

    if (hdoipd_tstate(VTB_AUD_MASK)) {
#ifdef AUD_IN_PATH
        hoi_drv_reset(DRV_RST_AUD_IN);
#endif
        hdoipd_set_tstate(VTB_AUD_OFF);
    }

    rscp_response_teardown(rsp, media->sessionid);

    return RSCP_SUCCESS;
}

void vtb_audio_pause(t_rscp_media *media)
{
    report("vtb_audio_pause");

    if (hdoipd_tstate(VTB_AUDIO)) {
#ifdef AUD_IN_PATH
        hoi_drv_reset(DRV_RST_AUD_IN);
#endif
        hdoipd_set_tstate(VTB_AUD_IDLE);
    }

    // goto ready without further communication
    rscp_media_force_ready(media);
}


int vtb_audio_event(t_rscp_media *media, uint32_t event)
{
    switch (event) {
        case EVENT_AUDIO_IN0_ON:
            if (rscp_media_splaying(media)) {
                vtb_audio_pause(media);
                rscp_server_update(media, EVENT_AUDIO_IN0_OFF);
            }
            rscp_server_update(media, EVENT_AUDIO_IN0_ON);
        break;
        case EVENT_AUDIO_IN0_OFF:
            if (rscp_media_splaying(media)) {
                vtb_audio_pause(media);
                rscp_server_update(media, EVENT_AUDIO_IN0_OFF);
            }
        break;
    }

    return RSCP_SUCCESS;
}


t_rscp_media vtb_audio = {
    .name = "audio",
    .owner = 0,
    .cookie = 0,
    .setup = (frscpm*)vtb_audio_setup,
    .play = (frscpm*)vtb_audio_play,
    .teardown_q = (frscpm*)vtb_audio_teardown,
    .event = (frscpe*)vtb_audio_event
};
