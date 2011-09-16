/*
 * hdoipd_osd.h
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */

#ifndef HDOIPD_OSD_H_
#define HDOIPD_OSD_H_

#include <stdio.h>
#include "hdoipd.h"

#define OSD_BUFFER_LENGTH   256

void hdoipd_osd_deactivate();
void hdoipd_osd_activate();
void hdoipd_osd_timer_start();

extern char osdtmp[OSD_BUFFER_LENGTH];

#define osd_printf(...) \
{ \
    hdoipd_osd_activate(); \
    int n = snprintf(osdtmp, OSD_BUFFER_LENGTH, __VA_ARGS__); \
    write(hdoipd.drv, osdtmp, n); \
}

static inline void osd_permanent(bool b)
{
    int osd_timeout = reg_get_int("osd-time");

    if (b) {
        hdoipd.osd_timeout = -1;
    } else {
        if ((hdoipd.osd_timeout < 0) || (hdoipd.osd_timeout > osd_timeout)) hdoipd.osd_timeout = osd_timeout;
    }
}

#endif /* HDOIPD_OSD_H_ */
