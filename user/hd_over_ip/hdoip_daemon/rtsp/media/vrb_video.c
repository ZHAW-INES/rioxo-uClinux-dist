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

#include "box_sys.h"
#include "edid.h"
#include "hdcp.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hdoipd_osd.h"
#include "hoi_drv_user.h"
#include "multicast.h"
#include "rtsp_client.h"
#include "rtsp_error.h"
#include "rtsp_net.h"
#include "rtsp_string.h"
#include "vrb_video.h"

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

int vrb_video_describe(t_rtsp_media *media, void *_data, t_rtsp_connection *con)
{
    t_rtsp_req_describe *data = _data;

    if (!data)
        return -1;

    rtsp_handle_describe_generic(media, data, con);

    /* Add media specific content */

    /* TODO: List all available media descriptors */

    rtsp_send(con);

    return 0;
}

int vrb_video_setup(t_rtsp_media *media, t_rtsp_rsp_setup* m, t_rtsp_connection* rsp UNUSED)
{
    int n;
    t_rtsp_client* client = media->creator;
    report(VRB_METHOD "vrb_video_setup");

    if ((n = net_get_local_hwaddr(hdoipd.listener.sockfd, "eth0", (uint8_t*)&hdoipd.local.mac)) != RTSP_SUCCESS) {
        return n;
    }

    if ((n = net_get_local_addr(hdoipd.listener.sockfd, "eth0", &hdoipd.local.address)) != RTSP_SUCCESS) {
        return n;
    }

    vrb.remote.address = client->con.address;
    vrb.remote.vid_port = PORT_RANGE_START(m->transport.server_port);
    hdoipd.local.vid_port = PORT_RANGE_START(m->transport.client_port);

    vrb.multicast_en = m->transport.multicast;

    if (vrb.multicast_en) {
        vrb.dst_ip = m->transport.destination;
    }
    else {
        vrb.dst_ip = hdoipd.local.address;
    }

    REPORT_RTX("RX", hdoipd.local, "<-", vrb.remote, vid);

    // do not output HDCP encrypted content on SDI
    if ((m->hdcp.hdcp_on == 1) && (hdoipd.drivers & DRV_GS2972)) {
        report(INFO "encrypted video output on SDI is not allowed");
        osd_permanent(true);
        osd_printf("encrypted video output on SDI is not allowed\n");
        rtsp_client_set_teardown(client);
        rtsp_client_set_kill(client);
        return RTSP_REQUEST_ERROR;
    }

    /*start hdcp session key exchange if necessary */
    report("Check if HDCP is necessary and start ske");
    hdoipd.hdcp.enc_state = m->hdcp.hdcp_on;

    if ((m->hdcp.hdcp_on == 1) && !(hdoipd.hdcp.ske_executed)){
		if (rtsp_client_hdcp(client) != RTSP_SUCCESS){
			report(" ? Session key exchange failed");
			//rtsp_err_hdcp(rsp);  DONT CALL THIS AT VRB!!!!!!!!!!!!!!!
			return RTSP_REQUEST_ERROR;
		}
    }

#ifdef ETI_PATH
    // setup ethernet input
    hoi_drv_eti(vrb.dst_ip, vrb.remote.address, 0, hdoipd.local.vid_port, 0);
#endif

    hdoipd_set_vtb_state(VTB_VID_IDLE);

    media->result = RTSP_RESULT_READY;
    vrb.alive_ping = 1;
    vrb.timeout = 0;

    return RTSP_SUCCESS;
}

int vrb_video_play(t_rtsp_media *media, t_rtsp_rsp_play* m, t_rtsp_connection UNUSED *rsp)
{
    uint32_t compress = 0;
    report(VRB_METHOD "vrb_video_play");
    char stream_type[20];
    
	if (hdoipd.hdcp.enc_state) {
        //Test if HDCP parameters were set correctly
        if (!(get_hdcp_status() & HDCP_ETI_VIDEO_EN)) {
		    if (hdoipd.hdcp.ske_executed){
			    hoi_drv_hdcp(hdoipd.hdcp.keys); 	/* write keys to kernel */ 
			    report(INFO "Video encryption enabled (eti)!");
			    hoi_drv_hdcp_viden_eti();
                // enable HDCP on AD9889
                hoi_drv_hdcp_adv9889en();
		    }
		    else {
			    report(INFO "No valid HDCP ske executed!");
			    return RTSP_ERRORNO;
		    }
        } else {
            // disable HDCP on AD9889
            hoi_drv_hdcp_adv9889dis();
        }
	}

    media->result = RTSP_RESULT_PLAYING;

    osd_permanent(false);

    // join multicast group
    if (vrb.multicast_en) {
        multicast_group_join(vrb.dst_ip);
    }

    // set slave timer when not already synced
    if (!hdoipd_rsc(RSC_SYNC)) {
        hoi_drv_set_stime(m->format.rtptime+PROCESSING_DELAY_CORRECTION-21000); //TODO: set slave timer correctly
    }

    if (m->format.compress == FORMAT_UNKNOWN)
        m->format.compress = FORMAT_JPEG2000;
    // the driver expects 0 = plain, > 0 = compressed
    if (m->format.compress > 0)
      m->format.compress -= 1;

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

    if (vrb.multicast_en) {
        snprintf(stream_type, 20, " Multicast ");
    } else {
        snprintf(stream_type, 20, " ");
    }

    struct in_addr a1; a1.s_addr = vrb.remote.address;
    osd_printf("Streaming%sVideo %d x %d from %s\n", stream_type, m->timing.width, m->timing.height, inet_ntoa(a1));

    hoi_drv_set_led_status(SDI_OUT_NO_AUDIO);

    return RTSP_SUCCESS;
}

int vrb_video_teardown(t_rtsp_media *media, t_rtsp_rsp_teardown UNUSED *m, t_rtsp_connection *rsp UNUSED)
{
    report(VRB_METHOD "vrb_video_teardown");

    media->result = RTSP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_VIDEO|VTB_VID_IDLE)) {
#ifdef VID_OUT_PATH
        hdoipd_hw_reset(DRV_RST_VID_OUT);
#endif
        hdoipd_clr_rsc(RSC_VIDEO_OUT|RSC_OSD|RSC_VIDEO_SYNC);
        hdoipd_set_vtb_state(VTB_VID_OFF);
    }
    
    if (vrb.multicast_en) {
        multicast_group_leave(vrb.dst_ip);
    }

    osd_permanent(true);
    osd_printf("vrb.video connection lost...\n");

    // disable HDCP on AD9889
    hoi_drv_hdcp_adv9889dis();

    hoi_drv_set_led_status(SDI_OUT_OFF);

    return RTSP_SUCCESS;
}


int vrb_video_error(t_rtsp_media *media, intptr_t m, t_rtsp_connection* rsp)
{
	t_rtsp_client *client = media->creator;

    if(rsp) {
        report(" ? client failed (%d): %d - %s", m, rsp->ecode, rsp->ereason);
        osd_permanent(true);
        osd_printf("vrb.video streaming could not be established: %d - %s\n", rsp->ecode, rsp->ereason);
        switch(rsp->ecode) {
            case 300:   media->result = RTSP_RESULT_SERVER_TRY_LATER;
                        break;
            case 404:   media->result = RTSP_RESULT_SERVER_BUSY;
                        break;
            case 405:   media->result = RTSP_RESULT_SERVER_NO_VTB;
                        break;
            case 406:   media->result = RTSP_RESULT_SERVER_NO_VIDEO_IN;
                        break;
            case 408:   media->result = RTSP_RESULT_SERVER_HDCP_ERROR;
                        rtsp_client_set_teardown(client);
                        break;
            case RTSP_STATUS_INTERNAL_SERVER_ERROR:
            default:    media->result = RTSP_RESULT_SERVER_ERROR;
                        break;
        }
    } else {    
        osd_permanent(true);
        osd_printf("vrb.video streaming could not be established: connection refused\n");
        media->result = RTSP_RESULT_CONNECTION_REFUSED;
    }
    return RTSP_SUCCESS;
}


void vrb_video_pause(t_rtsp_media *media)
{
	report(INFO "vrb_video_pause");
    media->result = RTSP_RESULT_PAUSE_Q;

    report(VRB_METHOD "vrb_video_pause");

    if (hdoipd_tstate(VTB_VIDEO)) {
    report(INFO "vrb_video_pause: VTB_VIDEO");
#ifdef VID_OUT_PATH
        hdoipd_hw_reset(DRV_RST_VID_OUT);
#endif
#ifdef ETI_PATH
        hoi_drv_eti(vrb.dst_ip, vrb.remote.address, 0, hdoipd.local.vid_port, 0);
#endif

        // disable HDCP on AD9889
        hoi_drv_hdcp_adv9889dis();

        hdoipd_clr_rsc(RSC_VIDEO_OUT|RSC_OSD|RSC_VIDEO_SYNC);
        hdoipd_set_vtb_state(VTB_VID_IDLE);
    }

    hoi_drv_set_led_status(SDI_OUT_OFF);
}

int vrb_video_ext_pause(t_rtsp_media* media, void *m UNUSED, t_rtsp_connection *rsp UNUSED)
{
    vrb_video_pause(media);

    return RTSP_SUCCESS;
}

int vrb_video_update(t_rtsp_media *media, t_rtsp_req_update *m, t_rtsp_connection UNUSED *rsp)
{
	//report(INFO "EVENT NR: %08x", m->event);
    switch (m->event) {

        case EVENT_TICK:
            // reset timeout
            vrb.timeout = 0;
        break;
        case EVENT_VIDEO_IN_ON:
            // No multicast for now...simply stop before starting new
        	report(INFO "vrb_video_update; EVENT_VIDEO_IN_ON");
            if (!rtsp_media_splaying(media)) {
                vrb_video_pause(media);
                // restart
                rtsp_client_set_play(media->creator);
                return RTSP_PAUSE;
            }
        break;
        case EVENT_VIDEO_IN_OFF:
        	report(INFO "vrb_video_update; EVENT_VIDEO_IN_OFF");
            vrb_video_pause(media);
            osd_permanent(true);
            osd_printf("vtb.video stopped streaming - no video input\n");
            report(ERROR "vtb.video stopped streaming - no video input");
            return RTSP_PAUSE;
        break;
    }

    return RTSP_SUCCESS;
}

int vrb_video_ready(t_rtsp_media UNUSED *media)
{
    //if (hdoipd_tstate(VTB_VID_MASK)) {
    //    // not ready ->
    //    return RTSP_ERRORNO;
    //}
    if (!hdoipd_rsc(RSC_VIDEO_SINK)) {
        // no video sink
        return RTSP_ERRORNO;
    }
    return RTSP_SUCCESS;
}

int vrb_video_dosetup(t_rtsp_media *media, void *m UNUSED, void *rsp UNUSED)
{
    int port;
    t_rtsp_transport transport;
    t_rtsp_edid edid;
    t_rtsp_client *client = media->creator;
    t_rtsp_hdcp hdcp;

    hdcp.hdcp_on = reg_test("hdcp-force", "true");

    report(VRB_METHOD "vrb_video_dosetup");

    if (!client) return RTSP_NULL_POINTER;

    rtsp_default_transport(&transport); // TODO: daemon transport configuration
    port = reg_get_int("video-port");
    transport.client_port = PORT_RANGE(htons(port), htons(port+1));

    edid.segment = 0;
    hoi_drv_rdedid(edid.edid);
#ifdef REPORT_EDID
    edid_report((void*)edid.edid);
#endif

    return rtsp_client_setup(client, media, &transport, &edid, &hdcp);
}

int vrb_video_doplay(t_rtsp_media *media, void* m UNUSED, void *rsp UNUSED)
{
    char *s;
    t_rtsp_rtp_format fmt;
    t_rtsp_client *client = media->creator;

    report(VRB_METHOD "vrb_video_doplay");

    if (!client) return RTSP_NULL_POINTER;

    // open media
    s = reg_get("compress");
    if (strcmp(s, "jp2k") == 0) {
        fmt.compress = FORMAT_JPEG2000;
    } else {
        fmt.compress = FORMAT_PLAIN;
    }
    fmt.rtptime = 0;
    fmt.value = reg_get_int("advcnt-min");

    return rtsp_client_play(client, &fmt, media->name);
}

int vrb_video_event(t_rtsp_media *media, uint32_t event)
{
    int ret;
    t_rtsp_client *client = media->creator;

    switch (event) {
        case EVENT_VIDEO_SINK_ON:
            if (!rtsp_media_sinit(media))
                return RTSP_WRONG_STATE;

            ret = rtsp_media_setup(media);
            if (ret == RTSP_SUCCESS)
              rtsp_media_play(media);
            else {
                report(ERROR "vrb_video_event() rtsp_media_setup failed (%d)", ret);
                rtsp_client_set_kill(client);
                return ret;
            }
            break;

        case EVENT_VIDEO_SINK_OFF:
            if (rtsp_media_sinit(media))
                return RTSP_WRONG_STATE;

            // Note: after teardown call media/client is not anymore a valid struct
            rtsp_client_set_teardown(client);
            break;

    }

    return RTSP_SUCCESS;
}

t_rtsp_media vrb_video = {
    .name = "video",
    .owner = 0,
    .cookie = 0,
    .options = (frtspm*)box_sys_options,
    .describe = vrb_video_describe,
    .setup = (frtspm*)vrb_video_setup,
    .play = (frtspm*)vrb_video_play,
    .pause = (frtspm*)vrb_video_ext_pause,
    .teardown = (frtspm*)vrb_video_teardown,
    .error = (frtspm*)vrb_video_error,
    .update = (frtspm*)vrb_video_update,
    .ready = (frtspl*)vrb_video_ready,
    .dosetup = (frtspm*)vrb_video_dosetup,
    .doplay = (frtspm*)vrb_video_doplay,
    .event = (frtspe*)vrb_video_event
};

