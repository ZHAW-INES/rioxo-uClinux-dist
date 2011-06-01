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
#include "multicast.h"

#define TICK_TIMEOUT                    (hdoipd.eth_timeout)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)


int vtb_audio_setup(t_rscp_media* media, t_rscp_req_setup* m, t_rscp_connection* rsp)
{
    int n;
    t_multicast_cookie* cookie = media->cookie;

    report(INFO "vtb_audio_setup");

    if (!hdoipd_state(HOID_VTB)) {
        report(" ? not in state vtb");
        rscp_err_no_vtb(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // test if resource is available
    if(!get_multicast_enable() || (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_NOT_AVAILABLE)) {
        if (hdoipd_tstate(VTB_AUD_MASK)) {
            report(" ? vtb busy");
            rscp_err_busy(rsp);
            return RSCP_REQUEST_ERROR;
        }
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
    if ((n = net_get_remote_hwaddr(hdoipd.listener.sockfd, "eth0", rsp->address, (uint8_t*)&cookie->remote.mac)) != RSCP_SUCCESS) {
        report(" ? net_get_remote_hwaddr failed");
        rscp_err_retry(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // reserve resource
    if (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_NOT_AVAILABLE) {
        hdoipd_set_vtb_state(VTB_AUD_IDLE);
    }

    cookie->remote.address = rsp->address;
    cookie->remote.aud_port = PORT_RANGE_START(m->transport.client_port);
    m->transport.server_port = PORT_RANGE(hdoipd.local.aud_port, hdoipd.local.aud_port);
    m->transport.multicast = set_multicast_enable(reg_test("multicast_en", "true"));

    rscp_response_setup(rsp, &m->transport, media->sessionid);

    REPORT_RTX("TX", hdoipd.local, "->", cookie->remote, aud);

    media->result = RSCP_RESULT_READY;

    cookie->timeout = 0;
    cookie->alive_ping = 1;

    return RSCP_SUCCESS;
}

int vtb_audio_play(t_rscp_media* media, t_rscp_req_play UNUSED *m, t_rscp_connection* rsp)
{
    int n;
    t_rscp_rtp_format fmt;
    hdoip_eth_params eth;
    char dst_mac[6];

    t_multicast_cookie* cookie = media->cookie;

    media->result = RSCP_RESULT_PLAYING;

    report(INFO "vtb_audio_play");

    if (!hdoipd_state(HOID_VTB)) {
        // we don't have the resource reserved
        report(" ? require state VTB");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    if(!get_multicast_enable() || (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_NOT_AVAILABLE)) {
        if (!hdoipd_tstate(VTB_AUD_IDLE)) {
            // we don't have the resource reserved
            report(" ? require state VTB_AUD_IDLE");
            rscp_err_server(rsp);
            return RSCP_REQUEST_ERROR;
        }
    }

    if (!hdoipd_rsc(RSC_AUDIO_IN)) {
        // currently no audio input active
        report(" ? require active audio input");
        rscp_err_no_source(rsp);
        hdoipd_set_vtb_state(VTB_AUD_OFF);
        return RSCP_REQUEST_ERROR;
    }

    // start ethernet output
    if(get_multicast_enable()) {
        eth.ipv4_dst_ip = inet_addr(reg_get("multicast_group"));
        convert_ip_to_multicast_mac(inet_addr(reg_get("multicast_group")), dst_mac);
    }
    else {
        eth.ipv4_dst_ip = cookie->remote.address;
        for(n=0;n<6;n++) dst_mac[n] = cookie->remote.mac[n];
    }

    eth.ipv4_src_ip = hdoipd.local.address;
    eth.ipv4_tos = 0;
    eth.ipv4_ttl = 30;
    eth.packet_size = 512;
    eth.rtp_ssrc = 0;
    for(n=0;n<6;n++) eth.src_mac[n] = hdoipd.local.mac[n];
    for(n=0;n<6;n++) eth.dst_mac[n] = cookie->remote.mac[n];
    eth.udp_dst_port = cookie->remote.aud_port;
    eth.udp_src_port = hdoipd.local.aud_port;

    // asi parameters (TODO: restrict audio channel based on registry)
    t_hoi_msg_info *nfo;
    hoi_drv_info_all(&nfo);
    fmt.compress = nfo->audio_width[0];
    fmt.value = nfo->audio_fs[0];
    fmt.value2 = nfo->audio_cnt[0];
    hoi_drv_get_mtime(&fmt.rtptime);

    if ( ((nfo->audio_width[0]<8) || (nfo->audio_width[0]>32)) ||
         ((nfo->audio_fs[0]<32000) || (nfo->audio_fs[0]>96000)) ||
         ((nfo->audio_cnt[0]<1) || (nfo->audio_cnt[0]>8)) ) {
        rscp_err_def_source(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // send timing
    rscp_response_play(rsp, media->sessionid, &fmt, 0);

#ifdef AUD_IN_PATH
    if (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_NOT_AVAILABLE) {
        // activate asi
        uint8_t channel_select[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        hoi_drv_asi(0, &eth, nfo->audio_fs[0], nfo->audio_width[0], nfo->audio_cnt[0], channel_select);
        report(INFO "audio streaming started(fs = %d Hz, bitwidth = %d Bit)", nfo->audio_fs[0], nfo->audio_width[0]);
#endif
        // We are streaming Audio now...
        hdoipd_set_vtb_state(VTB_AUDIO);
    }

    add_client_to_vtb(MEDIA_IS_AUDIO, cookie->remote.address);

    return RSCP_SUCCESS;
}

int vtb_audio_teardown(t_rscp_media* media, t_rscp_req_teardown *m, t_rscp_connection* rsp)
{
    t_multicast_cookie* cookie = media->cookie;

    report(INFO "vtb_audio_teardown");

    media->result = RSCP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_AUD_MASK)) {
#ifdef AUD_IN_PATH
        if (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_AVAILABLE_ONLY_ONE) {
            hdoipd_hw_reset(DRV_RST_AUD_IN);
#endif
            hdoipd_set_vtb_state(VTB_AUD_OFF);
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

void vtb_audio_pause(t_rscp_media *media)
{
    t_multicast_cookie* cookie = media->cookie;

    report(INFO "vtb_audio_pause");

    media->result = RSCP_RESULT_PAUSE_Q;

    if (hdoipd_tstate(VTB_AUDIO)) {
#ifdef AUD_IN_PATH
        if (check_client_availability(MEDIA_IS_AUDIO) == CLIENT_AVAILABLE_ONLY_ONE) {
            hdoipd_hw_reset(DRV_RST_AUD_IN);
#endif
            hdoipd_set_vtb_state(VTB_AUD_IDLE);
        }
    }

    remove_client_from_vtb(MEDIA_IS_AUDIO, cookie->remote.address);
}


int vtb_audio_update(t_rscp_media *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
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


int vtb_audio_event(t_rscp_media *media, uint32_t event)
{
    t_multicast_cookie* cookie = media->cookie;

    switch (event) {
        case EVENT_AUDIO_IN0_ON:
            if (rscp_media_splaying(media)) {
                vtb_audio_pause(media);
                rscp_server_update(media, EVENT_AUDIO_IN0_OFF);
            }
            rscp_server_update(media, EVENT_AUDIO_IN0_ON);
            return RSCP_PAUSE;
        break;
        case EVENT_AUDIO_IN0_OFF:
            if (rscp_media_splaying(media)) {
                vtb_audio_pause(media);
                rscp_server_update(media, EVENT_AUDIO_IN0_OFF);
            }
            return RSCP_PAUSE;
        break;
        case EVENT_TICK:
            if (cookie->alive_ping) {
                cookie->alive_ping--;
            } else {
                cookie->alive_ping = TICK_SEND_ALIVE;
                // send tick we are alive (until something like rtcp is used)
                rscp_server_update(media, EVENT_TICK);
            }

            if (cookie->timeout <= TICK_TIMEOUT) {
                cookie->timeout++;
            } else {
                report(INFO "vtb_audio_event: timeout");
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


t_rscp_media vtb_audio = {
    .name = "audio",
    .owner = 0,
    .cookie = 0,
    .cookie_size = sizeof(t_multicast_cookie),
    .setup = (frscpm*)vtb_audio_setup,
    .play = (frscpm*)vtb_audio_play,
    .teardown = (frscpm*)vtb_audio_teardown,
    .update = (frscpm*)vtb_audio_update,
    .event = (frscpe*)vtb_audio_event
};
