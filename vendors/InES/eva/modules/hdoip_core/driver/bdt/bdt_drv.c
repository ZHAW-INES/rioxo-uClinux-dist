/*
 * bdt_drv.c
 *
 *  Created on: 06.07.2011
 *      Author: buan
 */
#include "i2c_drv.h"
#include "bdt_drv.h"
#include "hoi_msg.h"

static inline void bdt_write(t_bdt* handle, uint8_t x)
{
    i2c_drv_write(handle->p_i2c, BDT_ADDRESS, x);
}

static inline int bdt_read(t_bdt* handle)
{
    return i2c_drv_read(handle->p_i2c, BDT_ADDRESS);
}

uint32_t bdt_return_device(t_bdt* handle)
{
    return (uint32_t) handle->device;
}

void bdt_drv_read_id(t_bdt* handle, t_i2c* p_i2c)
{
    if (p_i2c) handle->p_i2c = p_i2c;

    handle->device = (bdt_read(handle) >> 4) & 0x0F;
}

void bdt_drv_set_video_mux(t_bdt* handle, void* p_video_mux)
{
    switch (handle->device) {
        case BDT_ID_HDMI_BOARD: bdt_set_video_mux(p_video_mux, INPUT_MUX_HDMI);
                                printk("\nset video mux to HDMI\n");
                                break;
        case BDT_ID_SDI8_BOARD: bdt_set_video_mux(p_video_mux, INPUT_MUX_SDI);
                                printk("\nset video mux to SDI\n");
                                break;
        default:                bdt_set_video_mux(p_video_mux, INPUT_MUX_IDLE);
                                printk("\nset video mux to IDLE\n");
    }
}

int bdt_drv_get_reset_to_default(t_bdt* handle, void* p_video_mux)
{
    int set_to_default = bdt_get_reset_to_default(p_video_mux);

    if (set_to_default) {
        bdt_set_reset_to_default(p_video_mux);
        bdt_clr_reset_to_default(p_video_mux);
    }

    return set_to_default;
}

// disable reset of video card rx chip
void bdt_drv_clear_reset_0(void* p_video_mux)
{
    uint32_t tmp = bdt_get_reset_chip(p_video_mux);
    bdt_set_reset_chip_0(p_video_mux, tmp);
}

// disable reset of video card tx chip
void bdt_drv_clear_reset_1(void* p_video_mux)
{
    uint32_t tmp = bdt_get_reset_chip(p_video_mux);
    bdt_set_reset_chip_1(p_video_mux, tmp);
}
