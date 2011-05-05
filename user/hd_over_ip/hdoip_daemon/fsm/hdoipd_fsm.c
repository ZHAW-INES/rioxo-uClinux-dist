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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hoi_drv_user.h"
#include "hoi_image.h"
#include "hoi_res.h"
#include "hoi_cfg.h"
#include "hdoipd_callback.h"
#include "hdoipd_msg.h"
#include "hdoipd_osd.h"
#include "hdoipd_fsm.h"
#include "hdoipd_task.h"
#include "rscp_include.h"
#include "edid.h"

#include "vrb_video.h"
#include "vtb_video.h"
#include "vrb_audio.h"
#include "vtb_audio.h"
#include "box_sys.h"
		

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
    if ((state & RSC_AUDIO_SYNC) && hdoipd_rsc(RSC_VIDEO_OUT)) {
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
            rscp_listener_teardown_all(&hdoipd.listener);
            rscp_listener_free_media(&hdoipd.listener);
            hdoipd_off(true);
            return true;
        break;
        case HOID_VRB:
            // shut down VRB
            rscp_client_deactivate(hdoipd.client);
            rscp_listener_free_media(&hdoipd.listener);
            hdoipd_off(true);
            return true;
        break;
        case HOID_READY:
            return true;
        break;
        case HOID_LOOP:
        case HOID_SHOW_CANVAS:
            hoi_image_free(hdoipd.canvas);
            hdoipd.canvas = 0;
            hdoipd_off(true);
            return true;
        break;
        default:
            report("request not supported in state %s\n", statestr(hdoipd.state));
            return false;
        break;
    }

    return false;
}

/** Force to ready state
 *
 * Do not consider ethernet state
 */
void hdoipd_force_ready()
{
    switch (hdoipd.state) {
        case HOID_VRB:
            rscp_client_force_close(hdoipd.client);
        break;
        case HOID_VTB:
            rscp_listener_close_all(&hdoipd.listener);
        break;
    }

    hdoipd_goto_ready();

    report(INFO, "hdoipd_force_ready() done");
}


/* Activate VTB
 *
 * First goto ready then goto VTB. Loads all media source for RSCPX.
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

                rscp_listener_add_media(&hdoipd.listener, &vtb_audio);
                rscp_listener_add_media(&hdoipd.listener, &vtb_video);
                rscp_listener_add_media(&hdoipd.listener, &box_sys);
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
 * First goto ready then goto VRB. Loads all media source for RSCPX.
 *
 * !!! Info: <hdoipd> must be locked to prevent state inconsistency !!!
 */
void hdoipd_goto_vrb()
{
    if (hdoipd_rsc(RSC_ETH_LINK)) {
        if (!hdoipd_state(HOID_VRB)) {
            if (hdoipd_goto_ready()) {
                report(CHANGE "goto vrb");

                // set output uses slave timer
                hoi_drv_timer(DRV_TMR_OUT);

                rscp_listener_add_media(&hdoipd.listener, &box_sys);

                hdoipd_set_state(HOID_VRB);

                // register remote for "hello"
                box_sys_set_remote(reg_get("remote-uri"));

                // first thing to try is setup a new connection based on registry

                if(hdoipd.auto_stream) {
                    hdoipd_set_task_start_vrb();
                }

            }
        } else {
            report(ERROR "already in state vrb");
        }
    } else {
        report(ERROR "ethernet down");
    }
}

/* Activate Media
 *
 * activates a media client.
 */
int hdoipd_vrb_setup(t_rscp_media* media, void UNUSED *d)
{
    t_rscp_client* client;
    int ret;
    char tmp[200];

    report(CHANGE "vrb_setup(%s)", media->name);

    if (!hdoipd_state(HOID_VRB)) {
        report(ERROR "hdoipd_vrb_setup() only valid in state VRB");
        return -1;
    }

    // test if we are ready to start media
    if (rscp_media_ready(media) != RSCP_SUCCESS) {
        report(ERROR "hdoipd_vrb_setup() media-client not ready");
        return -1;
    }

    // open media (currently all source defined by "remote-uri" + media->name)
    sprintf(tmp, "%s/%s", reg_get("remote-uri"), media->name);
    client = rscp_client_open(hdoipd.client, media, tmp);

    if (client) {
        // try to setup a connection
        ret = rscp_media_setup(media);
        if (ret == RSCP_SUCCESS) {
            rscp_media_play(media);
        } else {
            report(ERROR "hdoipd_vrb_setup() rscp_media_setup failed");
            rscp_client_close(client);
        }
    } else {
        report(ERROR "hdoipd_vrb_setup() rscp_client_open failed");
        osd_printf("VTB(%s) not found. Waiting for %s\n", media->name, tmp);
    }

    return 0;
}

/** Start to play stream
 *
 * When in VRB state this starts the stream
 */
int hdoipd_vrb_play(t_rscp_media *media, void UNUSED *d)
{
    report(CHANGE "vrb_play(%s)", media->name);

    if (!hdoipd_state(HOID_VRB)) {
        report(ERROR "hdoipd_vrb_play() only valid in state VRB");
        return -1;
    }

    return rscp_media_play(media);
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

void hdoipd_debug(uint32_t width, uint32_t height, uint32_t fps)
{
    t_video_timing* timing;

    if (hdoipd_goto_ready()) {

        if ((timing = hoi_res_timing(width, height, fps))) {
            hoi_drv_debug(timing);
            hdoipd_set_rsc(RSC_VIDEO_OUT|RSC_OSD);
        } else {
            report(ERROR "could not start debug %d x %d @ %d Hz", width, height, fps);
            return;
        }

    }
}

/** Restarts VRB when already in VRB state
 *
 * @return 0 on state change, other when state not changed
 */
int hdoipd_start_vrb_cb(t_rscp_media* media, void* d)
{
    int os = media->state;
    if (rscp_media_sinit(media)) {
        hdoipd_vrb_setup(media, d);
    } else if (rscp_media_sready(media)) {
        hdoipd_vrb_play(media, d);
    } else {
        return 0;
    }
    return (os == media->state);
}
int hdoipd_start_vrb(void *d)
{
    int failed = 0;
    report(CHANGE "attempt to start vrb");
    if (hdoipd_state(HOID_VRB)) {
        failed = hdoipd_media_callback(d, hdoipd_start_vrb_cb, 0);
    } else {
        report(ERROR "attempt to start vrb when not in state vrb");
    }
    if (failed) report(ERROR "attempt to start vrb failed");
    return failed;
}

void hdoipd_set_task_start_vrb(void)
{
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
            if(hdoipd_start_vrb(0)) {
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
            if(hdoipd.auto_stream) {
                hdoipd_set_task_start_vrb();
            }
        break;
        case E_ADV9889_CABLE_OFF:
            rscp_client_event(hdoipd.client, EVENT_VIDEO_SINK_OFF);
        break;
        case E_ETI_VIDEO_OFF:
            rscp_client_event(hdoipd.client, EVENT_VIDEO_STIN_OFF);
        break;
        case E_ETI_AUDIO_OFF:
            rscp_client_event(hdoipd.client, EVENT_AUDIO_STIN_OFF);
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
        case E_ADV7441A_NC:
            rscp_listener_event(&hdoipd.listener, EVENT_VIDEO_IN_OFF);
        break;
        case E_ADV7441A_NEW_RES:
            rscp_listener_event(&hdoipd.listener, EVENT_VIDEO_IN_ON);
        break;
        case E_ADV7441A_NEW_AUDIO:
            rscp_listener_event(&hdoipd.listener, EVENT_AUDIO_IN0_ON);
        break;
        case E_ADV7441A_NO_AUDIO:
            rscp_listener_event(&hdoipd.listener, EVENT_AUDIO_IN0_OFF);
        break;
    }
}

void hdoipd_event(uint32_t event)
{
    lock("hdoipd_event");
        report(EVENT "(%x) %s ", event, event_str(event));

    switch (event) {
        // Ethernet connection on/off
        case E_ETO_LINK_UP:
            hdoipd_set_rsc(RSC_ETH_LINK);
            if (hdoipd_state(HOID_READY)) {
                hdoipd_start();
            }
        break;
        case E_ETO_LINK_DOWN:
            hdoipd_clr_rsc(RSC_ETH_LINK);
            if (hdoipd_state(HOID_VTB|HOID_VRB)) {
                hdoipd_force_ready();
                osd_permanent(true);
                osd_printf("Ethernet connection lost\n");
            }
        break;

        // resource state
        case E_ADV9889_CABLE_ON:
            hdoipd_set_rsc(RSC_VIDEO_SINK);
        break;
        case E_ADV9889_CABLE_OFF:
            hdoipd_clr_rsc(RSC_VIDEO_SINK);
        break;
        case E_ADV7441A_NC:
            hdoipd_clr_rsc(RSC_VIDEO_IN);
        break;
        case E_ADV7441A_NEW_RES:
            hdoipd_set_rsc(RSC_VIDEO_IN);
        break;
        case E_ADV7441A_NEW_AUDIO:
            hdoipd_set_rsc(RSC_AUDIO0_IN);
        break;
        case E_ADV7441A_NO_AUDIO:
            hdoipd_clr_rsc(RSC_AUDIO0_IN);
        break;
        case E_ETI_VIDEO_ON:
            hdoipd_set_rsc(RSC_EVI);
        break;
        case E_ETI_VIDEO_OFF:
            hdoipd_clr_rsc(RSC_EVI);
        break;
        case E_ETI_AUDIO_ON:
            hdoipd_set_rsc(RSC_EAI);
        break;
        case E_ETI_AUDIO_OFF:
            hdoipd_clr_rsc(RSC_EAI);
        break;
        case E_ETO_VIDEO_ON:
            hdoipd_set_rsc(RSC_EVO);
        break;
        case E_ETO_VIDEO_OFF:
            hdoipd_clr_rsc(RSC_EVO);
        break;
        case E_ETO_AUDIO_ON:
            hdoipd_set_rsc(RSC_EAO);
        break;
        case E_ETO_AUDIO_OFF:
            hdoipd_clr_rsc(RSC_EAO);
        break;

        case E_ADV7441A_HDCP:
        	printf("\n ******* Incomming stream is encrypted!! ****** \n");
        	hdoipd_set_rsc(RSC_VIDEO_IN_HDCP);
        //if VRB do nothing
        //if VTB, check if encryption is already activated
        //	if not, change to idle state and do init
        break;
        case E_ADV7441A_NO_HDCP:
        	printf("\n ******* Incomming stream is !!! NOT !!! encrypted!! ****** \n");
        	hdoipd_clr_rsc(RSC_VIDEO_IN_HDCP);
            //if VRB do nothing
            //if VTB, check if encryption is already activated
            //	if encryption is active, disable encryption and ask VRB to disable encryption
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
            hoi_drv_repair();
        break;

        case E_ASI_RBF_ERROR:
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
    }

        if (hdoipd_state(HOID_VRB)) {
            hdoipd_fsm_vrb(event);
        } else if (hdoipd_state(HOID_VTB)) {
            hdoipd_fsm_vtb(event);
        }
    unlock("hdoipd_event");
}

void hdoipd_hello()
{
    t_rscp_client *client;
    char *s;

    s = reg_get("hello-uri");
    if (s) {
        client = rscp_client_open(hdoipd.client, 0, s);

        if (client) {
            report(INFO "say hello to %s", s);
            rscp_client_hello(client);
            rscp_client_close(client);
        } else {
            report(ERROR "could not say hello to %s", s);
        }
    }

}

void hdoipd_start()
{
    hdoipd_hello();
    
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

#define VID_SIZE        (4*1024*1024)
#define AUD_RX_SIZE     (1024*1024)
#define AUD_TX_SIZE     (1024*1024)

bool hdoipd_init(int drv)
{

    int ret;
    pthread_mutexattr_t attr;

    memset(&hdoipd, 0, sizeof(t_hdoipd));

    hdoipd.drv = drv;
    hdoipd.set_listener = 0;
    hdoipd.get_listener = 0;
    hdoipd.verify = 0;
    hdoipd.registry = 0;
    hdoipd.state = HOID_IDLE;
    hdoipd.rsc_state = 0;
    hdoipd.vtb_state = VTB_OFF;
    hdoipd.vrb_state = VRB_OFF;
    hdoipd.client = list_create();
    hdoipd.auto_stream = false;

    // allocate resource
    void* vid = malloc(VID_SIZE);
    void* aud_rx = malloc(AUD_RX_SIZE);
    void* aud_tx = malloc(AUD_TX_SIZE);

    hdoipd_set_default();

    hdoipd_register_task();

    hoi_cfg_read(CFG_FILE);

    hdoipd_registry_update();

    hoi_cfg_system();

    hdoipd.local.vid_port = htons(reg_get_int("video-port"));
    hdoipd.local.aud_port = htons(reg_get_int("audio-port"));
    hdoipd.drivers = DRV_ADV9889 | DRV_ADV7441;

    hdoipd.auto_stream = reg_test("auto-stream", "true");

    if(hdoipd_amx_open(&(hdoipd.amx), reg_test("amx-en", "true"), reg_get_int("amx-hello-interval"),
                    inet_addr(reg_get("amx-hello-ip")), htons(reg_get_int("amx-hello-port")))) {
        perror("[AMX] hdoipd_amx_open() failed");
    }

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

        if (hoi_drv_buf(
                aud_rx, AUD_RX_SIZE,
                vid, VID_SIZE,
                aud_tx, AUD_TX_SIZE,
                vid, VID_SIZE)) {
            report("set buffers failed");
            unlock("hdoipd_init");
            return false;
        }

        // setup default output
        osd_permanent(true);
        osd_printf("Welcome to <rioxo> - 2011 Barox\n\n");
    }
    unlock("hdoipd_init");

    rscp_listener_start(&hdoipd.listener, reg_get_int("rscp-server-port"));

#ifdef USE_OSD_TIMER
    hdoipd_osd_timer_start();
#endif

    return true;
}

