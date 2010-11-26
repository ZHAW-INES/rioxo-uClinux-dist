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
#include "hdoipd_fsm.h"


static bool new_active_state(t_hdoipd* handle)
{
    switch (handle->state) {
        case HOID_IDLE:
            printf("hw not ready\n");
            return;
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
            printf("could not start canvas");
        }

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
}

void hdoipd_osd_off(t_hdoipd* handle, t_hoic_cmd* cmd)
{
    hoi_drv_osdoff(handle->drv);
}


void hdoipd_web_debug(t_hdoipd* handle, t_hoic_web_debug* cmd) 
{
    printf("hdoipd_web_debug\n");
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
        hdoipdreq(HOIC_WEB_DEBUG, hdoipd_web_debug);
        default: printf("command not supported <%08x>\n", cmd[0]);
    }
}

void hdoipd_event(t_hdoipd* handle, uint32_t event)
{

}

void hdoipd_handler(t_hdoipd* handle)
{

}

void hdoipd_init(t_hdoipd* handle)
{
    handle->drivers = DRV_ADV9889 | DRV_ADV7441;

    if (!(hoi_drv_ldrv(handle->drv, handle->drivers, 0))) {
        handle->state = HOID_READY;
    } else {
        printf("loading drivers failed\n");
    }

}

