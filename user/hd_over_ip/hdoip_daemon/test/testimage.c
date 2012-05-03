/*
 * testimage.c
 *
 *  Created on: 05.12.2011
 *      Author: buan
 */

#include "hdoipd.h"
#include "testimage.h"
#include "hdoipd_fsm.h"
#include "hoi_drv_user.h"
#include "hoi_res.h"
#include "hdoipd_osd.h"

#define pattern_width   18
#define pattern_height  16

#define black   0x000000
#define white   0xFFFFFF
#define gray    0xAAAAAA
#define blue    0xFF0000
#define green   0x00FF00
#define red     0x0000FF

/** creates test-image from pattern
 *
 *  Size is given by "width" and "height"
 *  Image is stored in "buffer"
 *
 */
void create_test_image(uint32_t* buffer, uint32_t width, uint32_t height)
{
    uint32_t x, y;
    uint32_t pixel_count = 0;
    uint32_t tmp[4];
    char pattern[pattern_height][pattern_width] = {{'F', '0', '0', 'F', '0', '0', 'F', '0', '0', 'F', 'F', 'F', 'F', 'F', 'F', '0', '0', '0'},
                                                   {'F', '0', '0', 'F', '0', '0', 'F', '0', '0', '0', '0', '0', '0', '0', '0', 'F', '0', '0'},
                                                   {'F', '0', '0', 'F', '0', '0', 'F', '0', '0', '0', '0', '0', '0', '0', '0', 'F', '0', '0'},
                                                   {'F', '0', '0', 'F', '0', '0', 'F', '0', '0', 'F', 'F', 'F', 'F', 'F', 'F', 'F', '0', '0'},
                                                   {'F', '0', '0', 'F', '0', '0', 'F', '0', '0', '0', '0', '0', '0', '0', '0', 'F', '0', '0'},
                                                   {'F', '0', '0', 'F', '0', '0', 'F', '0', '0', '0', '0', '0', '0', '0', '0', 'F', '0', '0'},
                                                   {'0', 'F', 'F', 'F', 'F', 'F', '0', '0', '0', 'F', 'F', 'F', 'F', 'F', 'F', '0', '0', '0'},
                                                   {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'},
                                                   {'0', 'F', 'F', 'F', 'F', 'F', 'F', '0', '0', '0', 'F', 'F', 'F', 'F', 'F', '0', '0', '0'},
                                                   {'F', '0', '0', '0', '0', '0', '0', '0', '0', 'F', '0', '0', 'F', '0', '0', 'F', '0', '0'},
                                                   {'F', '0', '0', '0', '0', '0', '0', '0', '0', 'F', '0', '0', 'F', '0', '0', 'F', '0', '0'},
                                                   {'F', 'F', 'F', 'F', 'F', 'F', 'F', '0', '0', 'F', '0', '0', 'F', '0', '0', 'F', '0', '0'},
                                                   {'F', '0', '0', '0', '0', '0', '0', '0', '0', 'F', '0', '0', 'F', '0', '0', 'F', '0', '0'},
                                                   {'F', '0', '0', '0', '0', '0', '0', '0', '0', 'F', '0', '0', 'F', '0', '0', 'F', '0', '0'},
                                                   {'0', 'F', 'F', 'F', 'F', 'F', 'F', '0', '0', 'F', '0', '0', 'F', '0', '0', 'F', '0', '0'},
                                                   {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'}};

    for (y=0;y<height;y++) {
        for (x=0;x<width;x++) {

            switch (pattern[y%pattern_height][x%pattern_width]) {
                case '0':       tmp[x%4] = black;
                                break;
                case 'F':       tmp[x%4] = white;
                                break;
                default:        tmp[x%4] = black;
            }

            if (((x%4) == 3)) {

                buffer[pixel_count+2] = (((tmp[3] << 8)  & 0xFFFFFF00) | ((tmp[2] >> 16) & 0x000000FF));
                buffer[pixel_count+1] = (((tmp[2] << 16) & 0xFFFF0000) | ((tmp[1] >> 8)  & 0x0000FFFF));
                buffer[pixel_count+0] = (((tmp[1] << 24) & 0xFF000000) | ((tmp[0] >> 0)  & 0x00FFFFFF));
                pixel_count += 3;
            }
        }
    }
}

/** show test-image
 *
 *  Buffer for test-image is shared with compressed video
 *
 */
int testimage_show()
{
    uint32_t* buffer;
    t_video_timing* timing;
    uint32_t size;

    int width = 1920;
    int height = 1080;
    int fps = 24;

    if (!hdoipd_goto_ready()) {
        return 1;
    }

    size = width * height;

    buffer = hdoipd.img_buff;

    buffer[0] = (size * 3 / 4);

    if ((timing = hoi_res_timing(width, height, fps))) {

        create_test_image((uint32_t*)&buffer[1], width, height);

        if (hoi_drv_show(false, buffer, timing, 0)) {
            report (ERROR "testimage: hoi_drv_show failed");
            return 1;
        }

        hdoipd.canvas = (uint8_t*)(&buffer[1]);
        hdoipd.rsc_state |= RSC_VIDEO_OUT;
        hdoipd_set_state(HOID_SHOW_CANVAS);

        hdoipd_osd_deactivate();
    } else {
        report (ERROR "testimage: cannot find resolution: %ix%i@%iHz", width, height, fps);
        return 1;
    }

    return 0;
}
