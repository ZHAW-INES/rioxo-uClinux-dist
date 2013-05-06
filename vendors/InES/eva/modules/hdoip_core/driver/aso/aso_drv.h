/*
 * aso_drv.h
 *
 * Created on: 01.11.2010
 *     Author: stth
 */

#ifndef ASO_DRV_H_
#define ASO_DRV_H_

#include "std.h"
#include "stdaud.h"
#include "aso_hal.h"
#include "aso_drv_cfg.h"
#include "aso_str.h"

int aso_drv_init(t_aso* aso, void* p_aso, int unsigned stream_nr);
int aso_drv_flush_buf(t_aso* handle);
int aso_drv_handler(t_aso* handle, t_queue* event_queue);
int aso_drv_update(t_aso* aso, struct hdoip_aud_params* aud_params, uint32_t aud_delay_us, uint16_t config);
int aso_drv_start(t_aso* handle);
int aso_drv_stop(t_aso* handle);
int aso_drv_set_aud_params(t_aso* handle, struct hdoip_aud_params* aud_params);
int aso_drv_get_aud_params(t_aso* handle, struct hdoip_aud_params* aud_params);
int aso_drv_set_aud_delay(t_aso* handle,  uint32_t aud_delay_us);
int aso_drv_get_aud_delay(t_aso* handle,  uint32_t* aud_delay_us);
uint32_t aso_drv_get_aud_id(void* p_aso);

#endif /* ASO_DRV_H_ */
