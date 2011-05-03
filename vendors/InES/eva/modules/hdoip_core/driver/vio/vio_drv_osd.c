/*
 * vio_drv_osd.c
 *
 *  Created on: 24.08.2010
 *      Author: alda
 */
#include "vio_drv_osd.h"

int vio_drv_osd_seek_(t_osd* p_osd, int32_t off)
{

	return SUCCESS;
}


int vio_drv_osd_write_(void *p, t_osd* p_osd, uint8_t* src, size_t size)
{
	uint32_t w = vio_osd_get_width(p);
	uint32_t h = vio_osd_get_height(p);
	uint32_t x = p_osd->x;
	uint32_t y = p_osd->y;
	uint8_t* screen = vio_osd_screen(p) + x + y * w;
    uint8_t x_border = p_osd->x_border;
    uint8_t y_border = p_osd->y_border;

	for(;size;size--,src++) {
		switch (*src) {
			case '\n':
                for(;x<w;x++,screen++)
                    *screen = ' ';
			break;
			default:
                if (x < x_border) {
                    x = x_border;
                    screen += x_border;
                }
                if (y < y_border) {
                    y = y_border;
                    screen += y_border * w;
                }
				*screen = *src;
				screen++;
                x++;
			break;
		}
		if (x >= (w-x_border)) {
            for(;x<w;x++,screen++)
                *screen = ' ';
			y++;
            x = x_border;
            screen += x_border;
			if (y == (h - x_border)) {
				y--; screen = screen - w;
				vio_osd_move_screen(p, 1);
			}
		}
	}

	p_osd->x = x;
	p_osd->y = y;

	return SUCCESS;
}
