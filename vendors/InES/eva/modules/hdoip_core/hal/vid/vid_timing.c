#include "stdvid.h"


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
const t_video_timing VFORMAT[] = {
    { 0,  640,  480,  25000000,   16,  96,   48,     H,   11,   2,   32,   H,   P},
    { 0,  800,  600,  40000000,   40, 128,   88,     H,    1,   4,   23,   H,   P},
    { 0, 1024,  768,  65000000,   24, 136,  160,     H,    3,   6,   29,   H,   P},
    { 0, 1152,  864,  81624000,   64, 120,  184,     H,    1,   3,   27,   H,   P},
    { 0, 1280,  960, 102103680,   80, 136,  216,     H,    1,   3,   30,   H,   P},
    { 0, 1280, 1024, 108000000,   48, 112,  248,     H,    1,   3,   38,   H,   P},
    { 0, 1600, 1200, 162000000,   64, 192,  304,     H,    1,   3,   46,   H,   P},
    { 0,  720,  576,  27000000,   13,  64,   67,     L,    5,   5,   39,   L,   P},
    { 0,  720,  480,  27000000,   16,  62,   60,     L,    9,   6,   30,   L,   P}
};
