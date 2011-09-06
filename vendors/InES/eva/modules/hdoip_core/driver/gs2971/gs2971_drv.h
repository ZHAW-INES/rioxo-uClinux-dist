/*
 * gs2971_drv.h
 *
 *  Created on: 02.09.2011
 *      Author: buan
 */

#ifndef GS2971_DRV_H_
#define GS2971_DRV_H_

#include "gs2971_reg.h"
#include "spi_drv.h"


typedef struct {
    void        *p_spi;
    void        *p_i2c;
} t_gs2971;

void gs2971_driver_init(t_gs2971 *handle, void* spi_ptr, void* i2c_ptr);
void gs2971_handler(t_gs2971 *handle, t_queue *event_queue);

#endif /* GS2971_DRV_H_ */
