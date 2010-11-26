#ifndef VID_CONST_H_
#define VID_CONST_H_

#include "stdvid.h"

/** extern data
 */
extern const t_video_timing VFORMAT[];

extern const t_color_transform xyz_one;
extern const t_color_transform rgb2yuv;
extern const t_color_transform yuv2rgb;
extern const t_color_transform rgb2y;
extern const t_color_transform x2gray;
extern const t_color_transform y2gray;
extern const t_color_transform z2gray;

extern const void* coltf[3][6];

extern const t_osd_font vid_font_8x13;


#endif /*VID_CONST_H_*/
