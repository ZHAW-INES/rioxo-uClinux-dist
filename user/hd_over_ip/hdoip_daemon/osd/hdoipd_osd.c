/*
 * hdoipd_osd.c
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */
#include "hoi_drv_user.h"
#include "hdoipd.h"
#include "hdoipd_msg.h"
#include "hdoipd_osd.h"
#include "hdoipd_fsm.h"
#include "hoi_image.h"
#include "hoi_res.h"
#include "rscp_string.h"
#include "hdoip_log.h"

char osdtmp[OSD_BUFFER_LENGTH];

void hdoipd_osd_deactivate()
{
    //if (hdoipd.rsc_state & RSC_OSD) {
        hoi_drv_osdoff();
        report(CHANGE "osd off");
        hdoipd_clr_rsc(RSC_OSD);
    //}
}

void hdoipd_osd_activate(int res)
{
    t_video_timing* timing;
    int osd_timeout = reg_get_int("osd-time");
    int h_pixel, v_pixel, fps;

    if (res == 0) {
        if (hdoipd.drivers & DRV_GS2972) {
            h_pixel = 1280;
            v_pixel = 720;
            fps = 60;
        } else {
            h_pixel = 640;
            v_pixel = 480;
            fps = 60;
        }
    } else {
        h_pixel = 1920;
        v_pixel = 1080;
        fps = 24;
    }

    if (!(hdoipd.rsc_state & RSC_OSD)) {
        // if we dont have active video IN or Out -> create own resolution
        // else we use the present video timing
        if (!(hdoipd_rsc(RSC_VIDEO_IN | RSC_VIDEO_OUT))) {
            report(CHANGE "activate %dx%d@%d debug output screen for osd", h_pixel, v_pixel, fps);
            if ((timing = hoi_res_timing(h_pixel, v_pixel, fps))) {
                hoi_drv_set_timing(timing);
                hdoipd_set_rsc(RSC_VIDEO_OUT);
            }
        }
        hoi_drv_osdon();
        report(CHANGE "osd on");
        hdoipd_set_rsc(RSC_OSD);
    }
    if (hdoipd.osd_timeout >= 0) hdoipd.osd_timeout = osd_timeout;
}

void* hdoipd_osd_timer(void UNUSED *d)
{
    do {
        struct timespec ts = {
            .tv_sec = OSD_TIMER_INTERVAL_SEC,
            .tv_nsec = OSD_TIMER_INTERVAL_NSEC
        };
        nanosleep(&ts, 0);

        lock("hdoipd_osd_timer");
        if (hdoipd.osd_timeout > 0) {
            hdoipd.osd_timeout--;
        }
        if (hdoipd.osd_timeout == 0) {
            if (hdoipd_rsc(RSC_OSD)) {
                hdoipd_osd_deactivate();
            }
        }
        unlock("hdoipd_osd_timer");

        lock("hdoipd_tick_timer");
        // AMX handler
        alive_check_client_handler(&hdoipd.amx, reg_get("amx-hello-msg"));
        // initialize alive check if socket not exists
        alive_check_init_msg_vrb_alive();
        alive_check_handle_msg_vrb_alive(&hdoipd.alive_check);

        // USB handler
        usb_device_handler(&hdoipd.usb_devices);

        hdoipd.tick++;
#ifdef USE_SYS_TICK
        rscp_client_event(hdoipd.client, EVENT_TICK);
        rscp_listener_event(&hdoipd.listener, EVENT_TICK);
#endif
        // Log file handler
        hdoip_log_handler(&hdoipd.main_log);
        hdoip_log_handler(&hdoipd.rscp_log);

        unlock("hdoipd_tick_timer");

    } while (1);
}

void hdoipd_osd_timer_start()
{
    pthread_t th;
    pthread(th, hdoipd_osd_timer, 0);
}
