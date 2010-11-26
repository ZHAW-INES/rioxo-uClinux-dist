/*
 * adv9889_drv.h
 *
 *  Created on: 20.08.2010
 *      Author: alda
 */

#ifndef ADV9889_DRV_H_
#define ADV9889_DRV_H_

#include "i2c_drv.h"
#include "adv9889_reg.h"


typedef struct {
	t_i2c    		*p_i2c;       	//!< I2C driver
} t_adv9889;


int adv9889_drv_init(t_adv9889* handle, t_i2c* p_i2c);

#endif /* ADV9889_DRV_H_ */
