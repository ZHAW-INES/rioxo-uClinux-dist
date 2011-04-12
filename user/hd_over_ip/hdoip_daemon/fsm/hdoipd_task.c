/*
 * hdoipd_task.c
 *
 *  Created on: 07.12.2010
 *      Author: alda
 */
#include "hoi_drv_user.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "rscp_listener.h"

// local buffer
static char buf[256];

void task_get_drivers(char** p)
{
    char * tmp = buf;

    if (hdoipd.drivers & DRV_ADV9889) tmp += sprintf(tmp, "ADV9889 ");
    if (hdoipd.drivers & DRV_ADV7441) tmp += sprintf(tmp, "ADV7441 ");

    if (tmp!=buf) tmp--;
    *tmp = 0;
    *p = buf;
}

void task_get_state(char** p)
{
    switch (hdoipd.state) {
        case HOID_IDLE: *p = "idle"; break;
        case HOID_READY: *p = "ready"; break;
        case HOID_SHOW_CANVAS: *p = "show canvas"; break;
        case HOID_LOOP: *p = "loop"; break;
        case HOID_VRB: *p = "vrb"; break;
        case HOID_VTB: *p = "vtb"; break;
    }
}

void task_get_vtb_state(char** p)
{
    switch (hdoipd.state) {
        case HOID_IDLE:
        case HOID_READY:
        case HOID_SHOW_CANVAS:
        case HOID_LOOP:
            *p = "not valid";
        break;
        case HOID_VRB:
            *p = (char*)vtbstatestr(hdoipd.vtb_state);
        break;
        case HOID_VTB:
            *p = (char*)vtbstatestr(hdoipd.vtb_state);
        break;
    }
}

void task_get_vrb_state(char** p)
{
    switch (hdoipd.state) {
        case HOID_IDLE:
        case HOID_READY:
        case HOID_SHOW_CANVAS:
        case HOID_LOOP:
            *p = "not valid";
        break;
        case HOID_VTB:
            switch (hdoipd.vrb_state) {
                case VRB_OFF: *p = "off (remote state unknown)"; break;
                case VRB_IDLE: *p = "no audio backchannel"; break;
                case VRB_AUDIO: *p = "audio backchannel active"; break;
            }
        break;
        case HOID_VRB:
            switch (hdoipd.vrb_state) {
                case VRB_OFF: *p = "idle"; break;
                case VRB_IDLE: *p = "idle"; break;
                case VRB_AUDIO: *p = "sending audio back"; break;
            }
        break;
    }
}

void task_get_rsc_state(char** p)
{
    char * tmp = buf;

    if (hdoipd.rsc_state & RSC_VIDEO_IN) tmp += sprintf(tmp, "Video-In ");
    if (hdoipd.rsc_state & RSC_AUDIO_IN) tmp += sprintf(tmp, "Audio-In ");
    if (hdoipd.rsc_state & RSC_VIDEO_SINK) tmp += sprintf(tmp, "Video-Sink ");
    if (hdoipd.rsc_state & RSC_VIDEO_OUT) tmp += sprintf(tmp, "Video-Out ");
    if (hdoipd.rsc_state & RSC_AUDIO_OUT) tmp += sprintf(tmp, "Audio-Out ");
    if (hdoipd.rsc_state & RSC_OSD) tmp += sprintf(tmp, "OSD ");
    if (hdoipd.rsc_state & RSC_VIDEO_SYNC) tmp += sprintf(tmp, "Sync-Video ");
    if (hdoipd.rsc_state & RSC_AUDIO_SYNC) tmp += sprintf(tmp, "Sync-Audio ");

    if (tmp!=buf) tmp--;
    *tmp = 0;
    *p = buf;
}

void task_get_eth_status(char** p)
{
    t_hoi_msg_ethstat *stat;
    hoi_drv_ethstat(&stat);
    sprintf(buf, "TX|RX (C: %d|%d, V: %d|%d, A: %d|%d, Inv: %d|%d)",
            stat->tx_cpu_cnt, stat->rx_cpu_cnt,
            stat->tx_vid_cnt, stat->rx_vid_cnt,
            stat->tx_aud_cnt, stat->rx_aud_cnt,
            stat->tx_inv_cnt, stat->rx_inv_cnt);
    *p = buf;
}

void task_get_vso_status(char** p)
{
    t_hoi_msg_vsostat *stat;
    char * tmp = buf;

    hoi_drv_vsostat(&stat);
    tmp += sprintf(buf, "VFrames|Skip: %d|%d, Packets|Missed %d|%d, Received: %d packets",
            stat->vframe_cnt, stat->vframe_skip,
            stat->packet_cnt, stat->packet_lost,
            stat->packet_in_cnt);
    if (stat->status) tmp += sprintf(tmp, " [");
    if (stat->status&1) tmp += sprintf(tmp, "payload empty,");
    if (stat->status&2) tmp += sprintf(tmp, "marker timeout,");
    if (stat->status&4) tmp += sprintf(tmp, "idle,");
    if (stat->status&8) tmp += sprintf(tmp, "stream active,");
    if (stat->status&16) tmp += sprintf(tmp, "choked,");
    if (stat->status&32) tmp += sprintf(tmp, "timestamp error,");
    if (stat->status) sprintf(tmp-1, "]");

    *p = buf;
}

void task_get_vio_status(char** p)
{
    t_hoi_msg_viostat *stat;
    hoi_drv_viostat(&stat);
    sprintf(buf, "fin: %d Hz, fout: %d Hz, vclk-error: %d * 1/fout, pll-error: %d\n"
            "In|Out (Vid: %d|%d St: %d|%d), force-vsync: %d",
            stat->fin, stat->fout,
            stat->tgerr, stat->pllerr,
            stat->vid_in, stat->vid_out,
            stat->st_in, stat->st_out,
            stat->fvsync);
    *p = buf;
}

void task_get_sync_delay(char** p)
{
    int32_t x;
    hoi_drv_get_syncdelay(&x);
    if (x == 0)
        sprintf(buf, "stream synchronization is not running");
    else
        sprintf(buf, "Stream-delay = %dus", x);
    *p = buf;
}

void task_get_system_state(char** p)
{
    char *tmp = buf;

    if (hdoipd_state(HOID_VTB)) tmp += sprintf(tmp, "Video Transmitter Box: ");
    else if (hdoipd_state(HOID_VTB)) tmp += sprintf(tmp, "Video Receiver Box: ");
    else tmp += sprintf(tmp, "Box is Idle. ");

    if (hdoipd_state(HOID_VTB|HOID_VRB)) {
        int m = hdoipd_tstate(VTB_VIDEO|VTB_AUDIO);
        if (m==(VTB_VIDEO|VTB_AUDIO)) tmp += sprintf(tmp, "Streaming Video & Audio. ");
        if (m==VTB_VIDEO) tmp += sprintf(tmp, "Streaming Video. ");
        if (m==VTB_AUDIO) tmp += sprintf(tmp, "Streaming Audio. ");
    }

    if (hdoipd.update) tmp += sprintf(tmp, "Device update pending. Restart box to continue");

    *p = buf;
}

char* task_conv_rscp_state(int state)
{
    switch(state) {
        case RSCP_RESULT_IDLE                   : return "idle";
        case RSCP_RESULT_READY                  : return "ready";
        case RSCP_RESULT_PLAYING                : return "playing";
        case RSCP_RESULT_TEARDOWN               : return "teardown";
        case RSCP_RESULT_TEARDOWN_Q             : return "teardown_q";
        case RSCP_RESULT_PAUSE                  : return "pause";
        case RSCP_RESULT_PAUSE_Q                : return "pause_q";
        case RSCP_RESULT_NO_VIDEO_IN            : return "no video in";
        case RSCP_RESULT_SERVER_ERROR           : return "server error";
        case RSCP_RESULT_SERVER_BUSY            : return "server busy";
        case RSCP_RESULT_SERVER_NO_VTB          : return "server no vtb";
        case RSCP_RESULT_SERVER_TRY_LATER       : return "server try later";
        case RSCP_RESULT_SERVER_NO_VIDEO_IN     : return "server no video in";
        default                                 : 
        case RSCP_RESULT_CONNECTION_REFUSED     : return "connection refused";
    }
}

char *buf_ptr;
void task_get_rscp_sessions(char UNUSED *key, char* value, void* fd)
{
    int *cnt = (int *) fd; 
    t_rscp_media* media = (t_rscp_media*) value;
    buf_ptr += sprintf(buf_ptr, "stream %02d {out}: %s (%s)\n", *cnt, task_conv_rscp_state(media->result), media->owner->name);
    *cnt += 1;
}

void task_get_rscp_state(char** s)
{
    t_video_timing vid_timing;
    uint32_t advcnt;
    int cnt = 0;
    
    buf_ptr = buf;
    
    buf_ptr += sprintf(buf_ptr, "\nvideo sink     : ");
    if(hdoipd.rsc_state & RSC_VIDEO_SINK) {
        buf_ptr += sprintf(buf_ptr, "connected\n");
    } else {
        buf_ptr += sprintf(buf_ptr, "not connected\n");
    }

    buf_ptr += sprintf(buf_ptr, "video source   : ");
    if(hdoipd.rsc_state & RSC_VIDEO_IN) {
        buf_ptr += sprintf(buf_ptr, "connected\n");
    } else {
        buf_ptr += sprintf(buf_ptr, "not connected\n");
    }

    hoi_drv_info(&vid_timing, &advcnt);
    buf_ptr += sprintf(buf_ptr, "resolution     : %d x %d @ %dHz\n", vid_timing.width, vid_timing.height, vid_timing.pfreq);


    t_rscp_client* client;
    LIST_FOR(client, hdoipd.client) {
        buf_ptr += sprintf(buf_ptr, "stream %02d {in} : %s (%s)\n",client->nr, task_conv_rscp_state(client->media->result), client->media->name);
    }

    if(hdoipd.listener.sessions) {
        unlock("task_get_stream_state");
        rscp_listener_session_traverse(&hdoipd.listener, task_get_rscp_sessions, &cnt);
        lock("task_get_stream_state"); 
    }

    *s = buf;
}

int task_ready()
{
    int state = hdoipd.state;
    if (hdoipd_state(HOID_VRB|HOID_VTB)) {
        hdoipd_goto_ready();
    }
    return state;
}

void task_restart(int state)
{
    switch (state) {
        case HOID_VRB: hdoipd_goto_vrb(); break;
        case HOID_VTB: hdoipd_goto_vtb(); break;
    }
}

void task_set_bw(char* p)
{
    int bw = atoi(p);
    report("update bandwidth: %d Byte/s", bw);
    bw = bw - bw / 20; // 5% overhead approx.
    if (bw) hoi_drv_bw(bw);
}

void task_set_ip(char* p)
{
    int tmp = task_ready();
    // ip changed:
    sprintf(buf, "/sbin/ifconfig %s %s netmask %s up", reg_get("system-ifname"), p, reg_get("system-subnet"));
    system(buf);
    task_restart(tmp);
}

void task_set_subnet(char* p)
{
    // subnet changed:
    sprintf(buf, "/sbin/ifconfig %s %s netmask %s up", reg_get("system-ifname"), reg_get("system-ip"), p);
    system(buf);
}

void task_set_gateway(char* p)
{
    // gateway changed:
    sprintf(buf, "/sbin/route add default gw %s", p);
    system(buf);
}

void task_set_mac(char* p)
{
    // mac changed:
    sprintf(buf, "/sbin/ifconfig %s hw ether %s", reg_get("system-ifname"), p);
    system(buf);
}

void task_set_remote(char* p)
{
    if (hdoipd_state(HOID_VRB)) {
        hdoipd_goto_ready();
        hdoipd_goto_vrb();
    }
}

void task_set_hello(char* p)
{
    hdoipd_hello();
}

void task_set_mode_start(char* p)
{
    hdoipd_goto_ready();
    hdoipd_start();
}

void task_set_mode_media(char* p)
{
    if (hdoipd_state(HOID_VRB)) {
        hdoipd_goto_ready();
        hdoipd_start();
    }
}

void hdoipd_register_task()
{
    get_listener("system-state", task_get_system_state);
    get_listener("daemon-driver", task_get_drivers);
    get_listener("daemon-state", task_get_state);
    get_listener("daemon-vtb-state", task_get_vtb_state);
    get_listener("daemon-vrb-state", task_get_vrb_state);
    get_listener("daemon-rsc-state", task_get_rsc_state);
    get_listener("eth-status", task_get_eth_status);
    get_listener("vso-status", task_get_vso_status);
    get_listener("vio-status", task_get_vio_status);
    get_listener("sync-delay", task_get_sync_delay);
    get_listener("stream-state", task_get_rscp_state);

    // set-listener are called when a new value is written to the register
    // if the same value is written as already stored the listener isn't called
    set_listener("bandwidth", task_set_bw);
    set_listener("system-ip", task_set_ip);
    set_listener("system-subnet", task_set_subnet);
    set_listener("system-gateway", task_set_gateway);
    set_listener("system-mac", task_set_mac);
    set_listener("mode-start", task_set_mode_start);
    set_listener("mode-media", task_set_mode_media);
    set_listener("remote-uri", task_set_remote);
    set_listener("hello-uri", task_set_hello);
}


