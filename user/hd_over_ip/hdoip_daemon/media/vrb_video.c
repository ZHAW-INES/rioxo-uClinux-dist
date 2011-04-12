/*
 * vrb_video.c
 *
 * TODO: multicast
 *
 * hdoipd.vtb_state represents the state of the transmitter box!
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */

#include "hoi_drv_user.h"
#include "hdoipd.h"
#include "box_sys.h"
#include "vrb_video.h"
#include "hdoipd_osd.h"
#include "hdoipd_fsm.h"
#include "edid.h"
#include "../../../hdcp/receiver/receiver.h"

#define PROCESSING_DELAY_CORRECTION     (6000)
#define TICK_TIMEOUT                    (hdoipd.eth_timeout)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

static struct {
    t_hdoip_eth         remote;
    int                 timeout;
    int                 alive_ping;
} vrb;

int vrb_video_setup(t_rscp_media *media, t_rscp_rsp_setup* m, t_rscp_connection* rsp)
{
    int n;
    t_rscp_client* client = media->creator;
    report(INFO "vrb_video_setup");

    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac)) != RSCP_SUCCESS) {
        return n;
    }

    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RSCP_SUCCESS) {
        return n;
    }

    vrb.remote.address = client->con.address;
    vrb.remote.vid_port = PORT_RANGE_START(m->transport.server_port);
    hdoipd.local.vid_port = PORT_RANGE_START(m->transport.client_port);

    REPORT_RTX("RX", hdoipd.local, "<-", vrb.remote, vid);

    //start HDCP code!? If (hdcp= on) {....}
    /* - if hdcp is necessary, check if it is already enabled
     * - get port number
     * - start server and SKE if necessary
     * - if successful, write keys to HW, else go back to ?idle? state
     *
     * */
    printf("hdcp_on1: %d\n",m->hdcp.hdcp_on);
    printf("hdcp_port1: %d\n",m->hdcp.port_nr);
    sleep(2); //wait 2 sec to allow the server to start up
    //TODO: replace to sleep command (with a loop?!)
    printf("hdcp_extern_forced: %d\n",hdoipd.hdcp_extern_forced);
    /*check if hdcp is needed, only check the external command*/
    if (hdoipd.hdcp_extern_forced == 1){
  	  //TODO: check if already enabled
  	  char port_nr_string[10];
  	  char riv[17];
  	  char session_key[33];
  	  char *ip;
  	  ip=reg_get("remote-uri");
  	  ip += 7;
  	  printf("Remote-uri: %s\n",ip);
  	  sprintf(port_nr_string, "%d",m->hdcp.port_nr);
  	  receiver(port_nr_string,ip,session_key,riv); //start server for SKE
  	  //TODO: check if SKE was successful
  	  //write keys down to HW
    }

#ifdef ETI_PATH
    // setup ethernet input
    hoi_drv_eti(hdoipd.local.address, vrb.remote.address,
            hdoipd.local.vid_port, 0);
#endif

    hdoipd_set_tstate(VTB_VID_IDLE);

    media->result = RSCP_RESULT_READY;
    vrb.alive_ping = 1;
    vrb.timeout = 0;

    return RSCP_SUCCESS;
}

int vrb_video_play(t_rscp_media *media, t_rscp_rsp_play* m, t_rscp_connection UNUSED *rsp)
{
    uint32_t compress = 0;
    report(INFO "vrb_video_play");

    media->result = RSCP_RESULT_PLAYING;

    osd_permanent(false);

    // set slave timer when not already synced
    if (!hdoipd_rsc(RSC_SYNC)) {
        hoi_drv_set_stime(m->format.rtptime+PROCESSING_DELAY_CORRECTION);
    }

    if (m->format.compress) {
        compress |= DRV_CODEC_JP2;
    }
    if (reg_test("mode-sync", "streamsync")) {
        if (reg_test("sync-target", "video") || !hdoipd_rsc(RSC_SYNC)) {
            compress |= DRV_STREAM_SYNC;
            hdoipd_set_rsc(RSC_VIDEO_SYNC);
        }
    }

#ifdef VID_OUT_PATH
    // start vso (20 ms max. network delay)
    hoi_drv_vso(compress, 0, &m->timing, m->format.value, reg_get_int("network-delay"));
#endif

    hdoipd_set_tstate(VTB_VIDEO);
    hdoipd_set_rsc(RSC_VIDEO_OUT);

    struct in_addr a1; a1.s_addr = vrb.remote.address;
    osd_printf("Streaming Video %d x %d from %s\n", m->timing.width, m->timing.height, inet_ntoa(a1));

    return RSCP_SUCCESS;
}

int vrb_video_teardown(t_rscp_media *media, t_rscp_rsp_teardown UNUSED *m, t_rscp_connection UNUSED *rsp)
{
    report(INFO "vrb_video_teardown");

    media->result = RSCP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_VIDEO|VTB_VID_IDLE)) {
#ifdef VID_OUT_PATH
        hdoipd_hw_reset(DRV_RST_VID_OUT);
#endif
        hdoipd_clr_rsc(RSC_VIDEO_OUT|RSC_OSD|RSC_VIDEO_SYNC);
        hdoipd_set_tstate(VTB_VID_OFF);
    }

    if (rsp) {
        osd_permanent(true);
        osd_printf("video remote off...\n");
    }

    return RSCP_SUCCESS;
}


int vrb_video_error(t_rscp_media *media, intptr_t m, t_rscp_connection* rsp)
{
    if(rsp) {
        report(" ? client failed (%d): %d - %s", m, rsp->ecode, rsp->ereason);
        osd_permanent(true);
        osd_printf("vrb.video streaming could not be established: %d - %s\n", rsp->ecode, rsp->ereason);
        switch(rsp->ecode) {
            case 300:   media->result = RSCP_RESULT_SERVER_TRY_LATER;
                        break;
            case 404:   media->result = RSCP_RESULT_SERVER_BUSY;
                        break;
            case 405:   media->result = RSCP_RESULT_SERVER_NO_VTB;
                        break;
            case 406:   media->result = RSCP_RESULT_SERVER_NO_VIDEO_IN;
                        break;
            case 400:
            default:    media->result = RSCP_RESULT_SERVER_ERROR;
                        break;
        }
    } else {    
        osd_permanent(true);
        osd_printf("vrb.video streaming could not be established: connection refused\n");
        media->result = RSCP_RESULT_CONNECTION_REFUSED;
    }
    return RSCP_SUCCESS;
}

void vrb_video_pause(t_rscp_media *media)
{
    media->result = RSCP_RESULT_PAUSE_Q;

    if (hdoipd_tstate(VTB_VIDEO)) {

#ifdef VID_OUT_PATH
        hdoipd_hw_reset(DRV_RST_VID_OUT);
#endif
#ifdef ETI_PATH
        hoi_drv_eti(hdoipd.local.address, vrb.remote.address,
                hdoipd.local.vid_port, 0);
#endif
        hdoipd_clr_rsc(RSC_VIDEO_OUT|RSC_OSD|RSC_VIDEO_SYNC);
        hdoipd_set_tstate(VTB_VID_IDLE);
    }

    // goto ready without further communication
    rscp_media_force_ready(media);
}

int vrb_video_update(t_rscp_media *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
{
    switch (m->event) {

        case EVENT_TICK:
            // reset timeout
            vrb.timeout = 0;
        break;

        case EVENT_VIDEO_IN_ON:
            // No multicast for now...simply stop before starting new
            if (rscp_media_splaying(media)) {
                vrb_video_pause(media);
            }

            // restart
            unlock("vrb_video_update");
                hdoipd_launch(hdoipd_start_vrb, media, 250, 3, 1000);
            lock("vrb_video_update");
        break;

        case EVENT_VIDEO_IN_OFF:
            vrb_video_pause(media);
            osd_permanent(true);
            osd_printf("vtb.video stoped streaming...\n");
            report(ERROR "vtb.video stoped streaming");
        break;
    }

    return RSCP_SUCCESS;
}

int vrb_video_ready(t_rscp_media UNUSED *media)
{
    if (hdoipd_tstate(VTB_VID_MASK)) {
        // not ready ->
        return RSCP_ERRORNO;
    }
    if (!hdoipd_rsc(RSC_VIDEO_SINK)) {
        // no video sink
        return RSCP_ERRORNO;
    }
    return RSCP_SUCCESS;
}

int vrb_video_dosetup(t_rscp_media *media)
{
    int port;
    char *p;
    t_rscp_transport transport;
    t_rscp_edid edid;
    t_rscp_client *client = media->creator;
    t_rscp_hdcp hdcp;
    //hdcp.hdcp_on = 0;
    hdcp.port_nr = 55000;	  //set port number
    hdoipd.hdcp_hdmi_forced=0;//this value has to be set by the hdmi chip
    hdoipd.hdcp_extern_forced=0;

    p = reg_get("hdcp-force");
    printf("Value HDCP force: %s/n",p);
    /*check if hdcp is needed*/
    if ((hdoipd.hdcp_hdmi_forced==1)||(hdoipd.hdcp_extern_forced==1)||((strcmp(p, "1")==0))){
      hdcp.hdcp_on = 1;
    }
    else{
      hdcp.hdcp_on = 0;
    }
    printf("hdcp_dccp_on: %d/n",hdcp.hdcp_on);

    if (!client) return RSCP_NULL_POINTER;

    rscp_default_transport(&transport); // TODO: daemon transport configuration
    port = reg_get_int("video-port");
    transport.client_port = PORT_RANGE(htons(port), htons(port+1));

    edid.segment = 0;
    hoi_drv_rdedid(edid.edid);
    edid_report((void*)edid.edid);

    return rscp_client_setup(client, &transport, &edid, &hdcp);
}

int vrb_video_doplay(t_rscp_media *media)
{
    char *s;
    t_rscp_rtp_format fmt;
    t_rscp_client *client = media->creator;

    if (!client) return RSCP_NULL_POINTER;

    // open media
    s = reg_get("compress");
    if (strcmp(s, "jp2k") == 0) {
        fmt.compress = FORMAT_JPEG2000;
    } else {
        fmt.compress = FORMAT_PLAIN;
    }
    fmt.rtptime = 0;
    fmt.value = reg_get_int("advcnt-min");

    return rscp_client_play(client, &fmt);
}

int vrb_video_event(t_rscp_media *media, uint32_t event)
{
    t_rscp_client *client = media->creator;

    switch (event) {
        case EVENT_TICK:
            if (vrb.alive_ping) {
                vrb.alive_ping--;
            } else {
                vrb.alive_ping = TICK_SEND_ALIVE;
                // send tick we are alive (until something like rtcp is used)
                rscp_client_update(media->creator, EVENT_TICK);
            }
            if (vrb.timeout <= TICK_TIMEOUT) {
                vrb.timeout++;
            } else {
                report(INFO "vrb_video_event: timeout");
                // timeout -> kill connection
                vrb.timeout = 0;
                rscp_client_kill(client);
                osd_permanent(true);
                osd_printf("vrb.video connection lost...\n");
            }
        break;

        case EVENT_VIDEO_SINK_OFF:
            // Note: after teardown call media/client is not anymore a valid struct
            rscp_client_teardown(client);
        break;

    }

    return RSCP_SUCCESS;
}

t_rscp_media vrb_video = {
    .name = "video",
    .owner = 0,
    .cookie = 0,
    .setup = (frscpm*)vrb_video_setup,
    .play = (frscpm*)vrb_video_play,
    .teardown = (frscpm*)vrb_video_teardown,
    .error = (frscpm*)vrb_video_error,
    .update = (frscpm*)vrb_video_update,

    .ready = (frscpl*)vrb_video_ready,
    .dosetup = (frscpl*)vrb_video_dosetup,
    .doplay = (frscpl*)vrb_video_doplay,
    .event = (frscpe*)vrb_video_event
};

