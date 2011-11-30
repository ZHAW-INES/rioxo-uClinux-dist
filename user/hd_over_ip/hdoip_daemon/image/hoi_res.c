/*
 * hoi_res.c
 *
 *  Created on: 20.10.2010
 *      Author: alda
 */
#include "hoi_res.h"


/** Defines
 */
// progressive/interlaced
#define P               VID_PROGRESSIVE
#define I               VID_INTERLACED
#define H               VID_POL_HIGH
#define L               VID_POL_LOW


/******************************************************************************\
 * ID    | Video Mode ID
 * H     | Active Horizontal Pixel
 * V     | Active Vertical Pixel
 * PFREQ | Pixel frequency in [HZ]
 * HFP   | Horizontal Front Porch in [clk]
 * HP    | Horizontal Sync Pulse in [clk]
 * HBP   | Horizontal Back Porch in [clk]
 * HPOL  | Horizontal Pulse Polarity (H: Active High; L: Active Low)
 * VFP   | Vertical Front Porch in [line]
 * VP    | Vertical Sync Pulse in [line]
 * VBP   | Vertical Back Porch in [line]
 * VPOL  | Vertical Pulse Polarity (H: Active High; L: Active Low)
 * INTL  | I: Interlace; P: Progressive)
\******************************************************************************/

/** video timing table
 *
 *  +--+-----+-----+----------+-----+----+-----+------+-----+----+-----+------+------+
 *  |ID|   H |   V | PFREQ    | HFP | HP | HBP | HPOL | VFP | VP | VBP | VPOL | INTL |
 *  +--+-----+-----+----------+-----+----+-----+------+-----+----+-----+------+------+
 */
#define FORMAT_COUNT 12
const t_video_timing VFORMAT[] = {
    { 0,  640,  480,  25000000,   16,  96,   48,     H,   11,   2,   32,   H,   P},
    { 1,  800,  600,  40000000,   40, 128,   88,     H,    1,   4,   23,   H,   P},
    { 2, 1024,  768,  65000000,   24, 136,  160,     H,    3,   6,   29,   H,   P},
    { 3, 1152,  864,  81624000,   64, 120,  184,     H,    1,   3,   27,   H,   P},
    { 4, 1280,  960, 102103680,   80, 136,  216,     H,    1,   3,   30,   H,   P},
    { 5, 1280, 1024, 108000000,   48, 112,  248,     H,    1,   3,   38,   H,   P},
    { 6, 1600, 1200, 162000000,   64, 192,  304,     H,    1,   3,   46,   H,   P},
    { 7,  720,  576,  27000000,   13,  64,   67,     L,    5,   5,   39,   L,   P},
    { 8,  720,  480,  27000000,   16,  62,   60,     L,    9,   6,   30,   L,   P},
    { 9, 1280,  720,  74250000,  110,  40,  220,     H,    5,   5,   20,   H,   P},
    {10, 1920, 1080, 148500000,   88,  44,  148,     H,    4,   5,   36,   H,   P},
    {11, 1920, 1080,  74250000,  638,  44,  148,     H,    4,   5,   36,   H,   P}
};

inline int intabs(int a, int b)
{
    int c = a - b;
    if (c<0) c=-c;
    return c;
}


/** get frames per second from the timing
 *
 * @param p_vt the video timing struct
 * @return frames per second
 */
int hoi_res_fps(t_video_timing* p_vt)
{
    int h, v, p;

    h = p_vt->hfront + p_vt->hpulse + p_vt->hback + p_vt->width;
    v = p_vt->vfront + p_vt->vpulse + p_vt->vback + p_vt->height;

    p = v * h;

    return ((int)p_vt->pfreq + (p/2)) / p;
}

/* Converts a String to a video timing
 *
 * Format (1): 'Width'x'Height'
 *        (2): 'Width'x'Height'p'FPS'
 *
 * ex.: "640x480", "800x600p60"
 */
t_video_timing* hoi_res_timing(uint32_t width, uint32_t height, uint32_t fps)
{
    t_video_timing* ret = 0;

    for (int i=0;i<FORMAT_COUNT;i++) {
        if ((width == VFORMAT[i].width) &&
            (height == VFORMAT[i].height)) {
            // found resolution
            if (!ret) {
                ret = (t_video_timing*)&VFORMAT[i];
            } else if (fps) {
                if (intabs(hoi_res_fps((t_video_timing*)&VFORMAT[i]), fps) < intabs(hoi_res_fps(ret), fps) ) {
                    ret = (t_video_timing*)&VFORMAT[i];
                }
            }
        }
    }

    return ret;
}


