/*
 * hdoipd_fsm.c
 *
 * TODO: separate audio/video/backchannel/usb tunneling on/off handling etc.
 * currently the complete hardware is switched off before a new function is started.
 * this is also true for the driver!
 *
 *  Created on: 03.11.2010
 *      Author: alda
 */

#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "box_sys.h"
#include "box_sys_vrb.h"
#include "box_sys_vtb.h"
#include "hdoipd_callback.h"
#include "hdoipd_fsm.h"
#include "hdoipd_msg.h"
#include "hdoipd_osd.h"
#include "hdoipd_task.h"
#include "hoi_cfg.h"
#include "hoi_drv_user.h"
#include "hoi_image.h"
#include "hoi_res.h"
#include "rtsp_client.h"
#include "rtsp_error.h"
#include "rtsp_string.h"
#include "usb.h"
#include "version.h"
#include "vrb_audio.h"
#include "vrb_video.h"
#include "vtb_audio.h"
#include "vtb_video.h"
#include "usb_media.h"

const char* statestr(int state)
{
    switch (state) {
        case HOID_IDLE: return "idle";
        case HOID_READY: return "ready";
        case HOID_SHOW_CANVAS: return "show";
        case HOID_LOOP: return "loop";
        case HOID_VRB: return "vrb";
        case HOID_VTB: return "vtb";
    }
    return "unknown";
}

const char* vtbstatestr(int state)
{
    static char *tmp = "(v:idle|a:idle)";
    char *p = tmp;
    if (state == VTB_OFF) {
        p += sprintf(p, "off");
    } else {
        p += sprintf(p, "(v:");
        if (state & VTB_VID_IDLE) p += sprintf(p, "idle");
        else if (state & VTB_VIDEO) p += sprintf(p, "on");
        else p += sprintf(p, "off");
        p += sprintf(p, "|a:");
        if (state & VTB_AUD_IDLE) p += sprintf(p, "idle");
        else if (state & VTB_AUDIO) p += sprintf(p, "on");
        else p += sprintf(p, "off");
        p += sprintf(p, ")");
    }
    return tmp;
}

const char* vrbstatestr(int state)
{
    switch (state) {
        case VRB_OFF: return "off";
        case VRB_IDLE: return "idle";
        case VRB_AUDIO: return "audio";
    }
    return "unknown";
}

bool hdoipd_clr_rsc(int state)
{
    bool tmp = hdoipd.rsc_state & state;
    hdoipd.rsc_state &= ~state;
    // TODO: when clearing aud/vid sync but an other stream is still running -> switch sync
    if ((state & RSC_VIDEO_SYNC) && hdoipd_rsc(RSC_AUDIO_OUT)) {
        if (reg_test("mode-sync", "streamsync")) {
            // TODO: switch sync to
            hdoipd_set_rsc(RSC_AUDIO_SYNC);
        }
    }
    if ((state & RSC_AUDIO_SYNC) && hdoipd_rsc(RSC_VIDEO_OUT) && !hdoipd_rsc(RSC_OSD)) {
        if (reg_test("mode-sync", "streamsync")) {
            // TODO: switch sync to
            hdoipd_set_rsc(RSC_VIDEO_SYNC);
        }
    }
    return tmp;
}

bool hdoipd_set_rsc(int state)
{
    bool tmp = ~hdoipd.rsc_state & state;
    if (state & RSC_SYNC) hdoipd.rsc_state = (hdoipd.rsc_state & ~RSC_SYNC) | state;
    else hdoipd.rsc_state |= state;
    return tmp;
}

bool hdoipd_set_state(int state)
{
    hdoipd.state = state;
    hdoipd.vtb_state = VTB_OFF;
    hdoipd.vrb_state = VRB_OFF;
    report(" § change state to %s", statestr(hdoipd.state));
    return true;
}

bool hdoipd_set_vtb_state(int vtb_state)
{
    if (hdoipd_state(HOID_VTB|HOID_VRB)) {
        if (vtb_state & VTB_VID_MASK) hdoipd.vtb_state = (hdoipd.vtb_state & ~VTB_VID_MASK) | vtb_state;
        else if (vtb_state & VTB_AUD_MASK) hdoipd.vtb_state = (hdoipd.vtb_state & ~VTB_AUD_MASK) | vtb_state;
        else hdoipd.vtb_state = vtb_state;
        hdoipd.vtb_state &= VTB_VALID;
        if (!hdoipd.vtb_state) hdoipd.vtb_state = VTB_OFF;
    } else {
        report(" ? tryed to change vtb_state when not in state vtb or vrb");
    }
    report(" § change state to %s/%s", statestr(hdoipd.state), vtbstatestr(hdoipd.vtb_state));
    return true;
}

bool hdoipd_set_vrb_state(int vrb_state)
{
    if (hdoipd_state(HOID_VTB|HOID_VRB)) {
        if (vrb_state & VRB_AUD_MASK) hdoipd.vrb_state = (hdoipd.vrb_state & ~VRB_AUD_MASK) | vrb_state;
        else hdoipd.vtb_state = vrb_state;
    } else {
        report(" ? tryed to change vrb_state when not in state vtb or vrb");
    }
    report(" § change state to %s/%s", statestr(hdoipd.state), vrbstatestr(hdoipd.vrb_state));
    return true;
}


/* Stops the Hardware from doing anything
 *
 * After this function is called the hardware is in off state.
 *
 * @param osd true = when writing to the osd make it permanently visibly
 * @param state new device state
 * @param vtb_state new vtb state (streaming)
 * @param vrb_state new vrb state (backchannel)
 */
void hdoipd_off(bool osd)
{
    hoi_drv_reset(DRV_RST);

    hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
    hdoipd_set_state(HOID_READY);

    osd_printf("device is idle...\n");
    osd_permanent(osd);
}

void hdoipd_hw_reset(int rv)
{
    char tmp[200], *s = tmp;
    hoi_drv_reset(rv);
    s += sprintf(s, INFO "reset: ");
    if (rv & DRV_RST_TMR) s += sprintf(s, "TIMER ");
    if (rv & DRV_RST_STSYNC) s += sprintf(s, "SYNC ");
    if (rv & DRV_RST_VID_OUT) s += sprintf(s, "Video-OUT ");
    if (rv & DRV_RST_VID_IN) s += sprintf(s, "Video-IN ");
    if (rv & DRV_RST_AUD_OUT) s += sprintf(s, "Audio-OUT ");
    if (rv & DRV_RST_AUD_IN) s += sprintf(s, "Audio-IN ");
    if (rv & DRV_RST_VRP) s += sprintf(s, "VRP ");
    report(tmp);
}

/** Goto ready state
 *
 * stop all operation and goto ready state
 *
 */
bool hdoipd_goto_ready()
{
    switch (hdoipd.state) {
        case HOID_IDLE:
            report("hw not ready");
            return false;
        break;
        case HOID_VTB:
            rtsp_listener_teardown_all(&hdoipd.listener);
            rtsp_listener_close_all(&hdoipd.listener);
            rtsp_listener_free_media(&hdoipd.listener);
            hdoipd_off(true);
        break;
        case HOID_VRB:
            // shut down VRB
            rtsp_client_teardown(hdoipd.client, NULL);
            rtsp_client_close(hdoipd.client, true);
            hdoipd.client = NULL;
            rtsp_listener_free_media(&hdoipd.listener);
            hdoipd_off(true);
        break;
        case HOID_READY:
        break;
        case HOID_LOOP:
        case HOID_SHOW_CANVAS:
            hoi_image_free(hdoipd.canvas);
            hdoipd.canvas = 0;
            hdoipd_off(true);
        break;
        default:
            report("request not supported in state %s\n", statestr(hdoipd.state));
            return false;
        break;
    }
    return true;
}

/** Force to ready state
 *
 * Do not consider ethernet state
 */
void hdoipd_force_ready()
{
    switch (hdoipd.state) {
        case HOID_VRB:
            rtsp_client_close(hdoipd.client, true);
            hdoipd.client = NULL;
        break;
        case HOID_VTB:
            rtsp_listener_close_all(&hdoipd.listener);
        break;
    }

    hdoipd_goto_ready();

    report(INFO "hdoipd_force_ready() done");
}


/* Activate VTB
 *
 * First goto ready then goto VTB. Loads all media source for RTSPX.
 *
 */
void hdoipd_goto_vtb()
{
    if (hdoipd_rsc(RSC_ETH_LINK)) {
        if (!hdoipd_state(HOID_VTB)) {
            if (hdoipd_goto_ready()) {
                report(CHANGE "goto vtb");

                // set output uses slave timer
                hoi_drv_timer(DRV_TMR_OUT);

                rtsp_listener_add_media(&hdoipd.listener, &vtb_audio);
                rtsp_listener_add_media(&hdoipd.listener, &vtb_video);
                rtsp_listener_add_media(&hdoipd.listener, &box_sys_vtb);
                rtsp_listener_add_media(&hdoipd.listener, &usb_media);
                hdoipd_set_state(HOID_VTB);
            }
        } else {
            report(" ? already in state vtb");
        }
    } else {
        report(" ? ethernet down");
    }
}

/* Activate VRB
 *
 * First goto ready then goto VRB. Loads all media source for RTSPX.
 *
 * !!! Info: <hdoipd> must be locked to prevent state inconsistency !!!
 */
int hdoipd_goto_vrb()
{
    if (hdoipd_rsc(RSC_ETH_LINK)) {
        if (!hdoipd_state(HOID_VRB)) {
            if (hdoipd_goto_ready()) {
                report(CHANGE "goto vrb");

                // set output uses slave timer
                hoi_drv_timer(DRV_TMR_OUT);

                rtsp_listener_add_media(&hdoipd.listener, &box_sys_vrb);
                rtsp_listener_add_media(&hdoipd.listener, &usb_media);

                hdoipd_set_state(HOID_VRB);
            }
        } else {
            report(INFO "already in state vrb");
            return 0;
        }
    } else {
        report(ERROR "ethernet down");
        return -1;
    }
    return 0;
}

/* Activate Media
 *
 * activates a media client.
 */
int hdoipd_vrb_setup(t_rtsp_media* media, void UNUSED *d)
{
    int ret;
    char *uri;

    if (!hdoipd_state(HOID_VRB)) {
        report(ERROR "hdoipd_vrb_setup() only valid in state VRB");
        return -1;
    }

    // test if we are ready to start media
    if (rtsp_media_ready(media) != RTSP_SUCCESS) {
        //report(ERROR "hdoipd_vrb_setup() media-client not ready");
        return -1;
    }

    uri = reg_get("remote-uri");

    // we can't always make sure that hdoipd.client is set to NULL
    // after freeing it so we instead free it here.
    if (hdoipd.client != NULL && !hdoipd.client->open)
    {
        report(" ? freeing old RTSP Client [%d]", hdoipd.client->nr);
        free(hdoipd.client);
        hdoipd.client = NULL;
    }

    // don't create client if it's already running
    if (hdoipd.client == NULL) {
        // new connection = new hdcp-key exchange
        hdoipd.hdcp.ske_executed = 0;

        hdoipd.client = rtsp_client_open(uri, &box_sys_vrb);
    }

    if (hdoipd.client != NULL)
        rtsp_client_add_media(hdoipd.client, media);

    // try to start only when not usb
    if (strcmp(media->name, "usb") != 0) {
        if (hdoipd.client) {
            // try to setup a connection
            ret = rtsp_media_setup(media);
            if (ret == RTSP_SUCCESS) {
                rtsp_media_play(media);
            } else {
                report(ERROR "hdoipd_vrb_setup() rtsp_media_setup failed (%d)", ret);
                rtsp_client_close(hdoipd.client, true);
                hdoipd.client = NULL;
                return -1;
            }
        } else {
            report(ERROR "hdoipd_vrb_setup() rtsp_client_open failed");
            //osd_printf("VTB(%s) not found. Waiting for %s\n", media->name, uri);
            return -1;
        }
    }

    return 0;
}

/** Start to play stream
 *
 * When in VRB state this starts the stream
 */
int hdoipd_vrb_play(t_rtsp_media *media, void UNUSED *d)
{
    report(CHANGE "vrb_play(%s)", media->name);

    if (!hdoipd_state(HOID_VRB)) {
        report(ERROR "hdoipd_vrb_play() only valid in state VRB");
        return -1;
    }
    return rtsp_media_play(media);
}

void hdoipd_canvas(uint32_t width, uint32_t height, uint32_t fps)
{
    t_video_timing* timing;

    if (hdoipd_goto_ready()) {

        if ((timing = hoi_res_timing(width, height, fps))) {
            hoi_image_canvas_create(timing);
            hdoipd_set_rsc(RSC_VIDEO_OUT);
            hdoipd_set_state(HOID_SHOW_CANVAS);
        } else {
            report(ERROR "could not start canvas %d x %d @ %d Hz", width, height, fps);
            return;
        }

    }
}

int hdoipd_start_vrb(bool force)
{
  if (!hdoipd_state(HOID_VRB))
    return RTSP_CLIENT_ERROR;

  if ((hdoipd.client != NULL && hdoipd.client->open) || (hdoipd.task_commands & TASK_START_VRB))
    return RTSP_CLIENT_ERROR;

  if (!force && !hdoipd.auto_stream)
    return RTSP_CLIENT_ERROR;

  hdoipd_set_task_start_vrb();
}

/** Restarts VRB when already in VRB state
 *
 * @return 0 on state change, other when state not changed
 */
int hdoipd_init_vrb_cb(t_rtsp_media* media, void* d)
{
    uint32_t dev_id;

    // USB
    if (strcmp(media->name, "usb") == 0) {
        if (hdoipd_vrb_setup(media, d) < 0)
            return 1;
        return 0;
    }

    // detect connected video card
    hoi_drv_get_device_id(&dev_id);

    // If SDI there is no event if a source is connected
    if (dev_id == BDT_ID_SDI8_BOARD)
        hdoipd_set_rsc(RSC_VIDEO_SINK);

    if (rtsp_media_sinit(media)) {
        if (hdoipd_vrb_setup(media, d) < 0)
            return 1;
    }
    else if (rtsp_media_sready(media)) {
        if (hdoipd_vrb_play(media, d) < 0)
            return 1;
    }
    else
      return 1;

    return 0;
}

int hdoipd_init_vrb(void *d)
{
    int failed = 0;
    report(CHANGE "attempt to start vrb");

    if (hdoipd_state(HOID_VRB)) {
        if ((failed = hdoipd_media_callback(d, hdoipd_init_vrb_cb, 0))) {
            report(ERROR "attempt to start vrb failed");
            return failed;
        }

        usb_try_to_connect_device(&hdoipd.usb_devices);
    }
    else
      report(ERROR "attempt to start vrb when not in state vrb");

    return 0;
}

void hdoipd_set_task_start_vrb(void)
{
    // stop sending alive packets
    hdoipd.task_commands |= TASK_START_VRB;
    hdoipd.task_timeout = 1;
    hdoipd.task_repeat = 2;
}

void hdoipd_task(void)
{
    // Task: start vrb
    if(hdoipd.task_commands & TASK_START_VRB) {
        if(hdoipd.task_timeout) {
            hdoipd.task_timeout--;
        } else {
            if(hdoipd_init_vrb(0)) {
            	if(hdoipd.task_repeat > 0) {
                    hdoipd.task_timeout = 50;
                    hdoipd.task_repeat--;
                } else {
                    hdoipd.task_commands &= ~TASK_START_VRB;
                }
            } else {
                hdoipd.task_repeat = 0;
                hdoipd.task_commands &= ~TASK_START_VRB;
            }
        }
    }
}



//------------------------------------------------------------------------------

/** distributes events in vrb mode
 *
 * when in VRB mode events are sent to all active
 * media-clients
 * monitor plugin is a Starting Point (= device starts operation on this event)
 * this starting point is active when reg("mode-media") contains video or audio(embedded)
 *
 * @param event new HW-Event
 */
void hdoipd_fsm_vrb(uint32_t event)
{
    switch (event) {
        case E_ADV9889_CABLE_ON:
            // plug in the cable is a start point for the VRB to
            // work when video or embedded audio is desired ...
            if (hdoipd.client != NULL && hdoipd.client->open)
                rtsp_client_event(hdoipd.client, EVENT_VIDEO_SINK_ON);
            else
                hdoipd_start_vrb(false);
        break;
        case E_ADV9889_CABLE_OFF:
            rtsp_client_event(hdoipd.client, EVENT_VIDEO_SINK_OFF);
            hdoipd.hdcp.ske_executed = 0;
        break;
        case E_ETI_VIDEO_OFF:
            rtsp_client_event(hdoipd.client, EVENT_VIDEO_STIN_OFF);
        break;
        case E_ETI_AUDIO_OFF:
            rtsp_client_event(hdoipd.client, EVENT_AUDIO_STIN_OFF);
        break;
    }
}


/** distributes events in vtb mode
 *
 * when in VTB mode events are sent to all active
 * media-server(from listener)
 * Only relevant Events are sent
 *  - Video: on/change, off
 *  - Audio: on, off
 *
 * @param event new HW-Event
 */
void hdoipd_fsm_vtb(uint32_t event)
{
    switch (event) {
        case E_GS2971_VIDEO_ON:
            rtsp_listener_event(&hdoipd.listener, EVENT_VIDEO_IN_ON);
        break;
        case E_GS2971_VIDEO_OFF:
            rtsp_listener_event(&hdoipd.listener, EVENT_VIDEO_IN_OFF);
        break;
        case E_ADV7441A_NC:
            rtsp_listener_event(&hdoipd.listener, EVENT_VIDEO_IN_OFF);
            rtsp_listener_event(&hdoipd.listener, EVENT_AUDIO_IN0_OFF);
        break;
        case E_ADV7441A_NEW_HDMI_RES:
            rtsp_listener_event(&hdoipd.listener, EVENT_VIDEO_IN_ON);
        break;
        case E_ADV7441A_NEW_VGA_RES:
            rtsp_listener_event(&hdoipd.listener, EVENT_VIDEO_IN_ON);
        break;
        case E_ADV7441A_NO_AUDIO:
            rtsp_listener_event(&hdoipd.listener, EVENT_AUDIO_IN0_OFF);
        break;
        case E_ASI_NEW_FS:
            rtsp_listener_event(&hdoipd.listener, EVENT_AUDIO_IN0_ON);
        break;
    }
}

void show_local_ip_address_on_osd()
{
    int fd;
    struct ifreq ifr;
    t_hoi_msg_info* info;
    uint32_t image_freq, image_pixel;
    uint32_t fs;
    bool vtb;
    hoi_drv_info_all(&info);
    hoi_drv_get_fs(&fs);

    if (!strcmp("vtb", reg_get("mode-start"))) {
        vtb = true;
    } else  {
        vtb = false;
    }

    hoi_drv_clr_osd();

    osd_printf("\nUnit Information");
    osd_printf("\n----------------");

    if(hdoipd_rsc(RSC_ETH_LINK)) {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        ifr.ifr_addr.sa_family = AF_INET;           // get an IPv4 IP address
        strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);  // get IP address attached to "eth0"
        ioctl(fd, SIOCGIFADDR, &ifr);
        close(fd);

        // show IP address on OSD
        osd_printf("\nIP address:        %s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    } else {
        osd_printf("\nIP address:        not available");
    }

    osd_printf("\nSubnetmask:        %s", reg_get("system-subnet"));
    osd_printf("\nGateway:           %s", reg_get("system-gateway"));
    osd_printf("\nHostname:          %s", reg_get("system-hostname"));

    if (vtb) {
        osd_printf("\nSystem mode :      Transmitter");
    } else {
        osd_printf("\nSystem mode :      Receiver");
    }

    osd_printf("\n\nSerial number:     %s", reg_get("serial-number"));
    osd_printf("\nFW-Version:        %i.%i", VERSION_MAJOR, VERSION_MINOR);

    osd_printf("\n\n\nStream");
    osd_printf("\n------");
    osd_printf("\nUnicast address:   %s", reg_get("remote-uri"));
    if (vtb) {
        osd_printf("\nMulticast address: %s", reg_get("multicast_group"));
    } else {
        osd_printf("\nMulticast address: (transmitter only)");
    }

    if (vtb) {
        osd_printf("\nMax. data rate:    %i Mbit/s", ((uint32_t)atol(reg_get("bandwidth"))) / 131072 );
    } else {
        osd_printf("\nMax. data rate:    (transmitter only)");
    }

    if (vtb) {
        if (hdoipd.rsc_state & RSC_VIDEO_IN) {

            image_pixel = (info->timing.height + info->timing.vfront + info->timing.vback + info->timing.vpulse) * (info->timing.width + info->timing.hfront + info->timing.hback + info->timing.hpulse);
            image_freq = (info->timing.pfreq / (image_pixel / 100));
            osd_printf("\nResolution:        %d x %d @ %d.%02d Hz / %d MHz", info->timing.width, info->timing.height, (image_freq/100), (image_freq%100), (info->timing.pfreq/1000000));
        } else {
            osd_printf("\nResolution:        (no input)");
        }
    } else {
        osd_printf("\nResolution:        (transmitter only)");
    }

    if (vtb) {
        if (fs != 0) {
            osd_printf("\nAudio:             %i Hz  %i Bit  %i channel", info->audio_fs[0], info->audio_width[0], info->audio_cnt[0]);
        } else {
            osd_printf("\nAudio:             (no audio)");
        }
    } else {
        osd_printf("\nAudio:             (transmitter only)");
    }
}

void hdoipd_event(uint32_t event)
{
    uint32_t buff;
    char *s;
    uint32_t hdcp;

    lock("hdoipd_event");
        report(EVENT "(%x) %s ", event, event_str(event));

    switch (event) {
        // Ethernet connection on/off
        case E_ETO_LINK_UP:
            hoi_drv_set_led_status(ETHERNET_ACTIVE);
            hdoipd_set_rsc(RSC_ETH_LINK);
            if (hdoipd_state(HOID_READY)) {
                hdoipd_start();
            }
            if (!hdoipd.ethernet_init) {
                show_local_ip_address_on_osd();
                hdoipd.ethernet_init = true;
            }
        break;
        case E_ETO_LINK_DOWN:
            hoi_drv_set_led_status(ETHERNET_INACTIVE);
            hdoipd_clr_rsc(RSC_ETH_LINK);
            if (hdoipd_state(HOID_VTB|HOID_VRB)) {
                hdoipd_force_ready();
                osd_permanent(true);
                osd_printf("Ethernet connection lost\n");
            }
        break;

        // resource state
        case E_ADV9889_CABLE_ON:
            // interrupt appears sometimes often than one time
            if ((!hdoipd_rsc(RSC_VIDEO_SINK))) {
                hdoipd_set_rsc(RSC_VIDEO_SINK);
                hoi_drv_set_led_status(DVI_OUT_CONNECTED_NO_AUDIO);
            } else {
                event = event & ~E_ADV9889_CABLE_ON;                // clear event to prevent that video will be started in hdoipd_fsm_vrb
            }
        break;
        case E_ADV9889_CABLE_OFF:
            hdoipd_clr_rsc(RSC_VIDEO_SINK);
            if (hdoipd_state(HOID_VTB)) {
                hoi_drv_set_led_status(DVI_OUT_DISCONNECTED_VTB);
            } else {
                hoi_drv_set_led_status(DVI_OUT_DISCONNECTED_VRB);
            }
        break;
        case E_ADV9889_HDCP_NOT_OK:
            hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
            osd_permanent(true);
            osd_printf("HDCP link is not ok\n");
            hoi_drv_hdcp_black_output();
        break;
        case E_ADV7441A_NEW_HDMI_RES:
            hoi_drv_get_encrypted_status(&hdcp);        // wait up to 1s until register is valid
            if (hdcp) {
                report(INFO "\n ******* Incoming stream is encrypted!! ****** ");
                hdoipd_set_rsc(RSC_VIDEO_IN_HDCP);
                // enable HDCP on AD9889 for loopback
                hoi_drv_hdcp_adv9889en();
            }
            if (!hdoipd_rsc(RSC_VIDEO_IN)) {
                hdoipd_set_rsc(RSC_VIDEO_IN);
                hdoipd_clr_rsc(RSC_VIDEO_IN_VGA);
                hdoipd_set_rsc(RSC_AUDIO0_IN);
                hoi_drv_set_led_status(DVI_IN_CONNECTED_WITH_AUDIO);
                hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
                show_local_ip_address_on_osd();
            }
        break;
        case E_ADV7441A_NEW_VGA_RES:
            hdoipd_set_rsc(RSC_VIDEO_IN);
            hdoipd_set_rsc(RSC_VIDEO_IN_VGA);
            hdoipd_clr_rsc(RSC_AUDIO0_IN);
            hoi_drv_get_analog_timing(&buff);
            if (buff != 7) {
                hoi_drv_set_led_status(DVI_IN_CONNECTED_NO_AUDIO);
            } else {
                hdoipd_clr_rsc(RSC_VIDEO_IN);
                hdoipd_clr_rsc(RSC_VIDEO_IN_VGA);
                event = event & ~E_ADV7441A_NEW_VGA_RES;                // clear event to prevent that video will be started in hdoipd_fsm_vtb
                event = event | E_ADV7441A_NC;                          // set event to stop video, LEDs are set in case "E_ADV7441A_NC"
            }
        break;
        case E_ADV7441A_NC:
            hdoipd_clr_rsc(RSC_VIDEO_IN);
            hdoipd_clr_rsc(RSC_AUDIO0_IN);
            if (hdoipd_state(HOID_VTB)) {
                hoi_drv_set_led_status(DVI_IN_DISCONNECTED_VTB);
                hoi_drv_hdcp_adv9889dis();
                if(!hdoipd_rsc(RSC_EVO)) {
                    hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
                    osd_permanent(true);
                    osd_printf("no video input...\n");
                }
            } else {
                hoi_drv_set_led_status(DVI_IN_DISCONNECTED_VRB);
            }
        break;
        case E_ADV7441A_NO_AUDIO:
        break;
        case E_ETI_VIDEO_ON:
            hdoipd_set_rsc(RSC_EVI);
            hoi_drv_set_led_status(STREAM_ACTIVE);
        break;
        case E_ETI_VIDEO_OFF:
            hdoipd_clr_rsc(RSC_EVI);
            if(hdoipd_rsc(RSC_ETH_LINK)) {
                hoi_drv_set_led_status(ETHERNET_ACTIVE);
            }
        break;
        case E_ETI_AUDIO_ON:
            hdoipd_set_rsc(RSC_EAI);
            hoi_drv_set_led_status(DVI_OUT_CONNECTED_WITH_AUDIO);
        break;
        case E_ETI_AUDIO_OFF:
            hdoipd_clr_rsc(RSC_EAI);
            if(hdoipd_rsc(RSC_VIDEO_SINK)) {
                hoi_drv_set_led_status(DVI_OUT_CONNECTED_NO_AUDIO);
            }
        break;
        case E_ETO_VIDEO_ON:
            hdoipd_set_rsc(RSC_EVO);
            hoi_drv_set_led_status(STREAM_ACTIVE);
        break;
        case E_ETO_VIDEO_OFF:
            hdoipd_clr_rsc(RSC_EVO);
            if(hdoipd_rsc(RSC_ETH_LINK)) {
                hoi_drv_set_led_status(NO_STREAM_ACTIVE);
            }
        break;
        case E_ETO_AUDIO_ON:
            hdoipd_set_rsc(RSC_EAO);
        break;
        case E_ETO_AUDIO_OFF:
            hdoipd_clr_rsc(RSC_EAO);
        break;
        case E_HDCP_STREAMING_ERROR:   // restart VTB if Kernel detects HDCP streaming error
        	report(INFO "*********** HDCP streaming error ***********");
            if (((hdoipd_state(HOID_VTB)) && (hdoipd_rsc(RSC_VIDEO_IN))) || ((hdoipd_state(HOID_VRB)) && (hdoipd_rsc(RSC_VIDEO_SINK)))) {
                hdoipd_force_ready();
                report(INFO "HDCP ERROR, restarting system");
                hdoipd_start();
            }
        break;
        case E_GS2971_VIDEO_ON:
            hdoipd_set_rsc(RSC_VIDEO_IN);
            hdoipd_set_rsc(RSC_VIDEO_IN_SDI);
            hdoipd_set_rsc(RSC_AUDIO0_IN);
            s = reg_get("mode-start"); // hdoipd_state(HOID_VTB) is not set yet, when sdi input is active on startup
            if (strcmp(s, "vtb") == 0) {
                hoi_drv_set_led_status(SDI_IN_CONNECTED_WITH_AUDIO);
            }
        break;
        case E_GS2971_VIDEO_OFF:
            hdoipd_clr_rsc(RSC_VIDEO_IN);
            hdoipd_clr_rsc(RSC_VIDEO_IN_SDI);
            hdoipd_clr_rsc(RSC_AUDIO0_IN);
            if (hdoipd_state(HOID_VTB)) {
                hoi_drv_set_led_status(SDI_IN_DISCONNECTED);
            }
        break;
        case E_GS2971_LOOP_ON:
            s = reg_get("mode-start");
            if (strcmp(s, "vtb") == 0) {
                hoi_drv_set_led_status(SDI_LOOP_ON_WITH_AUDIO);
            }
        break;
        case E_GS2971_LOOP_OFF:
            if (hdoipd_state(HOID_VTB)) {
                hoi_drv_set_led_status(SDI_LOOP_OFF);
            }
        break;
        // ...
        case E_VSI_FIFO2_FULL:
        break;
        case E_VSI_CDFIFO_FULL:
        break;
        case E_VSO_PAYLOAD_EMPTY:
        break;
        case E_VSO_PAYLOAD_MTIMEOUT:
        break;

        case E_VSO_CHOKED:
        case E_VSO_TIMESTAMP_ERROR:
            // try to repair
            //hoi_drv_repair();
        break;

        case E_ASI_RBF_ERROR:
        break;
        case E_ASI_NEW_FS:
            hoi_drv_get_fs(&buff);
            if (buff == 0) {
                event = event & ~E_ASI_NEW_FS;                          // clear event to prevent that audio will be started in hdoipd_fsm_vtb
                event = event | E_ADV7441A_NO_AUDIO;                    // set event to stop audio
            } else {
                hoi_drv_new_audio(buff);
            }
        break;
        case E_ASO_SIZE_ERROR:
        break;
        case E_ASO_FIFO_EMPTY:
        break;
        case E_ASO_FIFO_FULL:
        break;
        case E_ASO_RAM_FULL:
        break;

        case E_VIO_JD0ENC_OOS:
        case E_VIO_JD1ENC_OOS:
        case E_VIO_JD2ENC_OOS:
        case E_VIO_JD3ENC_OOS:
        break;

        case E_VIO_PLL_OOS:
        break;

        case E_VIO_PLL_SNC:
        break;

        case E_VIO_RES_CHANGE:
        break;

        case E_VIO_ADV212_CFERR:
        case E_VIO_ADV212_CFERR+1:
        case E_VIO_ADV212_CFERR+2:
        case E_VIO_ADV212_CFERR+3:
        break;

        case E_VIO_ADV212_CFTH:
        case E_VIO_ADV212_CFTH+1:
        case E_VIO_ADV212_CFTH+2:
        case E_VIO_ADV212_CFTH+3:
        break;
        case E_BDT_RESET_BUTTON:
            show_local_ip_address_on_osd();
        break;
    }

        if (hdoipd_state(HOID_VRB)) {
            hdoipd_fsm_vrb(event);
        } else if (hdoipd_state(HOID_VTB)) {
            hdoipd_fsm_vtb(event);
        }
    unlock("hdoipd_event");
}

void hdoipd_start()
{
    char *s = reg_get("mode-start");
    if (strcmp(s, "vtb") == 0) {
        hdoipd_goto_vtb();
    } else if (strcmp(s, "vrb") == 0) {
        hdoipd_goto_vrb();
    } else if (strcmp(s, "none") == 0) {
        ;
    } else {
        report("unknown mode-start = %s", s);
    }

}

//#define VID_SIZE        ((3*1920*1080)+4)
#define VID_SIZE        (3*1024*1024)
#define AUD_RX_SIZE     (512*1024)
#define AUD_TX_SIZE     (512*1024)

bool hdoipd_init(int drv)
{
    uint32_t dev_id;
    uint32_t reset_to_default;
    char *s;

    pthread_mutexattr_t attr;

    hdoipd.drv = drv;
    hdoipd.set_listener = 0;
    hdoipd.get_listener = 0;
    hdoipd.verify = 0;
    hdoipd.registry = 0;
    hdoipd.state = HOID_IDLE;
    hdoipd.rsc_state = 0;
    hdoipd.vtb_state = VTB_OFF;
    hdoipd.vrb_state = VRB_OFF;
    hdoipd.client = NULL;
    hdoipd.auto_stream = false;
    hdoipd.hdcp.ske_executed = 0;
    hdoipd.hdcp.enc_state = 0;

    // allocate resource
    void* vid = malloc(VID_SIZE);
    void* aud_rx = malloc(AUD_RX_SIZE);
    void* aud_tx = malloc(AUD_TX_SIZE);

    hdoipd_set_default();

    hdoipd_register_task();

    // if reset button was not pressed longer than 5sec, load config
    hoi_drv_get_reset_to_default(&reset_to_default);
    hoi_cfg_read(CFG_FILE, (bool) reset_to_default);

    hdoipd_registry_update();

    hdoipd.dhcp = reg_test("system-dhcp", "true");

    hoi_cfg_system();

    hdoipd.local.vid_port = htons(reg_get_int("video-port"));
    hdoipd.local.aud_port = htons(reg_get_int("audio-port"));
    hdoipd.ethernet_init = false;

    // detect connected video card
    hoi_drv_get_device_id(&dev_id);

    switch (dev_id) {
        case BDT_ID_HDMI_BOARD: hdoipd.drivers = DRV_ADV9889 | DRV_ADV7441;
                                break;
        case BDT_ID_SDI8_BOARD: hdoipd.drivers = DRV_GS2971  | DRV_GS2972;
                                break;
        default:                report("Video card detection failed");
    }

    // activate red LED on used video-board connectors
    s = reg_get("mode-start");
    if (strcmp(s, "vtb") == 0) {
        hoi_drv_set_led_status(CONFIGURE_VTB);
    } else if (strcmp(s, "vrb") == 0) {
        hoi_drv_set_led_status(CONFIGURE_VRB);
    }

    hdoipd.auto_stream = reg_test("auto-stream", "true");

    alive_check_client_open(&(hdoipd.amx), reg_test("amx-en", "true"), reg_get_int("amx-hello-interval"), reg_get("amx-hello-ip"), reg_get_int("amx-hello-port"), 1, true);

    pthread_mutexattr_init(&attr);
    //pthread_mutexattr_setrobust_np(&attr, PTHREAD_MUTEX_ROBUST_NP);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    pthread_mutexattr_setpshared (&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&hdoipd.mutex, &attr);

    // starting with load drivers the driver may send events
    // so we lock before calling hoi_drv_ldrv
    lock("hdoipd_init");
    {
        report("load drivers...");

        if ((hoi_drv_ldrv(hdoipd.drivers))) {
            report("loading drivers failed");
            unlock("hdoipd_init");
            return false;
        }

        hdoipd_set_state(HOID_READY);

        report("setup buffers...");

        if (!vid) {
            report("malloc(VID_SIZE) failed");
            unlock("hdoipd_init");
            return false;
        }

        if (!aud_rx) {
            report("malloc(AUD_RX_SIZE) failed");
            unlock("hdoipd_init");
            return false;
        }

        if (!aud_tx) {
            report("malloc(AUD_TX_SIZE) failed");
            unlock("hdoipd_init");
            return false;
        }

        hdoipd.img_buff = vid;

        if (hoi_drv_buf(
                aud_rx, AUD_RX_SIZE,
                vid, VID_SIZE,
                aud_tx, AUD_TX_SIZE,
                vid, VID_SIZE)) {
            report("set buffers failed");
            unlock("hdoipd_init");
            return false;
        }

        // init frame rate reduction
        hoi_drv_set_fps_reduction(reg_get_int("fps_divide"));

        // setup default output
        osd_permanent(true);
        show_local_ip_address_on_osd();
    }
    unlock("hdoipd_init");

    rtsp_listener_start(&hdoipd.listener, reg_get_int("rtsp-server-port"));

#ifdef USE_OSD_TIMER
    hdoipd_osd_timer_start();
#endif

    // initialize usb handler
    usb_handler_init(&hdoipd.usb_devices);

    //hoi_drv_wdg_init(2000000000); //set and start watchdog (to 20 sec.)
    //hoi_drv_wdg_enable();
    report(INFO "START WATCHDOG");
    return true;
}
