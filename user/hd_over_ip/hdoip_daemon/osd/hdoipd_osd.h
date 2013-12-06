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
void hdoipd_osd_activate(int res);
void hdoipd_osd_timer_start();

extern char osdtmp[OSD_BUFFER_LENGTH];

#define osd_printf(...) \
{ \
    hdoipd_osd_activate(0); \
    int n = snprintf(osdtmp, OSD_BUFFER_LENGTH, "      " __VA_ARGS__); \
    write(hdoipd.drv, osdtmp, n); \
}

#define osd_printf_testpattern_focus_1080p60() \
{ \
    char *p; \
    int i; \
    hdoipd_osd_deactivate(); \
    hdoipd_clr_rsc(RSC_VIDEO_OUT); \
    hdoipd_osd_activate(108060); \
    hoi_drv_osd_clr_border(); \
    if((p = malloc(0x8000)) != NULL) { \
        for (i=0; i<(0x8000); i+=2) { \
            p[i]   = 128; \
            p[i+1] = 129; \
        } \
        write(hdoipd.drv, p, (0x8000-27)); \
        free(p); \
    } \
}

#define osd_printf_testpattern_focus_1080p24() \
{ \
    char *p; \
    int i; \
    hdoipd_osd_deactivate(); \
    hdoipd_clr_rsc(RSC_VIDEO_OUT); \
    hdoipd_osd_activate(108024); \
    hoi_drv_osd_clr_border(); \
    if((p = malloc(0x8000)) != NULL) { \
        for (i=0; i<(0x8000); i+=2) { \
            p[i]   = 128; \
            p[i+1] = 129; \
        } \
        write(hdoipd.drv, p, (0x8000-27)); \
        free(p); \
    } \
}

#define osd_printf_testpattern_focus_720p60() \
{ \
    char *p; \
    int i; \
    hdoipd_osd_deactivate(); \
    hdoipd_clr_rsc(RSC_VIDEO_OUT); \
    hdoipd_osd_activate(72060); \
    hoi_drv_osd_clr_border(); \
    if((p = malloc(0x8000)) != NULL) { \
        for (i=0; i<(0x8000); i+=2) { \
            p[i]   = 128; \
            p[i+1] = 129; \
        } \
        write(hdoipd.drv, p, (0x8000-27)); \
        free(p); \
    } \
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
