/*
 * gs2972_drv.c
 *
 *  Created on: 02.09.2011
 *      Author: buan
 */

#include "gs2972_drv.h"

void gs2972_driver_init(t_gs2972 *handle, void *spi_ptr, void *i2c_ptr)
{
    handle->p_spi = spi_ptr;
    handle->p_i2c = i2c_ptr;
}

void gs2972_handler(t_gs2972 *handle, t_queue *event_queue)
{
	
}
