/*
 * hdoipd_fsm.c
 *
 *  Created on: 03.11.2010
 *      Author: alda
 */

#include <stdio.h>
#include <stdlib.h>

#include "hoi_drv_user.h"
#include "hoi_image.h"
#include "hoi_res.h"
#include "hoi_cfg.h"
#include "hdoipd_msg.h"
#include "hdoipd_osd.h"
#include "hdoipd_fsm.h"
#include "rtsp_include.h"

#include "vrb_video.h"

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

bool hdoipd_set_state(int state)
{
    hdoipd.state = state;
    report("change state to %s", statestr(state));
    return true;
}

bool hdoipd_goto_ready()
{
    switch (hdoipd.state) {
        case HOID_IDLE:
            printf("hw not ready\n");
            return false;
        break;
        case HOID_VTB:
            // shut down VTB
            hdoipd_set_state(HOID_READY);
            return true;
        break;
        case HOID_VRB:
            // shut down VRB
            hdoipd_set_state(HOID_READY);
            return true;
        break;
        case HOID_READY:
        case HOID_LOOP:
        case HOID_SHOW_CANVAS:
            hoi_image_free(hdoipd.canvas);
            // from this states it is possible to change to a new active state
            hdoipd_set_state(HOID_READY);
            return true;
        break;
        default:
            report("request not supported in state %s\n", statestr(hdoipd.state));
            return false;
        break;
    }

    return false;
}

void hdoipd_canvas(t_hoic_canvas* cmd)
{
    t_video_timing* timing;

    if (hdoipd_goto_ready()) {
        if ((timing = hoi_res_timing(cmd->width, cmd->height, cmd->fps))) {
            hoi_image_canvas_create(hdoipd.drv, timing);
        } else {
            printf("could not start canvas %d x %d @ %d Hz\n", cmd->width, cmd->height, cmd->fps);
        }

        hdoipd.rsc_state |= RSC_VIDEO_OUT;
        hdoipd_set_state(HOID_SHOW_CANVAS);

    }
}


void hdoipd_load(t_hoic_load* cmd)
{
    FILE* f;

    if (hdoipd_goto_ready()) {

        if ((f = fopen(cmd->filename, "r"))) {
            hdoipd.canvas = hoi_image_load(hdoipd.drv, f);
            fclose(f);
        } else {
            printf("open file <%s> failed\n", cmd->filename);
        }
        if (hdoipd.canvas) {
            hdoipd.rsc_state |= RSC_VIDEO_OUT;
            hdoipd_set_state(HOID_SHOW_CANVAS);
        } else {
            hdoipd_set_state(HOID_READY);
        }

    }
}

void hdoipd_capture(t_hoic_capture* cmd)
{
    FILE* f;

    if (hdoipd_goto_ready()) {

        if ((f = fopen(cmd->filename, "w"))) {
            if (cmd->compress) {
                hoi_image_capture_jpeg2000(hdoipd.drv, f, cmd->size);
            } else {
                hoi_image_capture(hdoipd.drv, f);
            }
            fclose(f);
        } else {
            printf("open file <%s> failed\n", cmd->filename);
        }

        hdoipd_set_state(HOID_READY);

    }
}

void hdoipd_fmt_in(t_hoic_param* cmd)
{
    hoi_drv_ifmt(hdoipd.drv, cmd->value);
}

void hdoipd_fmt_out(t_hoic_param* cmd)
{
    hoi_drv_ofmt(hdoipd.drv, cmd->value);
}

void hdoipd_fmt_proc(t_hoic_param* cmd)
{
    hoi_drv_pfmt(hdoipd.drv, cmd->value);
}

void hdoipd_loop(t_hoic_cmd* cmd)
{
    if (hdoipd_goto_ready()) {
        hoi_drv_loop(hdoipd.drv);
        hdoipd_set_state(HOID_LOOP);
    }
}

void hdoipd_osd_on(t_hoic_cmd* cmd)
{
    hoi_drv_osdon(hdoipd.drv);
    hdoipd.rsc_state |= RSC_OSD;
}

void hdoipd_osd_off(t_hoic_cmd* cmd)
{
    hoi_drv_osdoff(hdoipd.drv);
    hdoipd.rsc_state &= ~RSC_OSD;
}

void hdoipd_vtb(t_hoic_cmd* cmd)
{
    if (hdoipd_goto_ready()) {
        hdoipd_set_state(HOID_VTB);
    }
}



void hdoipd_vrb_setup(t_hoic_load* cmd)
{
    t_rtsp_transport transport;
    int ret;

    printf("open: %s\n", cmd->filename);

    if (hdoipd_goto_ready()) {

        // open media
        ret = rtsp_client_open(&hdoipd.client, &vrb_video, cmd->filename);

        if (ret == RTSP_SUCCESS) {
            rtsp_default_transport(&transport); // TODO: daemon transport configuration
            transport.client_port = PORT_RANGE(htons(90), htons(91));
            ret = rtsp_client_setup(&hdoipd.client, &transport);
        }

        if (ret == RTSP_SUCCESS) {
            hdoipd_set_state(HOID_VRB);
        } else {
            hdoipd_set_state(HOID_READY);
        }

    }
}


void hdoipd_vrb_play(t_hoic_cmd* cmd)
{
    int ret;
    t_rtsp_rtp_format fmt;

    if (hdoipd.state == HOID_VRB) {

        // open media
        fmt.compress = 1;
        fmt.rtptime = 0;
        fmt.value = 0;
        ret = rtsp_client_play(&hdoipd.client, &fmt);

    }
}

void hdoipd_set_param(t_hoic_kvparam* cmd)
{
    printf("Set %s = \"%s\"\n", cmd->str, &cmd->str[cmd->offset]);
    report("Set %s = \"%s\"", cmd->str, &cmd->str[cmd->offset]);
    reg_set(cmd->str, &cmd->str[cmd->offset]);
//    reg_call(cmd->str, &cmd->str[cmd->offset]);
    
}

void hdoipd_get_param(t_hoic_kvparam* cmd, int rsp)
{
    char* s = reg_get(cmd->str);

    if (!s) s = "";
    hoic_ret_param(rsp, s);

    printf("Get %s = \"%s\"\n", cmd->str, s);
    report("Get %s = \"%s\"", cmd->str, s);
}

#define hdoipdreq(x, y) case x: y((void*)cmd); break
#define hdoipdreq_rsp(x, y) case x: y((void*)cmd, rsp); break
void hdoipd_request(uint32_t* cmd, int rsp)
{
    switch (cmd[0]) {
        hdoipdreq(HOIC_CANVAS, hdoipd_canvas);
        hdoipdreq(HOIC_LOAD, hdoipd_load);
        hdoipdreq(HOIC_CAPTURE, hdoipd_capture);
        hdoipdreq(HOIC_LOOP, hdoipd_loop);
        hdoipdreq(HOIC_OSD_ON, hdoipd_osd_on);
        hdoipdreq(HOIC_OSD_OFF, hdoipd_osd_off);
        hdoipdreq(HOIC_FMT_IN, hdoipd_fmt_in);
        hdoipdreq(HOIC_FMT_OUT, hdoipd_fmt_out);
        hdoipdreq(HOIC_FMT_PROC, hdoipd_fmt_proc);
        hdoipdreq(HOIC_VTB, hdoipd_vtb);
        hdoipdreq(HOIC_VRB_SETUP, hdoipd_vrb_setup);
        hdoipdreq(HOIC_VRB_PLAY, hdoipd_vrb_play);
        hdoipdreq(HOIC_PARAM_SET, hdoipd_set_param);
        hdoipdreq_rsp(HOIC_PARAM_GET, hdoipd_get_param);
        default: printf("command not supported <%08x>\n", cmd[0]);
    }
}

void hdoipd_event(uint32_t event)
{

}

void hdoipd_handler()
{

}

#define VID_SIZE        (1*1024*1024)
#define AUD_RX_SIZE     (1024*1024)
#define AUD_TX_SIZE     (1024*1024)

bool hdoipd_init(int drv)
{
    memset(&hdoipd, 0, sizeof(t_hdoipd));


    hdoipd.drv = drv;
    hdoipd.reg_listener = 0;
    hdoipd.registry = 0;
    hdoipd.tasklet = queue_create();
    hdoipd.state = HOID_IDLE;
    hdoipd.rsc_state = RSC_VIDEO_IN;
    hdoipd.vtb_state = VTB_OFF;
    hdoipd.vrb_state = VRB_OFF;

    hdoipd.local.vid_port = htons(90);
    hdoipd.local.aud_port = htons(91);
    hdoipd.drivers = 0;//hdoipd.drivers = DRV_ADV9889 | DRV_ADV7441;

    hoi_cfg_init(&hdoipd.config, "/mnt/config/hdoipd0.config");

    pthread_mutex_init(&hdoipd.mutex, NULL);

    report("load drivers...");

    if ((hoi_drv_ldrv(hdoipd.drv, hdoipd.drivers, 0))) {
        report("loading drivers failed");
        return false;
    }

    hdoipd_set_state(HOID_READY);

    report("setup buffers...");

    void* vid = malloc(VID_SIZE);

    if (!vid) {
        report("malloc(VID_SIZE) failed");
        return false;
    }

    if (hoi_drv_buf(hdoipd.drv,
            malloc(AUD_RX_SIZE), AUD_RX_SIZE,
            vid, VID_SIZE,
            malloc(AUD_TX_SIZE), AUD_TX_SIZE,
            vid, VID_SIZE)) {
        report("set buffers failed");
        return false;
    }

    // setup default output
    //report("set osd...");
    //hdoipd_osd_init();
    //osd_printf("Welcome to <HD over IP> - 2010 InES");

    rtsp_listener_start(&hdoipd.listener, 554);

    return true;
}

