/*
 * vtb_video.c
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */

#include "hdoipd.h"
#include "vtb_video.h"

struct {
    t_hdoip_eth         remote;
} vtb;


int vtb_video_setup(t_rtsp_media* media, t_rtsp_req_setup* m, t_rtsp_connection* rsp)
{
    int n;

    report("vtb_video_setup");

    // test if resource is available
    if ((hdoipd.vtb_state != VTB_OFF) && (hdoipd.vtb_state != VTB_IDLE)) {
        // currently no video input active
        rtsp_err_busy(rsp);
        return RTSP_RESOURCE_ERROR;
    }

    if (!(hdoipd.rsc_state & RSC_VIDEO_IN)) {
        // currently no video input active
        rtsp_err_no_source(rsp);
        return RTSP_RESOURCE_ERROR;
    }

    // reserve resource
    hdoipd.vtb_state = VTB_VIDEO;

    //
    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.mac)) != RTSP_SUCCESS) {
        rtsp_err_server(rsp);
        return n;
    }

    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RTSP_SUCCESS) {
        rtsp_err_server(rsp);
        return n;
    }

    if ((n = net_get_remote_hwaddr(hdoipd.listener.sockfd, "eth0", rsp->address, &vtb.remote.mac)) != RTSP_SUCCESS) {
        rtsp_err_retry(rsp);
        return n;
    }

    vtb.remote.address = rsp->address;
    vtb.remote.vid_port = PORT_RANGE_START(m->transport.client_port);
    m->transport.server_port = PORT_RANGE(hdoipd.local.vid_port, hdoipd.local.vid_port);
    rtsp_response_setup(rsp, &m->transport, media->sessionid);

    struct in_addr a1, a2; a1.s_addr = hdoipd.local.address; a2.s_addr = vtb.remote.address;
    printf("TX: %s:%d %s",
            inet_ntoa(a1), ntohs(hdoipd.local.vid_port), mac_ntoa(hdoipd.local.mac));
    printf(" -> %s:%d %s\n",
            inet_ntoa(a2), ntohs(vtb.remote.vid_port), mac_ntoa(vtb.remote.mac));

    return RTSP_SUCCESS;
}

int vtb_video_play(t_rtsp_media* media, t_rtsp_req_play* m, t_rtsp_connection* rsp)
{
    int n;
    t_video_timing timing;
    t_rtsp_rtp_format fmt;
    hdoip_eth_params eth;

    report("vtb_video_play");

    // start ethernet output
    eth.ipv4_dst_ip = vtb.remote.address;
    eth.ipv4_src_ip = hdoipd.local.address;
    eth.ipv4_tos = 0;
    eth.ipv4_ttl = 4;
    eth.packet_size = 1500;
    eth.rtp_ssrc = 0;
    for(n=0;n<6;n++) eth.src_mac[n] = hdoipd.local.mac[n];
    for(n=0;n<6;n++) eth.dst_mac[n] = vtb.remote.mac[n];
    eth.udp_dst_port = vtb.remote.vid_port;
    eth.udp_src_port = hdoipd.local.vid_port;

    // start vsi
    fmt.compress = 1;

    if ((n = hoi_drv_vsi(hdoipd.drv, true, 10000000, &eth, &timing, &fmt.value))) {
        rtsp_err_server(rsp);
        return RTSP_SERVER_ERROR;
    }

    hoi_drv_get_mtime(hdoipd.drv, &fmt.rtptime);

    // send timing
    rtsp_response_play(rsp, media->sessionid, &fmt, &timing);

    return RTSP_SUCCESS;
}

int vtb_video_teardown(t_rtsp_media* media, t_rtsp_req_teardown* m, t_rtsp_connection* rsp)
{
    // get daemon struct

    report("vtb_video_play");
}

node_anchor(vtb_video_sessions);
t_rtsp_media vtb_video = {
    .owner = 0,
    .cookie = 0,
    .session = &vtb_video_sessions,
    .setup = vtb_video_setup,
    .play = vtb_video_play,
    .teardown = vtb_video_teardown
};
