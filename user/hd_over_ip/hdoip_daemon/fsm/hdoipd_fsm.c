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
#include "hdoipd_msg.h"
#include "hdoipd_osd.h"
#include "hdoipd_fsm.h"
#include "rtsp_include.h"

bool new_active_state(t_hdoipd* handle)
{
    switch (handle->state) {
        case HOID_IDLE:
            printf("hw not ready\n");
            return;
        break;
        case HOID_VTB:
            // shut down VTB
            return true;
        break;
        case HOID_VRB:
            // shut down VRB
            return true;
        break;
        case HOID_READY:
        case HOID_LOOP:
        case HOID_SHOW_CANVAS:
            hoi_image_free(handle->canvas);
            // from this states it is possible to change to a new active state
            return true;
        break;
        default:
            printf("request not supported in state <%08x>\n", handle->state);
            return;
        break;
    }

    return false;
}

void hdoipd_canvas(t_hdoipd* handle, t_hoic_canvas* cmd)
{
    t_video_timing* timing;

    if (new_active_state(handle)) {
        if ((timing = hoi_res_timing(cmd->width, cmd->height, cmd->fps))) {
            hoi_image_canvas_create(handle->drv, timing);
        } else {
            printf("could not start canvas %d x %d @ %d Hz\n", cmd->width, cmd->height, cmd->fps);
        }

        handle->rsc_state |= RSC_VIDEO_OUT;
        handle->state = HOID_SHOW_CANVAS;

    }
}


void hdoipd_load(t_hdoipd* handle, t_hoic_load* cmd)
{
    FILE* f;

    if (new_active_state(handle)) {

        if ((f = fopen(cmd->filename, "r"))) {
            handle->canvas = hoi_image_load(handle->drv, f);
            fclose(f);
        } else {
            printf("open file <%s> failed\n", cmd->filename);
        }
        if (handle->canvas) {
            handle->rsc_state |= RSC_VIDEO_OUT;
            handle->state = HOID_SHOW_CANVAS;
        } else {
            handle->state = HOID_READY;
        }

    }
}

void hdoipd_capture(t_hdoipd* handle, t_hoic_capture* cmd)
{
    FILE* f;

    if (new_active_state(handle)) {

        if ((f = fopen(cmd->filename, "w"))) {
            if (cmd->compress) {
                hoi_image_capture_jpeg2000(handle->drv, f, cmd->size);
            } else {
                hoi_image_capture(handle->drv, f);
            }
            fclose(f);
        } else {
            printf("open file <%s> failed\n", cmd->filename);
        }

        handle->state = HOID_READY;

    }
}

void hdoipd_fmt_in(t_hdoipd* handle, t_hoic_param* cmd)
{
    hoi_drv_ifmt(handle->drv, cmd->value);
}

void hdoipd_fmt_out(t_hdoipd* handle, t_hoic_param* cmd)
{
    hoi_drv_ofmt(handle->drv, cmd->value);
}

void hdoipd_fmt_proc(t_hdoipd* handle, t_hoic_param* cmd)
{
    hoi_drv_pfmt(handle->drv, cmd->value);
}

void hdoipd_loop(t_hdoipd* handle, t_hoic_cmd* cmd)
{
    switch (handle->state) {
        case HOID_IDLE:
            printf("hw not ready\n");
            return;
        break;
        case HOID_READY:
        case HOID_LOOP:
        case HOID_SHOW_CANVAS:
            hoi_drv_loop(handle->drv);
            handle->state = HOID_LOOP;
        break;
        default:
            printf("loop request not supported in state <%08x>\n", handle->state);
            return;
        break;
    }
}

void hdoipd_osd_on(t_hdoipd* handle, t_hoic_cmd* cmd)
{
    hoi_drv_osdon(handle->drv);
    handle->rsc_state |= RSC_OSD;
}

void hdoipd_osd_off(t_hdoipd* handle, t_hoic_cmd* cmd)
{
    hoi_drv_osdoff(handle->drv);
    handle->rsc_state &= ~RSC_OSD;
}

void hdoipd_vtb(t_hdoipd* handle, t_hoic_cmd* cmd)
{
    if (new_active_state(handle)) {
        handle->state = HOID_VTB;
    }
}



void hdoipd_vrb_setup(t_hdoipd* handle, t_hoic_load* cmd)
{
    t_rtsp_transport transport;
    int ret;

    printf("open: %s\n", cmd->filename);

    if (new_active_state(handle)) {

        // open media
        ret = rtsp_client_open(&handle->client, 0, cmd->filename);

        if (ret == RTSP_SUCCESS) {
            rtsp_default_transport(&transport); // TODO: daemon transport configuration
            ret = rtsp_client_setup(&handle->client, &transport);
        }

        if (ret == RTSP_SUCCESS) {
            handle->state = HOID_VRB;
        } else {
            handle->state = HOID_READY;
        }

    }
}


void hdoipd_vrb_play(t_hdoipd* handle, t_hoic_cmd* cmd)
{
    int ret;

    if (handle->state == HOID_VRB) {

        // open media
        ret = rtsp_client_play(&handle->client);

    }
}


#define hdoipdreq(x, y) case x: y(handle, (void*)cmd); break
void hdoipd_request(t_hdoipd* handle, uint32_t* cmd, int rsp)
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
        default: printf("command not supported <%08x>\n", cmd[0]);
    }
}

void hdoipd_event(t_hdoipd* handle, uint32_t event)
{

}

void hdoipd_handler(t_hdoipd* handle)
{

}

#define VID_SIZE        (4*1024*1024)
#define AUD_RX_SIZE     (1024*1024)
#define AUD_TX_SIZE     (1024*1024)

void hdoipd_init(t_hdoipd* handle)
{
    handle->drivers = DRV_ADV9889 | DRV_ADV7441;
    handle->rsc_state = RSC_VIDEO_IN;

    if (!(hoi_drv_ldrv(handle->drv, handle->drivers, 0))) {
        handle->state = HOID_READY;
    } else {
        printf("loading drivers failed\n");
    }

    void* vid = malloc(VID_SIZE);
    hoi_drv_buf(handle->drv,
            malloc(AUD_RX_SIZE), AUD_RX_SIZE,
            vid, VID_SIZE,
            malloc(AUD_TX_SIZE), AUD_TX_SIZE,
            vid, VID_SIZE);

    // setup default output
    hdoipd_osd_init(handle);
    hdoipd_report(handle, "Welcome to <HD over IP> - 2010 InES");

    rtsp_listener_start(&handle->listener, 554);
}

