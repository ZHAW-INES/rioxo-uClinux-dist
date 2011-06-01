/*
 * hoi_image.c
 *
 *  Created on: 19.10.2010
 *      Author: alda
 */
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "hoi_image.h"
#include "hoi_res.h"
#include "debug.h"

uint8_t* hoi_image_canvas_create(t_video_timing* timing)
{
    uint32_t* buffer;
    size_t size = timing->width * timing->height * 3;

    buffer = malloc(size + 4);

    if (!buffer) return NULL;

    // first field of output contains size
    buffer[0] = size/4;

    hoi_drv_show(false, buffer, timing, 0);

    return (uint8_t*)buffer + 4;
}

void hoi_image_free(uint8_t* pimage)
{
    if (pimage) {
        free(pimage - 4);
    }
}

void hoi_image_capture_jpeg2000(FILE* out, size_t size)
{
    int ret;
    uint32_t* buffer;
    t_video_timing timing;
    uint32_t advcnt;

    size = size * 4 / 3;

    buffer = malloc(size);

    if (!buffer) return;

    if ((ret = hoi_drv_capture(true, buffer, size, &timing, &advcnt))) {
        report("hoi_drv_capture failed. (error code = %d)", ret);
        return;
    }

    // write 'header', timing, advcnt
    fwrite("ines", 4, 1, out);          // ines file format...
    fwrite("j2kstimg", 8, 1, out);      // jpeg2000 still image
    fwrite(&timing, sizeof(t_video_timing), 1, out);
    fwrite(&advcnt, 4, 1, out);
    fwrite(buffer, buffer[0]*4+4, 1, out);

    free(buffer);
}

void hoi_image_capture(FILE* out)
{
    int ret;
    uint32_t* buffer;
    t_video_timing timing;
    uint32_t advcnt;

    // TODO proper buffer size
    size_t size = 1920*1080*3+16;

    buffer = malloc(size);

    if (!buffer) return;

    if ((ret = hoi_drv_capture(false, buffer, size, &timing, &advcnt))) {
        report("hoi_drv_capture failed. (error code = %d)", ret);
        return;
    }

    // write 'header', timing, advcnt
    fwrite("ines", 4, 1, out);          // ines file format...
    fwrite("rawimage", 8, 1, out);      // raw image
    fwrite(&timing, sizeof(t_video_timing), 1, out);
    fwrite(buffer, buffer[0]*4+4, 1, out);

    free(buffer);
}

uint8_t* hoi_image_load(FILE* in)
{
    int ret;
    char hdr[5], fmt[9];
    uint32_t* buffer;
    t_video_timing timing;
    uint32_t advcnt;
    uint32_t size, fps;

    // Header
    fread(hdr, 4, 1, in); hdr[4] = 0;
    fread(fmt, 8, 1, in); fmt[8] = 0;

    if (strcmp(hdr, "ines") == 0) {
        report("image type: %s\n", fmt);

        if (strcmp(fmt, "j2kstimg") == 0) {
            // jpeg2000 still image
            fread(&timing, sizeof(t_video_timing), 1, in);
            fread(&advcnt, 4, 1, in);
            fread(&size, 4, 1, in);
            if ((buffer = malloc(size*4+4))) {
                buffer[0] = size;
                fread(&buffer[1], size*4, 1, in);
                if ((ret = hoi_drv_show(true, buffer, &timing, advcnt))) {
                    free(buffer);
                    report("hoi_drv_show failed. (error code = %d)\n", ret);
                    return 0;
                }
            } else {
                report("not enough memory for image load\n");
                return 0;
            }
        } else if (strcmp(fmt, "rawimage") == 0) {
            // jpeg2000 still image
            fread(&timing, sizeof(t_video_timing), 1, in);
            fps = hoi_res_fps(&timing);
            if (fps < 60) timing.pfreq = (uint32_t)((uint64_t)timing.pfreq * 60 / fps);
            fread(&size, 4, 1, in);
            if ((buffer = malloc(size*4+4))) {
                buffer[0] = size;
                fread(&buffer[1], size*4, 1, in);
                if ((ret = hoi_drv_show(false, buffer, &timing, 0))) {
                    free(buffer);
                    report("hoi_drv_show failed. (error code = %d)\n", ret);
                    return 0;
                }
            } else {
                report("not enough memory for image load\n");
                return 0;
            }
        } else {
            report("unknown image format\n");
            return 0;
        }

    } else {
        report("unknown fileformat <%s>\n", hdr);
        return 0;
    }

    return (uint8_t*)(&buffer[1]);
}
