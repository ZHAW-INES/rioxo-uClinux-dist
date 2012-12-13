/*
 * hdoipd_task.c
 *
 *  Created on: 07.12.2010
 *      Author: alda
 */

#include "hdoipd.h"
#include "hdoipd_osd.h"
#include "hdoipd_fsm.h"
#include "hoi_cfg.h"
#include "hoi_drv_user.h"
#include "multicast.h"
#include "rtsp_client.h"
#include "rtsp_listener.h"
#include "rtsp_media.h"
#include "testimage.h"
#include "usb.h"
#include "vrb_audio.h"
#include "vrb_video.h"

// local buffer
static char buf[256];
static uint32_t update_vector = 0;

enum {
	HOID_TSK_UPD_SYS_IP 		= 0x00000001,
	HOID_TSK_UPD_SYS_SUBNET 	= 0x00000001,
	HOID_TSK_UPD_SYS_GATEWAY 	= 0x00000002,
	HOID_TSK_UPD_SYS_MAC 		= 0x00000004,
	HOID_TSK_UPD_SYS_DNS        = 0x00000008,
	HOID_TSK_UPD_REMOTE_URI 	= 0x00000010,
	HOID_TSK_UPD_MODE_START 	= 0x00000040,
	HOID_TSK_UPD_AUTO_STREAM    = 0x00000080,
	HOID_TSK_UPD_AMX            = 0x00000100,
    HOID_TSK_UPD_ALIVE          = 0x00000200,
    HOID_TSK_UPD_MULTICAST      = 0x00000400,
	HOID_TSK_UPD_DHCP           = 0x00000800,
	HOID_TSK_UPD_USB_MODE       = 0x00001000,
	HOID_TSK_EXEC_GOTO_READY	= 0x01000000,
	HOID_TSK_EXEC_START			= 0x02000000,
	HOID_TSK_EXEC_RESTART		= 0x03000000,
	HOID_TSK_EXEC_RESTART_VRB   = 0x10000000,
	HOID_TSK_EXEC_RESTART_VTB   = 0x20000000
};

char* task_conv_rtsp_state(int state)
{
    switch(state) {
        case RTSP_RESULT_IDLE                   : return "idle";
        case RTSP_RESULT_READY                  : return "ready";
        case RTSP_RESULT_PLAYING                : return "playing";
        case RTSP_RESULT_TEARDOWN               : return "teardown";
        case RTSP_RESULT_TEARDOWN_Q             : return "teardown_q";
        case RTSP_RESULT_PAUSE                  : return "pause";
        case RTSP_RESULT_PAUSE_Q                : return "pause_q";
        case RTSP_RESULT_NO_VIDEO_IN            : return "no video in";
        case RTSP_RESULT_SERVER_ERROR           : return "server error";
        case RTSP_RESULT_SERVER_BUSY            : return "server busy";
        case RTSP_RESULT_SERVER_NO_VTB          : return "server no vtb";
        case RTSP_RESULT_SERVER_TRY_LATER       : return "server try later";
        case RTSP_RESULT_SERVER_NO_VIDEO_IN     : return "server no video in";
        default                                 :
        case RTSP_RESULT_CONNECTION_REFUSED     : return "connection refused";
    }
}

char *buf_ptr;
void task_stream_printer(char UNUSED *key, char* value, void UNUSED *data)
{
  t_rtsp_media *media = NULL;
  if (value == NULL)
    return;

  media = (t_rtsp_media*)value;
  if (media->name == NULL || strlen(media->name) == 0)
    return;

  buf_ptr += sprintf(buf_ptr, "stream %02d {in} : %s (%s)\n", hdoipd.client->nr, task_conv_rtsp_state(media->result), media->name);
}

void task_get_drivers(char** p)
{
    char *tmp = buf;

    if (hdoipd.drivers & DRV_ADV9889) tmp += sprintf(tmp, "ADV9889 ");
    if (hdoipd.drivers & DRV_ADV7441) tmp += sprintf(tmp, "ADV7441 ");
    if (hdoipd.drivers & DRV_GS2971)  tmp += sprintf(tmp, "GS2971 ");
    if (hdoipd.drivers & DRV_GS2972)  tmp += sprintf(tmp, "GS2972 ");

    if (tmp!=buf) tmp--;
    *tmp = 0;
    *p = buf;
}

void task_get_system_ip(char** p)
{
    if(hdoipd.dhcp) {
        hoi_cfg_get_ip(buf);
        *p = buf;
    } else {
        *p = reg_get("system-ip");
    }
}

void task_get_system_gateway(char** p)
{
    if(hdoipd.dhcp) {
        hoi_cfg_get_default_gw(buf);
        *p = buf;
    } else {
        *p = reg_get("system-gateway");
    }
}

void task_get_system_subnet(char** p)
{
    if(hdoipd.dhcp) {
        hoi_cfg_get_subnet(buf);
        *p = buf;
    } else {
        *p = reg_get("system-subnet");
    }
}

void task_get_system_dns1(char** p)
{
    if(hdoipd.dhcp) {
        hoi_cfg_get_dns_server(buf, NULL);
        *p = buf;
    } else {
        *p = reg_get("system-dns1");
    }
}

void task_get_system_dns2(char** p)
{
    if(hdoipd.dhcp) {
        hoi_cfg_get_dns_server(NULL, buf);
        *p = buf;
    } else {
        *p = reg_get("system-dns2");
    }
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

void task_get_multicast_client(char ** p)
{
    multicast_client_report();
    sprintf(buf, "multicast enable: %i", (int) multicast_get_enabled());
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

void task_get_aso_status(char** p)
{
    t_hoi_msg_asoreg *stat;
    char * tmp = buf;

    hoi_drv_asoreg(&stat);

    tmp += sprintf(buf, "config: 0x%08x, status: 0x%08x\n",stat->config, stat->status);
    tmp += sprintf(tmp, " start = 0x%08x\n", stat->start);
    tmp += sprintf(tmp, " stop  = 0x%08x\n", stat->stop);
    tmp += sprintf(tmp, " read  = 0x%08x\n", stat->read);
    tmp += sprintf(tmp, " write = 0x%08x\n", stat->write);

    *p = buf;
}

void task_get_hdcp_status(char** p)
{
    if (reg_test("hdcp-force", "true") || hdoipd_rsc(RSC_VIDEO_IN_HDCP) || (hdoipd.hdcp.enc_state != 0)) {
        sprintf(buf, "encrypted");
    } else {
        sprintf(buf, "not encrypted");
    }

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

void task_get_system_update(char** p)
{
	int state;
	bool streaming = false;
	if(update_vector != 0) {

		// -------------------------------------------------------------
		// System commands (before update)

		/* save actual state */
		state = hdoipd.state;

		/* goto ready */
		if((update_vector & HOID_TSK_EXEC_GOTO_READY) ||
		   ((update_vector & HOID_TSK_EXEC_RESTART_VRB) && (state == HOID_VRB)) ||
		   ((update_vector & HOID_TSK_EXEC_RESTART_VTB) && (state == HOID_VTB))) {

			if (hdoipd_state(HOID_VRB|HOID_VTB)) {
				report("set device into ready state...");
				streaming = vrb_video.state == RTSP_STATE_PLAYING || vrb_audio.state == RTSP_STATE_PLAYING;
				hdoipd_goto_ready();
	    	}
		}

		// -------------------------------------------------------------
		// Parameter specific commands

        // USB mode (TODO: linux crashs if driver are loaded during stream is running)
	//	if(update_vector & HOID_TSK_UPD_USB_MODE) {
    //        report("Updating USB mode...");
    //        s = reg_get("usb-mode");
    //        usb_load_driver(s);
    //    }

		/* MAC address */
		if(update_vector & HOID_TSK_UPD_SYS_MAC) {
			report("Updating MAC address...");
			hoi_cfg_mac_addr(reg_get("system-ifname"), reg_get("system-mac"));
		}

		// DHCP flag
        if((update_vector & HOID_TSK_UPD_DHCP) == 0) {
            /* IP & subnet mask */
            if((update_vector & HOID_TSK_UPD_SYS_IP) ||
               (update_vector & HOID_TSK_UPD_SYS_SUBNET)) {
                report("Updating IP & subnet mask...");
                hoi_cfg_ip_addr(reg_get("system-ifname"), reg_get("system-ip"), reg_get("system-subnet"));
            }

            /* default gateway */
            if(update_vector & HOID_TSK_UPD_SYS_GATEWAY) {
                report("Updating Gateway...");
                hoi_cfg_default_gw(reg_get("system-gateway"));
            }

            /* DNS server */
            if(update_vector & HOID_TSK_UPD_SYS_DNS) {
                hoi_cfg_set_dns_server(reg_get("system-dns1"), reg_get("system-dns2"));
            }
        } else { // Update DHCP mode
            hdoipd.dhcp = reg_test("system-dhcp", "true");
            if(hdoipd.dhcp) {
                hoi_cfg_dynamic_ip();
            } else {
                hoi_cfg_static_ip();
            }
        }

		/* remote URI  */
		if(update_vector & HOID_TSK_UPD_REMOTE_URI) {
			report("Updating remote URI...");
		}

		/* AMX update */
		if(update_vector & HOID_TSK_UPD_AMX) {
		    report("Updating AMX...");
            if(alive_check_client_update(&(hdoipd.amx), reg_test("amx-en", "true"), reg_get_int("amx-hello-interval"), reg_get("amx-hello-ip"), reg_get_int("amx-hello-port"), 1, true) == -1) {
                report("[AMX] alive_check_client_update() failed");
            }
		}

		/* Auto-stream feature */
		if(update_vector & HOID_TSK_UPD_AUTO_STREAM) {
		    report("Updating auto-stream flag...");
		    hdoipd.auto_stream = reg_test("auto-stream", "true");
		}

		/* device mode */
		if(update_vector & HOID_TSK_UPD_MODE_START) {
			report("Updating device modus...");
		}

        /* multicast */
        if(update_vector & HOID_TSK_UPD_MULTICAST) {
		    report("Updating multicast...");
        }

		// -------------------------------------------------------------
		// System commands (after update)

		/* start device */
		if((update_vector & HOID_TSK_EXEC_START) ||
		   ((update_vector & HOID_TSK_EXEC_RESTART_VRB) && (state == HOID_VRB)) ||
		   ((update_vector & HOID_TSK_EXEC_RESTART_VTB) && (state == HOID_VTB))) {

			report("start device now...");

			switch(state) {
				case HOID_VTB : hdoipd_goto_vtb();
								break;
				case HOID_VRB : hdoipd_goto_vrb();
				                hdoipd_start_vrb(streaming || hdoipd.auto_stream);
								break;
				default 	  :
								break;
			}
		}

		update_vector = 0;
		sprintf(buf, "System parameters updated!");
	} else {
		sprintf(buf, "Nothing to update!");
	}

	*p = buf;
}

void task_get_rtsp_medias(char *key UNUSED, char* value, void* fd)
{
    int *cnt = (int *) fd;

    if (value == NULL || fd == NULL)
      return;

    t_rtsp_media* media = (t_rtsp_media*)value;
    if (media->name == NULL || strlen(media->name) == 0)
      return;

    buf_ptr += sprintf(buf_ptr, "stream %02d {out}: %s (%s)\n", *cnt, task_conv_rtsp_state(media->result), media->owner->name);
    *cnt += 1;
}

void task_get_rtsp_sessions(char *key UNUSED, char* value, void* fd)
{
    if (value == NULL || fd == NULL)
        return;

    t_rtsp_server* server = (t_rtsp_server*) value;
    if (server->media != NULL)
      bstmap_traverse(server->media, task_get_rtsp_medias, fd);
}

void task_get_rtsp_state(char** s)
{
    t_video_timing vid_timing;
    uint32_t advcnt;
    uint32_t image_freq, image_pixel;
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



    if (hdoipd.state & HOID_VTB) {
        if (hdoipd.rsc_state & RSC_VIDEO_IN) {
            hoi_drv_info(&vid_timing, &advcnt, 0);
            image_pixel = (vid_timing.height + vid_timing.vfront + vid_timing.vback + vid_timing.vpulse) * (vid_timing.width + vid_timing.hfront + vid_timing.hback + vid_timing.hpulse);
            image_freq = (vid_timing.pfreq / (image_pixel / 100));
            buf_ptr += sprintf(buf_ptr, "resolution     : %d x %d @ %d.%02d Hz / %d MHz\n", vid_timing.width, vid_timing.height, (image_freq/100), (image_freq%100), (vid_timing.pfreq/1000000));
        } else {
            buf_ptr += sprintf(buf_ptr, "resolution     : (no input)\n");
        }
    } else {
        buf_ptr += sprintf(buf_ptr, "resolution     : (only visible at transmitter box)\n");
    }

    // print all the active streams
    if (hdoipd.client != NULL)
        bstmap_traverse(hdoipd.client->media, task_stream_printer, NULL);

    if(hdoipd.listener.sessions) {
        unlock("task_get_stream_state");
        rtsp_listener_session_traverse(&hdoipd.listener, task_get_rtsp_sessions, &cnt);
        lock("task_get_stream_state");
    }

    *s = buf;
}

void task_get_vrb_is_playing(char** p)
{
    *p = "false";
    switch(vrb_video.state) {
        case RTSP_STATE_PLAYING:  *p = "true";
                            break;
    }

    switch(vrb_audio.state) {
        case RTSP_STATE_PLAYING:  *p = "true";
                            break;
    }
}

int task_ready(void)
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

void task_set_bw(char *p UNUSED)
{
    t_video_timing timing;
    uint32_t advcnt_old;
    uint32_t bw     = reg_get_int("bandwidth");             // bandwidth in byte/s
    uint32_t chroma = reg_get_int("chroma-bandwidth");      // percent of chroma bandwidth (0 .. 100)
    report("update bandwidth: %d Byte/s %d%% Chroma", bw, chroma);
    //bw = bw - bw / 20; // 5% overhead approx.

    // get video timing
    if (hoi_drv_info(&timing, 0, &advcnt_old)) {
        report("cant get video timing");
    }

    // use 4 ADV212 if 1080i and bandwidth >= 50Mbit/s
    if ((timing.width == 1920) && (timing.height == 540) && (bw >= (uint32_t)((uint64_t)(25+25*chroma/100)*(1048576)/8))) {
        if (advcnt_old != 4) {
            update_vector |= HOID_TSK_EXEC_RESTART_VTB;
            return;
        }
    }

    // use 2 ADV212 if 1080i and bandwidth < 50Mbit/s
    if ((timing.width == 1920) && (timing.height == 540) && (bw < (uint32_t)((uint64_t)(25+25*chroma/100)*(1048576)/8))) {
        if (advcnt_old != 2) {
            update_vector |= HOID_TSK_EXEC_RESTART_VTB;
            return;
        }
    }

    // limit bandwidth to 120Mbit/s for 1080i
    if ((timing.width == 1920) && (timing.height == 540) && (bw > (uint32_t)((uint64_t)(60+60*chroma/100)*(1048576)/8))) {
        bw = (uint32_t)((uint64_t)(60+60*chroma/100)*(1048576)/8);
    }

    // limit bandwidth to 50Mbit/s for 576i and 480i
    if ((timing.width == 720) && ((timing.height == 240) || (timing.height == 243) || (timing.height == 244) || (timing.height == 288)) && (bw > (uint32_t)((uint64_t)(25+25*chroma/100)*(1048576)/8))) {
        bw = (uint32_t)((uint64_t)(25+25*chroma/100)*(1048576)/8);
    }

    if (bw) hoi_drv_bw(bw, chroma);
}

void task_set_traffic_shaping(char* p UNUSED)
{
    update_vector |= HOID_TSK_EXEC_RESTART_VTB;
}

void task_set_system_dns1(char *p UNUSED)
{
    if(!hdoipd.dhcp) {
        update_vector |= HOID_TSK_UPD_SYS_DNS | HOID_TSK_EXEC_RESTART | HOID_TSK_UPD_AMX;
    }
}

void task_set_system_dns2(char *p UNUSED)
{
    if(!hdoipd.dhcp) {
        update_vector |= HOID_TSK_UPD_SYS_DNS | HOID_TSK_EXEC_RESTART | HOID_TSK_UPD_AMX;
    }
}

void task_set_ip(char *p UNUSED)
{
    if(!hdoipd.dhcp) {
        update_vector |= HOID_TSK_UPD_SYS_IP | HOID_TSK_EXEC_RESTART | HOID_TSK_UPD_AMX;
    }
}

void task_set_subnet(char *p UNUSED)
{
    if(!hdoipd.dhcp) {
        update_vector |= HOID_TSK_UPD_SYS_SUBNET | HOID_TSK_UPD_AMX;
    }
}

void task_set_gateway(char *p UNUSED)
{
    if(!hdoipd.dhcp) {
        update_vector |= HOID_TSK_UPD_SYS_GATEWAY | HOID_TSK_UPD_AMX;
    }
}

void task_set_mac(char *p UNUSED)
{
	update_vector |= HOID_TSK_UPD_SYS_MAC | HOID_TSK_EXEC_RESTART | HOID_TSK_UPD_AMX;
}

void task_set_remote(char *p UNUSED)
{
	update_vector |= HOID_TSK_UPD_REMOTE_URI | HOID_TSK_EXEC_RESTART_VRB;
}

void task_set_mode_start(char *p UNUSED)
{
	update_vector |= HOID_TSK_UPD_MODE_START; //    | HOID_TSK_EXEC_RESTART;  restart not necessary because device must be rebooted after this change
}

void task_set_mode_media(char *p UNUSED)
{
	update_vector |= HOID_TSK_EXEC_RESTART_VRB;
}

void task_set_amx_update(char *p UNUSED)
{
    update_vector |= HOID_TSK_UPD_AMX;
}

void task_set_auto_stream(char *p UNUSED)
{
    update_vector |= HOID_TSK_UPD_AUTO_STREAM | HOID_TSK_EXEC_RESTART_VRB;
}

void task_set_multicast_update(char *p UNUSED)
{
    update_vector |= HOID_TSK_UPD_MULTICAST | HOID_TSK_EXEC_RESTART_VTB;
}

void task_set_led_instruction(char* p)
{
    hoi_drv_set_led_status(atoi(p));
}

void task_set_osd_time(char* p)
{
    report("osd-time: %d", atoi(p));
    if (atoi(p) == 100) {
        hoi_drv_osdon();
        hdoipd.rsc_state |= RSC_OSD;
    } else {
        hoi_drv_osdoff();
        hdoipd.rsc_state &= ~RSC_OSD;
    }
}

void task_set_usb_mode(char *p UNUSED)
{
    update_vector |= HOID_TSK_UPD_USB_MODE;
}

void task_set_test_image(char *p)
{
    int a = atoi(p);
    if reg_test("mode-start", "vrb") {
        switch (a) {
            case 1:     hdoipd_goto_ready();
                        osd_printf_testpattern_focus_1080p60();
                        break;
            case 2:     hdoipd_goto_ready();
                        osd_printf_testpattern_focus_1080p24();
                        break;
            case 3:     hdoipd_goto_ready();
                        osd_printf_testpattern_focus_720p60();
                        break;
            case 4:     testimage_show();
                        break;
            default:    report(ERROR "test-image: no valid number");
        }
    }
    reg_set("test-image", "0");
}

void task_set_network_delay(char *p UNUSED)
{
    update_vector |= HOID_TSK_EXEC_RESTART_VRB;
}

void task_set_av_delay(char *p UNUSED)
{
    update_vector |= HOID_TSK_EXEC_RESTART_VRB;
}

void task_set_dhcp(char *p UNUSED)
{
    update_vector |= HOID_TSK_EXEC_RESTART | HOID_TSK_UPD_AMX | HOID_TSK_UPD_DHCP;
}

void task_set_edid_mode(char *p UNUSED)
{
    rtsp_listener_teardown_all(&hdoipd.listener);
    rtsp_listener_close_all(&hdoipd.listener);
}

void task_set_fps_divide(char *p UNUSED)
{
    hoi_drv_set_fps_reduction(reg_get_int("fps_divide"));
}

void hdoipd_register_task()
{
    get_listener("system-state", task_get_system_state);
    get_listener("system-update", task_get_system_update);
    get_listener("system-ip", task_get_system_ip);
    get_listener("system-subnet", task_get_system_subnet);
    get_listener("system-gateway", task_get_system_gateway);
    get_listener("system-dns1", task_get_system_dns1);
    get_listener("system-dns2", task_get_system_dns2);
    get_listener("daemon-driver", task_get_drivers);
    get_listener("daemon-state", task_get_state);
    get_listener("daemon-vtb-state", task_get_vtb_state);
    get_listener("daemon-vrb-state", task_get_vrb_state);
    get_listener("daemon-rsc-state", task_get_rsc_state);
    get_listener("eth-status", task_get_eth_status);
    get_listener("vso-status", task_get_vso_status);
    get_listener("vio-status", task_get_vio_status);
    get_listener("aso-status", task_get_aso_status);
    get_listener("hdcp-status", task_get_hdcp_status);
    get_listener("sync-delay", task_get_sync_delay);
    get_listener("stream-state", task_get_rtsp_state);
    get_listener("multicast", task_get_multicast_client);
    get_listener("vrb_is_playing", task_get_vrb_is_playing);

    // set-listener are called when a new value is written to the register
    // if the same value is written as already stored the listener isn't called
    set_listener("bandwidth", task_set_bw);
    set_listener("chroma-bandwidth", task_set_bw);
    set_listener("system-ip", task_set_ip);
    set_listener("system-subnet", task_set_subnet);
    set_listener("system-gateway", task_set_gateway);
    set_listener("system-mac", task_set_mac);
    set_listener("system-dhcp", task_set_dhcp);
    set_listener("system-hostname", task_set_dhcp);
    set_listener("system-dns1", task_set_system_dns1);
    set_listener("system-dns2", task_set_system_dns2);
    set_listener("network-delay", task_set_network_delay);
    set_listener("av-delay", task_set_av_delay);
    set_listener("mode-start", task_set_mode_start);
    set_listener("mode-media", task_set_mode_media);
    set_listener("remote-uri", task_set_remote);
    set_listener("auto-stream", task_set_auto_stream);
    set_listener("amx-en", task_set_amx_update);
    set_listener("amx-hello-ip", task_set_amx_update);
    set_listener("amx-hello-port", task_set_amx_update);
    set_listener("amx-hello-interval", task_set_amx_update);
    set_listener("multicast_en", task_set_multicast_update);
    set_listener("multicast_group", task_set_multicast_update);
    //set_listener("alive-check", task_set_alive_update);
    //set_listener("alive-check-interval", task_set_alive_update);
    //set_listener("alive-check-port", task_set_alive_update);
    set_listener("led_instruction", task_set_led_instruction);
    set_listener("osd-time", task_set_osd_time);
    set_listener("usb-mode", task_set_usb_mode);
    set_listener("test-image", task_set_test_image);
    set_listener("edid-mode", task_set_edid_mode);
    set_listener("fps_divide", task_set_fps_divide);;
}

