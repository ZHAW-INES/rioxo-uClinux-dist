/*
 * esc_drv.h
 *
 * Created on: 20.01.2011
 *     Author: buan
 */

#ifndef ESC_DRV_H_
#define ESC_DRV_H_

#include "std.h"
#include "esc_hal.h"
#include "esc_drv_cfg.h"

int esc_set_pll_1000(t_esc* handle);
int esc_set_pll_100(t_esc* handle);
int esc_set_pll_10(t_esc* handle);
int esc_handler(t_esc* handle);
int esc_init(t_esc* handle, void* ptr, void* mac_ptr);
int esc_read_pll_param(t_esc* handle, int32_t type, int32_t param);
int esc_set_pll_param(t_esc* handle, int32_t type, int32_t param, int32_t data_in);
int esc_reconfig_pll_param(t_esc* handle);

#endif /* ESC_DRV_H_ */
