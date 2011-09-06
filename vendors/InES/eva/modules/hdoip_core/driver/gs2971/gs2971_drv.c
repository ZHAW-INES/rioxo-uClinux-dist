/*
 * gs2971_drv.c
 *
 *  Created on: 02.09.2011
 *      Author: buan
 */

#include "gs2971_drv.h"
#include "spi_drv.h"

void gs2971_driver_init(t_gs2971 *handle, void *spi_ptr, void *i2c_ptr)
{
    handle->p_spi = spi_ptr;
    handle->p_i2c = i2c_ptr;

    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| GS2971-Driver: Initialize SDI-RX                 |");
    REPORT(INFO, "+--------------------------------------------------+");

//    spi_write_reg_16(handle->p_spi, 0x35, 0xAA);
}

void gs2971_handler(t_gs2971 *handle, t_queue *event_queue)
{
	
}
