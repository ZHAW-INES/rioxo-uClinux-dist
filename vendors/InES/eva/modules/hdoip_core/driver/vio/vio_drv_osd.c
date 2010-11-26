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

	for(;size;size--,src++) {
		switch (*src) {
			case '\n':
                for(;x<w;x++,screen++)
                    *screen = ' ';
			break;
			default:
				*screen = *src;
				screen++; x++;
			break;
		}
		if (x == w) {
			y++; x = 0;
			if (y == h) {
				y--; screen = screen - w;
				vio_osd_move_screen(p, 1);
			}
		}
	}

	p_osd->x = x;
	p_osd->y = y;

	return SUCCESS;
}
