/*
 * vsi_drv.h
 *
 * Created on: 05.10.2010
 *     Author: stth
 */

#ifndef VSI_DRV_H_
#define VSI_DRV_H_

#include "std.h"
#include "vsi_hal.h"
#include "vsi_drv_cfg.h"
#include "vsi_str.h"
#include "stdeth.h"

int vsi_drv_init(t_vsi* handle, void* p_vsi);
int vsi_drv_update(t_vsi* handle, struct hdoip_eth_params* eth_params);
int vsi_drv_handler(t_vsi* handle, t_queue* event_queue);
int vsi_drv_set_eth_params(t_vsi* handle, struct hdoip_eth_params* eth_params);
int vsi_drv_get_eth_params(t_vsi* handle, struct hdoip_eth_params* eth_params);
int vsi_drv_start(t_vsi* handle);
int vsi_drv_stop(t_vsi* handle);

#endif /* VSI_DRV_H_ */
