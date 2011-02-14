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

char osdtmp[OSD_BUFFER_LENGTH];

void hdoipd_osd_deactivate()
{
    //if (hdoipd.rsc_state & RSC_OSD) {
        hoi_drv_osdoff();
        report(CHANGE "osd off");
        hdoipd_clr_rsc(RSC_OSD);
    //}
}

void hdoipd_osd_activate()
{
    t_video_timing* timing;

    if (!(hdoipd.rsc_state & RSC_OSD)) {
        // if we dont have active video IN or Out -> create own resolution
        // else we use the present video timing
        if (!(hdoipd_rsc(RSC_VIDEO_IN | RSC_VIDEO_OUT))) {
            report(CHANGE "activate 640x480 debug output screen for osd");
            if ((timing = hoi_res_timing(640, 480, 60))) {
                hoi_drv_debug(timing);
                hdoipd_set_rsc(RSC_VIDEO_OUT);
            }
        }
        hoi_drv_osdon();
        report(CHANGE "osd on");
        hdoipd_set_rsc(RSC_OSD);
    }
    if (hdoipd.osd_timeout >= 0) hdoipd.osd_timeout = OSD_TIMEOUT;
}

void* hdoipd_osd_timer(void UNUSED *d)
{
    do {
        struct timespec ts = {
            .tv_sec = 1,
            .tv_nsec = 0
        };
        nanosleep(&ts, 0);

        if (hdoipd.osd_timeout > 0) {
            lock("hdoipd_osd_timer");
                if (hdoipd.osd_timeout > 0) {
                    hdoipd.osd_timeout--;
                    if (hdoipd.osd_timeout == 0) {
                        hdoipd_osd_deactivate();
                    }
                }
            unlock("hdoipd_osd_timer");
        }

        lock("hdoipd_tick_timer");
            hdoipd.tick++;
            rscp_client_event(hdoipd.client, EVENT_TICK);
            rscp_listener_event(&hdoipd.listener, EVENT_TICK);
        unlock("hdoipd_tick_timer");

    } while (1);
}

void hdoipd_osd_timer_start()
{
    pthread_t th;
    pthread(th, hdoipd_osd_timer, 0);
}
