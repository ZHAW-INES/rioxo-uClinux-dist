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
#define OSD_TIMEOUT         10

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
    if (b) {
        hdoipd.osd_timeout = -1;
    } else {
        if ((hdoipd.osd_timeout < 0) || (hdoipd.osd_timeout > OSD_TIMEOUT)) hdoipd.osd_timeout = OSD_TIMEOUT;
    }
}

#endif /* HDOIPD_OSD_H_ */
