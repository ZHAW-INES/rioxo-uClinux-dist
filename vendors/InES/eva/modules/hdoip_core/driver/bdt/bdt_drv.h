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

//card list is in "hoi_msg.h"

typedef struct {
	t_i2c    		    *p_i2c;       	//!< I2C driver
    uint8_t             device;         // connected card
} t_bdt;

uint32_t bdt_return_device(t_bdt* handle);
int bdt_drv_read_id(t_bdt* handle, t_i2c* p_i2c);

#endif /* BDT_DRV_H_ */
