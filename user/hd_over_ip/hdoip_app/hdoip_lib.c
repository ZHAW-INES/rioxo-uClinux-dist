/*
 * hdoip_lib.c
 *
 *  Created on: 05.11.2010
 *      Author: alda
 */

#include "hdoip_lib.h"

/** types
 */
typedef struct {
    int id;
    char* str;
} t_key_value;


#define COLOR_FORMAT_COUNT 8
const t_key_value colfmt[] = {
    { vid_cs(CS_RGB), "rgb" },
    { vid_cs(CS_YUV), "yuv" },
    { vid_cs(CS_YUV_ER), "yuver" },
    { vid_cs(CS_X2RGB), "x2rgb" },
    { vid_cs(CS_Y2RGB), "y2rgb" },
    { vid_cs(CS_Z2RGB), "z2rgb" },
    { vid_sm(SM_444), "444" },
    { vid_sm(SM_422), "422" }
};

int get_value(char* str, t_key_value* list, int count)
{
    for (int i=0;i<count;i++) {
        if (strcmp(str, list[i].str) == 0) {
            return list[i].id;
        }
    }
    return -1;
}

int hdoip_lib_color_format(char* str)
{
    return get_value(str, colfmt, COLOR_FORMAT_COUNT);
}

int get_int(char** s)
{
    int i=0;
    char* p=*s;

    while((*p>='0') && (*p<='9')) {
        i = i * 10 + (*p - '0');
        p++;
    }

    *s = p;

    return i;
}


/* Converts a String to a video timing
 *
 * Format (1): 'Width'x'Height'
 *        (2): 'Width'x'Height'p'FPS'
 *
 * ex.: "640x480", "800x600p60"
 */
int hdoip_lib_res(char* s, uint32_t* width, uint32_t* height, uint32_t* fps)
{
    int ret = 1;
    char* rs = s;

    *width = get_int(&s);
    if (*s != 'x') goto error;
    s++;

    *height = get_int(&s);

    // optional fps
    if (*s == 'p') {
        s++;
        *fps = get_int(&s);
    } else {
        *fps = 0;
    }

    // test for end of string
    if (*s != 0) goto error;

    return 0; // SUCCESS

error:
    printf("resolution <%s> not found\n", rs);
    *width = 640;
    *height = 480;
    *fps = 0;

    return 1; // ERROR TODO
}
