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

void hdoipd_osd_init(t_hdoipd* handle)
{
    t_hoic_canvas cmd;

    cmd.width = 640;
    cmd.height = 480;
    cmd.fps = 60;

    hdoipd_canvas(handle, &cmd);
}

void hdoipd_osd_activate(t_hdoipd* handle)
{
    if (!(handle->rsc_state & RSC_OSD)) {
        hoi_drv_osdon(handle->drv);
        handle->rsc_state |= RSC_OSD;
    }
}
