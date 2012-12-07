/*
 * vio_drv_osd.h
 *
 *  Created on: 24.08.2010
 *      Author: alda
 */

#ifndef VIO_DRV_OSD_H_
#define VIO_DRV_OSD_H_

#include "vio_hal.h"

/** Handle
 */
typedef struct {
	uint32_t	x, y;			//!< current position
    uint8_t     x_border;
    uint8_t     y_border;
} t_osd;

int vio_drv_osd_seek_(t_osd* p_osd, int32_t off);
void vio_drv_osd_set_border(t_osd* p_osd);
void vio_drv_osd_clr_border(t_osd* p_osd);
int vio_drv_osd_write_(void* p, t_osd* p_osd, uint8_t* src, size_t size);

#endif /* VIO_DRV_OSD_H_ */
