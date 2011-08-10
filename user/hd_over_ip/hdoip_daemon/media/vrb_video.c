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
#include "hdcp.h"
#include "rscp_string.h"
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

int vrb_video_setup(t_rscp_media *media, t_rscp_rsp_setup* m, t_rscp_connection* rsp)
{
    int n;
    t_rscp_client* client = media->creator;
    report(VRB_METHOD "vrb_video_setup");

    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac)) != RSCP_SUCCESS) {
        return n;
    }

    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RSCP_SUCCESS) {
        return n;
    }

    vrb.remote.address = client->con.address;
    vrb.remote.vid_port = PORT_RANGE_START(m->transport.server_port);
    hdoipd.local.vid_port = PORT_RANGE_START(m->transport.client_port);

    vrb.multicast_en = m->transport.multicast;

    if (vrb.multicast_en) {
        vrb.dst_ip = m->transport.multicast_group;
    }
    else {
        vrb.dst_ip = hdoipd.local.address;
    }

    REPORT_RTX("RX", hdoipd.local, "<-", vrb.remote, vid);

    /*start hdcp session key exchange if necessary */
    report("Check if HDCP is necessary and start ske");
    hdoipd.hdcp.enc_state = m->hdcp.hdcp_on;

    if ((m->hdcp.hdcp_on == 1) && !(hdoipd.hdcp.ske_executed)){
		if (rscp_client_hdcp(client) != RSCP_SUCCESS){
			report(" ? Session key exchange failed");
			rscp_err_hdcp(rsp);
			return RSCP_REQUEST_ERROR;
		}
    }

#ifdef ETI_PATH
    // setup ethernet input
    hoi_drv_eti(vrb.dst_ip, vrb.remote.address, 0, hdoipd.local.vid_port, 0);
#endif

    hdoipd_set_vtb_state(VTB_VID_IDLE);

    media->result = RSCP_RESULT_READY;
    vrb.alive_ping = 1;
    vrb.timeout = 0;

    return RSCP_SUCCESS;
}

int vrb_video_play(t_rscp_media *media, t_rscp_rsp_play* m, t_rscp_connection UNUSED *rsp)
{
    uint32_t compress = 0;
    report(VRB_METHOD "vrb_video_play");

    //Test if HDCP parameters were set correctly
	if (hdoipd.hdcp.enc_state && !(get_hdcp_status() & HDCP_ETI_VIDEO_EN)){
		if (hdoipd.hdcp.ske_executed){
			hoi_drv_hdcp(hdoipd.hdcp.keys); 	/* write keys to kernel */
			report(INFO "Video encryption enabled (eti)!");
			hoi_drv_hdcp_viden_eti();
		}
		else {
			report(INFO "No valid HDCP ske executed!");
			return RSCP_ERRORNO;
		}
	}

    media->result = RSCP_RESULT_PLAYING;

    osd_permanent(false);

    // join multicast group
    if (vrb.multicast_en) {
        join_multicast_group(vrb.dst_ip);
    }

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

    hdoipd_set_vtb_state(VTB_VIDEO);
    hdoipd_set_rsc(RSC_VIDEO_OUT);

    struct in_addr a1; a1.s_addr = vrb.remote.address;
    osd_printf("Streaming Video %d x %d from %s\n", m->timing.width, m->timing.height, inet_ntoa(a1));

    return RSCP_SUCCESS;
}

int vrb_video_teardown(t_rscp_media *media, t_rscp_rsp_teardown UNUSED *m, t_rscp_connection *rsp)
{
    report(VRB_METHOD "vrb_video_teardown");

    media->result = RSCP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_VIDEO|VTB_VID_IDLE)) {
#ifdef VID_OUT_PATH
        hdoipd_hw_reset(DRV_RST_VID_OUT);
#endif
        hdoipd_clr_rsc(RSC_VIDEO_OUT|RSC_OSD|RSC_VIDEO_SYNC);
        hdoipd_set_vtb_state(VTB_VID_OFF);
    }
    
    if (vrb.multicast_en) {
        leave_multicast_group(vrb.dst_ip);
    }

    if (rsp) {
        osd_permanent(true);
        osd_printf("video remote off...\n");
    }

    // TODO: start sending alive packets should not be in media
    // start sending hello frames to vtb
    if(hdoipd_rsc(RSC_VIDEO_SINK)) {
        if(hdoipd.auto_stream) {
            alive_check_start_vrb_alive();
        }
    }

    return RSCP_SUCCESS;
}


int vrb_video_error(t_rscp_media *media, intptr_t m, t_rscp_connection* rsp)
{
	t_rscp_client *client = media->creator;

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
            case 408:   media->result = RSCP_RESULT_SERVER_HDCP_ERROR;
                        rscp_client_set_teardown(client);
                        hdoipd_set_task_start_vrb();
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
	report(INFO "vrb_video_pause");
    media->result = RSCP_RESULT_PAUSE_Q;

    report(VRB_METHOD "vrb_video_pause");

    if (hdoipd_tstate(VTB_VIDEO)) {
    report(INFO "vrb_video_pause: VTB_VIDEO");
#ifdef VID_OUT_PATH
        hdoipd_hw_reset(DRV_RST_VID_OUT);
#endif
#ifdef ETI_PATH
        hoi_drv_eti(vrb.dst_ip, vrb.remote.address, 0, hdoipd.local.vid_port, 0);
#endif
        hdoipd_clr_rsc(RSC_VIDEO_OUT|RSC_OSD|RSC_VIDEO_SYNC);
        hdoipd_set_vtb_state(VTB_VID_IDLE);
    }
}

int vrb_video_update(t_rscp_media *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
{
	t_rscp_client *client = media->creator;
	//report(INFO "EVENT NR: %08x", m->event);
    switch (m->event) {

        case EVENT_TICK:
            // reset timeout
            vrb.timeout = 0;
        break;
        case EVENT_VIDEO_IN_ON:
            // No multicast for now...simply stop before starting new
        	report(INFO "vrb_video_update; EVENT_VIDEO_IN_ON");
            if (rscp_media_splaying(media)) {
                vrb_video_pause(media);
            }
            // restart
            rscp_client_set_play(media->creator);
            return RSCP_PAUSE;
        break;
        case EVENT_VIDEO_IN_OFF:
        	report(INFO "vrb_video_update; EVENT_VIDEO_IN_OFF");
            vrb_video_pause(media);
            osd_permanent(true);
            osd_printf("vtb.video stopped streaming...\n");
            report(ERROR "vtb.video stopped streaming");
            return RSCP_PAUSE;
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
    t_rscp_transport transport;
    t_rscp_edid edid;
    t_rscp_client *client = media->creator;
    t_rscp_hdcp hdcp;

    hdcp.hdcp_on = reg_test("hdcp-force", "true");

    report(VRB_METHOD "vrb_video_dosetup");

    if (!client) return RSCP_NULL_POINTER;

    rscp_default_transport(&transport); // TODO: daemon transport configuration
    port = reg_get_int("video-port");
    transport.client_port = PORT_RANGE(htons(port), htons(port+1));

    edid.segment = 0;
    hoi_drv_rdedid(edid.edid);
#ifdef REPORT_EDID
    edid_report((void*)edid.edid);
#endif

    return rscp_client_setup(client, &transport, &edid, &hdcp);
}

int vrb_video_doplay(t_rscp_media *media)
{
    char *s;
    t_rscp_rtp_format fmt;
    t_rscp_client *client = media->creator;

    report(VRB_METHOD "vrb_video_doplay");

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
                rscp_client_update(client, EVENT_TICK);
            }
            if (vrb.timeout <= TICK_TIMEOUT) {
                vrb.timeout++;
            } else {
                report(INFO "vrb_video_event: timeout");
                // timeout -> kill connection
                vrb.timeout = 0;
                rscp_client_set_kill(client);
                osd_permanent(true);
                osd_printf("vrb.video connection lost...\n");
            }
        break;

        case EVENT_VIDEO_SINK_OFF:
            // Note: after teardown call media/client is not anymore a valid struct
            rscp_client_set_teardown(client);
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

