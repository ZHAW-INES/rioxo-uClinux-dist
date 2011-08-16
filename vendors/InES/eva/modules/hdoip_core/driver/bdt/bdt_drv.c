/*
 * bdt_drv.c
 *
 *  Created on: 06.07.2011
 *      Author: buan
 */
#include "bdt_drv.h"

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

int bdt_drv_read_id(t_bdt* handle, t_i2c* p_i2c)
{
    if (p_i2c) handle->p_i2c = p_i2c;

    handle->device           = (bdt_read(handle) >> 4) & 0x0F;
}
