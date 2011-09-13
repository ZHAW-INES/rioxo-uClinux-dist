/*
 * bdt_drv.h
 *
 *  Created on: 06.07.2011
 *      Author: buan
 */

#ifndef BDT_DRV_H_
#define BDT_DRV_H_

#include "i2c_drv.h"

// I2C Address (0x22 << 1)
#define BDT_ADDRESS     (0x44)

// video mux control
#define MUX_CONTROL_OFFSET                  0x00000000
#define MUX_STATUS_OFFSET                   0x00000004
#define MUX_RESET_OFFSET                    0x00000008

#define INPUT_MUX_IDLE                      0x00000000
#define INPUT_MUX_SDI                       0x00000001
#define INPUT_MUX_HDMI                      0x00000002

#define bdt_set_video_mux(p, v)             HOI_WR32((p), MUX_CONTROL_OFFSET, (v))
#define bdt_get_video_mux_status(p)         HOI_RD32((p), MUX_STATUS_OFFSET)
#define bdt_get_reset_to_default(p)         (HOI_RD32((p), MUX_RESET_OFFSET) & (1))
#define bdt_set_reset_to_default(p)         HOI_WR32((p), MUX_RESET_OFFSET, (0x2))
#define bdt_clr_reset_to_default(p)         HOI_WR32((p), MUX_RESET_OFFSET, (0x0))

//card list is in "hoi_msg.h"

typedef struct {
	t_i2c    		    *p_i2c;       	//!< I2C driver
    uint8_t             device;         // connected card
} t_bdt;

uint32_t bdt_return_device(t_bdt* handle);
void bdt_drv_read_id(t_bdt* handle, t_i2c* p_i2c);
void bdt_drv_set_video_mux(t_bdt* handle, void* p_video_mux);
int  bdt_drv_get_reset_to_default(t_bdt* handle, void* p_video_mux);

#endif /* BDT_DRV_H_ */
