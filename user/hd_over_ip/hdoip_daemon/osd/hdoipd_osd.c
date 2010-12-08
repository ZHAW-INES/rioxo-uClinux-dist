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

void hdoipd_osd_init()
{
    t_hoic_canvas cmd;

    cmd.width = 640;
    cmd.height = 480;
    cmd.fps = 60;

    hdoipd_canvas(&cmd);
}

void hdoipd_osd_activate()
{
    if (!(hdoipd.rsc_state & RSC_OSD)) {
        hoi_drv_osdon(hdoipd.drv);
        hdoipd.rsc_state |= RSC_OSD;
    }
}
