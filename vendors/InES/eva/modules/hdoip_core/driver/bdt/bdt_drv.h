/*
 * bdt_drv.h
 *
 *  Created on: 06.07.2011
 *      Author: buan
 */
#include "aso_drv_cfg.h"

#ifndef BDT_DRV_H_
#define BDT_DRV_H_

// I2C Address (0x22 << 1)
#define BDT_ADDRESS     (0x44)

// video mux control
#define MUX_CONTROL_OFFSET                  0x00000000
#define MUX_RESET_CHIP_OFFSET               0x00000004
#define MUX_RESET_OFFSET                    0x00000008

#define INPUT_MUX_IDLE                      0x00000000
#define INPUT_MUX_SDI                       0x00000001
#define INPUT_MUX_HDMI                      0x00000002

#define RESET_CHIP_1                        0x00000001
#define RESET_CHIP_2                        0x00000002

#define bdt_set_video_mux(p, v)             HOI_WR32((p), MUX_CONTROL_OFFSET, ((HOI_RD32((p), MUX_CONTROL_OFFSET) & 0xFFFFFFF0) | (v & 0x0000000F)))
#define bdt_get_video_mux_status(p)         HOI_RD32((p), MUX_STATUS_OFFSET)

#define bdt_get_reset_to_default(p)         (HOI_RD32((p), MUX_RESET_OFFSET) & (1))
#define bdt_set_reset_to_default(p)         HOI_WR32((p), MUX_RESET_OFFSET, (0x2))
#define bdt_clr_reset_to_default(p)         HOI_WR32((p), MUX_RESET_OFFSET, (0x0))

#define bdt_get_reset_button(p)             (HOI_RD32((p), MUX_RESET_OFFSET) & (4))
#define bdt_set_reset_button(p)             HOI_WR32((p), MUX_RESET_OFFSET, (0x8))
#define bdt_clr_reset_button(p)             HOI_WR32((p), MUX_RESET_OFFSET, (0x0))

#define bdt_get_reset_chip(p)               HOI_RD32((p), MUX_RESET_CHIP_OFFSET)
#define bdt_set_reset_chip_0(p, v)          HOI_WR32((p), MUX_RESET_CHIP_OFFSET, ((v) | RESET_CHIP_1))
#define bdt_set_reset_chip_1(p, v)          HOI_WR32((p), MUX_RESET_CHIP_OFFSET, ((v) | RESET_CHIP_2))

//card list is in "hoi_msg.h"

typedef struct {
	t_i2c    		    *p_i2c;       	//!< I2C driver
    uint8_t             device_video;   // connected video card
	uint8_t				device_audio;	// connected audio card
} t_bdt;

uint32_t bdt_return_video_device(t_bdt* handle);
uint32_t bdt_return_audio_device(t_bdt* handle);
void bdt_drv_read_video_id(t_bdt* handle, t_i2c* p_i2c);
void bdt_drv_read_audio_id(t_bdt* handle, void* p_aso);
void bdt_drv_set_video_mux(t_bdt* handle, void* p_video_mux);
int  bdt_drv_get_reset_to_default(t_bdt* handle, void* p_video_mux);
void bdt_drv_clear_reset_0(void* p_video_mux);
void bdt_drv_clear_reset_1(void* p_video_mux);
void bdt_drv_handler(void* p_video_mux, t_queue* event);

#endif /* BDT_DRV_H_ */
