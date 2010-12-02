/*
 * vrb_video.c
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */

#include "hdoipd.h"
#include "vrb_video.h"

extern t_hdoipd handle;

struct {
    t_hdoip_eth         remote;
} vrb;

int vrb_video_setup(t_rtsp_media* media, t_rtsp_rsp_setup* m, t_rtsp_connection* rsp)
{
    int n;

    if ((n = net_get_local_hwaddr(handle.listener.sockfd, "eth0", &handle.local.mac)) != RTSP_SUCCESS) {
        rtsp_response_error(rsp, 400, "Server error");
        return n;
    }

    if ((n = net_get_local_addr(handle.listener.sockfd, "eth0", &handle.local.address)) != RTSP_SUCCESS) {
        rtsp_response_error(rsp, 400, "Server error");
        return n;
    }


    vrb.remote.address = rsp->address;
    vrb.remote.vid_port = PORT_RANGE_START(m->transport.server_port);
    handle.local.vid_port = PORT_RANGE_START(m->transport.client_port);

    struct in_addr a1, a2; a1.s_addr = handle.local.address; a2.s_addr = vrb.remote.address;
    printf("RX: %s:%d %s",
            inet_ntoa(a1), handle.local.vid_port, mac_ntoa(handle.local.mac));
    printf(" <- %s:%d %s\n",
            inet_ntoa(a2), vrb.remote.vid_port, mac_ntoa(vrb.remote.mac));
    // setup ethernet input
    hoi_drv_eti(handle.drv, handle.local.address, vrb.remote.address,
            handle.local.vid_port, handle.local.aud_port);

    return RTSP_SUCCESS;
}

int vrb_video_play(t_rtsp_media* media, t_rtsp_rsp_play* m, t_rtsp_connection* rsp)
{
    // set timer

    // start vso
    hoi_drv_vso(handle.drv, m->format.compress, &m->timing, m->format.value);

    return RTSP_SUCCESS;
}

int vrb_video_teardown(t_rtsp_media* media, t_rtsp_rsp_teardown* m, t_rtsp_connection* rsp)
{
    return RTSP_SUCCESS;
}

int vrb_video_error(t_rtsp_media* media, intptr_t m, t_rtsp_connection* rsp)
{
    printf(" ? client failed (%d): %d - %s\n", m, rsp->ecode, rsp->ereason);
    return RTSP_SUCCESS;
}


t_rtsp_media vrb_video = {
    .owner = 0,
    .cookie = 0,
    .setup = vrb_video_setup,
    .play = vrb_video_play,
    .teardown = vrb_video_teardown,
    .error = vrb_video_error
};
