/*
 * tag_drv.h
 *
 *  Created on: 14.09.2010
 *      Author: alda
 */

#ifndef TAG_DRV_H_
#define TAG_DRV_H_

#include "i2c_drv.h"


typedef struct {
	t_i2c    		*p_i2c;       	//!< I2C driver
	uint32_t 		devaddr;		//!< device address on i2c
	bool			available;		//!< board available
	uint8_t			buf[256];		//!< ram buffer
} t_tag;


int tag_drv_init(t_tag* handle, t_i2c* p_i2c, uint32_t devaddr);
int tag_drv_inquire(t_tag* handle);
int tag_drv_write(t_tag* handle);

#endif /* TAG_DRV_H_ */
