/*
 * asi_drv.h
 *
 * Created on: 25.10.2010
 *     Author: stth
 */

#ifndef ASI_DRV_H_
#define ASI_DRV_H_

#include "std.h"
#include "asi_hal.h"
#include "asi_drv_cfg.h"
#include "asi_str.h"

/* function prototypes */
int asi_drv_init(t_asi* handle, void* p_asi);
int asi_drv_flush_buf(t_asi* asi);
int asi_drv_handler(t_asi* handle, t_queue* event_queue);
int asi_drv_update(t_asi* handle, int unsigned stream, struct hdoip_eth_params* eth_params, struct hdoip_aud_params* aud_params);
int asi_drv_start(t_asi* handle, int unsigned stream);
int asi_drv_stop(t_asi* handle, int unsigned stream);
int asi_drv_set_eth_params(t_asi* asi, int unsigned stream, struct hdoip_eth_params* eth_params, struct hdoip_aud_params* aud_params);
int asi_drv_get_eth_params(t_asi* handle, int unsigned stream, struct hdoip_eth_params* eth_params);
int asi_drv_set_aud_params(t_asi* handle, int unsigned stream, struct hdoip_aud_params* aud_params);
int asi_drv_get_aud_params(t_asi* handle, int unsigned stream, struct hdoip_aud_params* aud_params);

static inline int asi_drv_get_detected_fs(t_asi* asi, int unsigned stream)
{
    return asi->detected_fs[stream];
}
static inline int asi_drv_get_detected_ch_map(t_asi* asi, int unsigned stream)
{
    return asi->detected_ch_map[stream];
}

#endif /* ASI_DRV_H_ */
