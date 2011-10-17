/*
 * gs2972_drv.h
 *
 *  Created on: 02.09.2011
 *      Author: buan
 */

#ifndef GS2972_DRV_H_
#define GS2972_DRV_H_

#include "gs2972_reg.h"
#include "spi_drv.h"


typedef struct {
    void        *p_spi;
    void        *p_i2c;
} t_gs2972;

void gs2972_driver_init(t_gs2972 *handle, void *spi_ptr, void *i2c_ptr, void *video_mux_ptr);
void gs2972_handler(t_gs2972 *handle, t_queue *event_queue);
void gs2972_driver_set_data_rate(t_gs2972 *handle, uint32_t pfreq);
void gs2972_debug(t_gs2972 *handle);

#endif /* GS2972_DRV_H_ */
