/*
 * vtb_video.c
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */

#include "hdoipd.h"
#include "vtb_video.h"

#define _mac0cpy(l, u, a) \
{ \
    l = ((uint32_t)a[0]) | ((uint32_t)a[1] << 8) | ((uint32_t)a[2] << 16) | ((uint32_t)a[3] << 24); \
    u = ((uint32_t)a[4]) | ((uint32_t)a[5] << 8); \
}

extern t_hdoipd handle;

struct {
    t_hdoip_eth         remote;
} vtb;


int vtb_video_setup(t_rtsp_media* media, t_rtsp_req_setup* m, t_rtsp_connection* rsp)
{
    int n;

    pthread_mutex_lock(&handle.mutex);

    // test if resource is available
    if ((handle.vtb_state != VTB_OFF) && (handle.vtb_state != VTB_IDLE)) {
        // currently no video input active
        rtsp_response_error(rsp, 404, "Busy");
        return RTSP_RESOURCE_ERROR;
    }

    if (!(handle.rsc_state & RSC_VIDEO_IN)) {
        // currently no video input active
        rtsp_response_error(rsp, 404, "No Video Input");
        return RTSP_RESOURCE_ERROR;
    }

    // reserve resource
    handle.vtb_state = VTB_VIDEO;

    //
    if ((n = net_get_local_hwaddr(handle.listener.sockfd, "eth0", &handle.local.mac)) != RTSP_SUCCESS) {
        rtsp_response_error(rsp, 400, "Server error");
        return n;
    }

    if ((n = net_get_local_addr(handle.listener.sockfd, "eth0", &handle.local.address)) != RTSP_SUCCESS) {
        rtsp_response_error(rsp, 400, "Server error");
        return n;
    }

    if ((n = net_get_remote_hwaddr(handle.listener.sockfd, "eth0", rsp->address, &vtb.remote.mac)) != RTSP_SUCCESS) {
        rtsp_response_error(rsp, 300, "Please retry later (ARP failure)");
        return n;
    }

    vtb.remote.address = rsp->address;
    vtb.remote.vid_port = PORT_RANGE_START(m->transport.client_port);
    handle.local.vid_port = PORT_RANGE_START(m->transport.server_port);
    rtsp_response_setup(rsp, &m->transport, media->sessionid);

    struct in_addr a1, a2; a1.s_addr = handle.local.address; a2.s_addr = vtb.remote.address;
    printf("TX: %s:%d %s",
            inet_ntoa(a1), handle.local.vid_port, mac_ntoa(handle.local.mac));
    printf(" -> %s:%d %s\n",
            inet_ntoa(a2), vtb.remote.vid_port, mac_ntoa(vtb.remote.mac));

    pthread_mutex_unlock(&handle.mutex);

    return RTSP_SUCCESS;
}

int vtb_video_play(t_rtsp_media* media, t_rtsp_req_play* m, t_rtsp_connection* rsp)
{
    // get daemon struct
    t_video_timing timing;
    t_rtsp_rtp_format fmt;
    hdoip_eth_params eth;

    // start ethernet output
    eth.ipv4_dst_ip = vtb.remote.address;
    eth.ipv4_src_ip = handle.local.address;
    eth.ipv4_tos = 0; // TODO:?
    eth.ipv4_ttl = 4;
    eth.packet_length = 1500;
    eth.rtp_ssrc = 0;
    _mac0cpy(eth.src_mac_lower32,
             eth.src_mac_upper16,
             handle.local.mac);
    _mac0cpy(eth.dst_mac_lower32,
             eth.dst_mac_upper16,
             vtb.remote.mac);
    eth.udp_dst_port = vtb.remote.vid_port;
    eth.udp_src_port = handle.local.vid_port;

    // start vsi
    fmt.compress = 1;
    //fmt.rtptime = hoi_drv_get_mtime(handle.drv);
    hoi_drv_vsi(handle.drv, true, 10000000, &eth, &timing, &fmt.value);

    // send timing
    rtsp_response_play(rsp, media->sessionid, &fmt, &timing);

    return RTSP_SUCCESS;
}

int vtb_video_teardown(t_rtsp_media* media, t_rtsp_req_teardown* m, t_rtsp_connection* rsp)
{
    // get daemon struct

}

t_rtsp_media vtb_video = {
    .owner = 0,
    .cookie = 0,
    .setup = vtb_video_setup,
    .play = vtb_video_play,
    .teardown = vtb_video_teardown
};
