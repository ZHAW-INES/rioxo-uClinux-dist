/*
 * vrp_drv.h
 *
 *  Created on: 04.10.2010
 *      Author: alda
 */

#ifndef VRP_DRV_H_
#define VRP_DRV_H_

#include "vrp_hal.h"
#include "vio_drv.h"


typedef struct {
    t_vio   *vio;
    void    *p_vrp;
} t_vrp;


void vrp_drv_init(t_vrp* handle, t_vio* vio, void* p_vrp);
int vrp_drv_off(t_vrp* handle);
int vrp_drv_capture_image(t_vrp* handle, void* buffer, size_t size);
int vrp_drv_show_image(t_vrp* handle, void* buffer, t_video_timing* p_vt);
int vrp_drv_capture_jpeg2000(t_vrp* handle, void* buffer, size_t size, size_t bandwidth);
int vrp_drv_show_jpeg2000(t_vrp* handle, void* buffer, t_video_timing* p_vt, int advcnt);

#endif /* VRP_DRV_H_ */
