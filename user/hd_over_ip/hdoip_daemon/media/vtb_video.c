/*
 * vtb_video.c
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */

#include "hdoipd.h"
#include "hoi_drv_user.h"
#include "vtb_video.h"
#include "hdoipd_fsm.h"
#include "box_sys.h"
#include "edid.h"
#include "rscp_server.h"
#include "hdcp.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


#define TICK_TIMEOUT                    (hdoipd.eth_timeout)
#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

static struct {
    t_hdoip_eth         remote;
    // this is per connection (must be changed for multicast)
    int                 timeout;
    int                 alive_ping;
} vtb;

int vtb_video_hdcp(t_rscp_media* media, t_rscp_req_hdcp* m, t_rscp_connection* rsp)
{
    hdcp_ske_s(media, m, rsp);

  /*  report(INFO "ID: %s CONTENT: %s ###########",m->id, m->content);
	int message = atoi(m->id);
	printf("The converted string: %d",message);
	char message1[] ="Message 1 ret";
	char message2[] ="Message 2 ret";
	char ret[100];
	switch (message) {
	        case HDCP_START:
	        	generate_rtx(&ret);
	        	strcpy(hdoipd.hdcp.rtx, ret);
	        	//TODO: set and check state
	            rscp_response_hdcp(rsp, media->sessionid, m->id, &ret); //respond to setup message
	            report(INFO "hdcp.rtx1: %s", hdoipd.hdcp.rtx);
	            return RSCP_SUCCESS;
	        break;
	        case HDCP_AKE_SEND_CERT:

	            rscp_response_hdcp(rsp, media->sessionid, m->id, &message2); //respond message to setup message
	            report(INFO "hdcp.rtx2: %s", hdoipd.hdcp.rtx);
	            return RSCP_SUCCESS;
	        break;
	}


    //rscp_response_hdcp(rsp, media->sessionid, m->id, m->content); //respond message to setup message
    //return RSCP_SUCCESS;*/
}

int vtb_video_setup(t_rscp_media* media, t_rscp_req_setup* m, t_rscp_connection* rsp)
{
    int n;
    char *p;
    int sockfd;                   // for hdcp socket
    struct sockaddr_in cli_addr;  // for hdcp socket

    //hdcp information should be ready in m

    report(INFO "vtb_video_setup");
    
    media->result = RSCP_RESULT_READY;

    if (!hdoipd_state(HOID_VTB)) {
        // currently no video input active
        report(" ? not in state vtb");
        rscp_err_no_vtb(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // test if resource is available
    // currently no multicast support -> video must be unused
    if (hdoipd_tstate(VTB_VID_MASK)) {
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

    // limit incoming edid
    // TODO: ...
    edid_report((void*)m->edid.edid);

    if (!hdoipd_tstate(VTB_VID_MASK)) {
        // TODO: dont reload when already the same, store edid in file for next boot
        //       have a list of all contributing edid source (to test if it is allready included in our edid)
        //
        // video source not  in use -> update
        hoi_drv_hpdoff();
        hoi_drv_wredid(m->edid.edid);
        hoi_drv_hpdon();
    }
    
    // reserve resource
    hdoipd_set_vtb_state(VTB_VID_IDLE);

    vtb.timeout = 0;
    vtb.alive_ping = 1;

    // open hdcp socket if necessary
   /* if ((n = hdcp_open_socket(&m->hdcp, &sockfd, &cli_addr)) != RSCP_SUCCESS){
        report(" ? Could not open hdcp socket");
        rscp_err_hdcp(rsp);
        return RSCP_REQUEST_ERROR;
    }*/

    vtb.remote.address = rsp->address;
    vtb.remote.vid_port = PORT_RANGE_START(m->transport.client_port);
    m->transport.server_port = PORT_RANGE(hdoipd.local.vid_port, hdoipd.local.vid_port);
    rscp_response_setup(rsp, &m->transport, media->sessionid, &m->hdcp); //respond message to setup message

    // start hdcp session key exchange if necessary
   /* char video[]="video";
    if ((n = hdcp_ske_server(&m->hdcp, &sockfd, &cli_addr, &video)) != RSCP_SUCCESS){
            report(" ? Session key exchange failed");
            rscp_err_hdcp(rsp);
            return RSCP_REQUEST_ERROR;
    }*/

    REPORT_RTX("TX", hdoipd.local, "->", vtb.remote, vid);

    return RSCP_SUCCESS;
}

int vtb_video_play(t_rscp_media* media, t_rscp_req_play* m, t_rscp_connection* rsp)
{
    int n;
    t_video_timing timing;
    t_rscp_rtp_format fmt;
    hdoip_eth_params eth;

    report(INFO "vtb_video_play");

    media->result = RSCP_RESULT_PLAYING;

    if (!hdoipd_state(HOID_VTB)) {
        // we don't have the resource reserved
        report(" ? require state VTB");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    // no multicast currently -> must be idle
    if (!hdoipd_tstate(VTB_VID_IDLE)) {
        // we don't have the resource reserved
        report(" ? require state VTB_IDLE");
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    if (!hdoipd_rsc(RSC_VIDEO_IN)) {
        // currently no video input active
        report(" ? require active video input");
        media->result = RSCP_RESULT_NO_VIDEO_IN;
        rscp_err_no_source(rsp);
        return RSCP_REQUEST_ERROR;
    }

    //check if Video HDCP status is unchanged, else return ERROR
    report("TEST HDCP before video play!!!!!!!!!");
    if ((reg_test("hdcp-force", "on") || hdoipd_rsc(RSC_VIDEO_IN_HDCP)) && !(get_hdcp_status() & HDCP_ETO_VIDEO_EN)) {
        report(" ? Video HDCP status has changed after SKE");
       /* rscp_err_hdcp(rsp);
        return RSCP_REQUEST_ERROR;*/
    }
    // if hdoipd_rsc(RSC_VIDEO_IN_HDCP) || hdcp is active -> error


    // test for valid resolution

    // start ethernet output
    eth.ipv4_dst_ip = vtb.remote.address;
    eth.ipv4_src_ip = hdoipd.local.address;
    eth.ipv4_tos = 0;
    eth.ipv4_ttl = 30;
    eth.packet_size = 1500;
    eth.rtp_ssrc = 0;
    for(n=0;n<6;n++) eth.src_mac[n] = hdoipd.local.mac[n];
    for(n=0;n<6;n++) eth.dst_mac[n] = vtb.remote.mac[n];
    eth.udp_dst_port = vtb.remote.vid_port;
    eth.udp_src_port = hdoipd.local.vid_port;

    // start vsi
    fmt.compress = m->format.compress;
    fmt.value = reg_get_int("advcnt-min");

    // probe config
    if ((n = hoi_drv_info(&timing, &fmt.value))) {
        rscp_err_server(rsp);
        return RSCP_REQUEST_ERROR;
    }

    hoi_drv_get_mtime(&fmt.rtptime);

    // send timing
    rscp_response_play(rsp, media->sessionid, &fmt, &timing);

#ifdef VID_IN_PATH
    // activate vsi
    if ((n = hoi_drv_vsi(fmt.compress, 0, reg_get_int("bandwidth"), &eth, &timing, &fmt.value))) {
        return RSCP_REQUEST_ERROR;
    }
#endif

    // We are streaming Video now...
    hdoipd_set_vtb_state(VTB_VIDEO);

    return RSCP_SUCCESS;
}

int vtb_video_teardown(t_rscp_media* media, t_rscp_req_teardown *m, t_rscp_connection* rsp)
{
    report(INFO "vtb_video_teardown");

    media->result = RSCP_RESULT_TEARDOWN;

    if (hdoipd_tstate(VTB_VIDEO|VTB_VID_IDLE)) {
#ifdef VID_IN_PATH
        hdoipd_hw_reset(DRV_RST_VID_IN);
#endif
        hdoipd_set_vtb_state(VTB_VID_OFF);
    }

    if (rsp) {
        rscp_response_teardown(rsp, media->sessionid);
    }

    vtb.timeout = 0;
    vtb.alive_ping = 1;

    return RSCP_SUCCESS;
}

void vtb_video_pause(t_rscp_media *media)
{
    report(INFO "vtb_video_pause");

    media->result = RSCP_RESULT_PAUSE_Q;

    if (hdoipd_tstate(VTB_VIDEO)) {
#ifdef VID_IN_PATH
        hdoipd_hw_reset(DRV_RST_VID_IN);
#endif
        hdoipd_set_vtb_state(VTB_VID_IDLE);
    }
}

int vtb_video_update(t_rscp_media UNUSED *media, t_rscp_req_update *m, t_rscp_connection UNUSED *rsp)
{
    switch (m->event) {
        case EVENT_TICK:
            // reset timeout
            vtb.timeout = 0;
        break;
    }

    return RSCP_SUCCESS;
}

int vtb_video_event(t_rscp_media *media, uint32_t event)
{
    switch (event) {
    //	case EVENT_HDCP_ON:                                     //ADV7441 HDCP event received
    //		if (get_hdcp_status() & HDCP_ETO_VIDEO_EN) break;  //check if HDCP is already running
    //		report(INFO "ETO_VIDEO not yet enabled -> pause");
        case EVENT_VIDEO_IN_ON:
            if (rscp_media_splaying(media)) {
                vtb_video_pause(media);
                rscp_server_update(media, EVENT_VIDEO_IN_OFF);
            }
            rscp_server_update(media, EVENT_VIDEO_IN_ON);
            return RSCP_PAUSE;
        break;
        case EVENT_VIDEO_IN_OFF:
            if (rscp_media_splaying(media)) {
                vtb_video_pause(media);
                rscp_server_update(media, EVENT_VIDEO_IN_OFF);
            }
            return RSCP_PAUSE;
        break;
        case EVENT_TICK:
            if (vtb.alive_ping) {
                vtb.alive_ping--;
            } else {
                vtb.alive_ping = TICK_SEND_ALIVE;
                // send tick we are alive (until something like rtcp is used)
                rscp_server_update(media, EVENT_TICK);
            }

            if (vtb.timeout <= TICK_TIMEOUT) {
                vtb.timeout++;
            } else {
                report(INFO "vtb_video_event: timeout");
                // timeout -> kill connection
                // server cannot kill itself -> add to kill list
                // (will be executed after all events are processed)
                vtb.timeout = 0;
                rscp_listener_add_kill(&hdoipd.listener, media);
            }
        break;
    }

    return RSCP_SUCCESS;
}

t_rscp_media vtb_video = {
    .name = "video",
    .owner = 0,
    .cookie = 0,
    .hdcp = (frscpm*)vtb_video_hdcp,
    .setup = (frscpm*)vtb_video_setup,
    .play = (frscpm*)vtb_video_play,
    .teardown = (frscpm*)vtb_video_teardown,
    .update = (frscpm*)vtb_video_update,
    .event = (frscpe*)vtb_video_event
};
