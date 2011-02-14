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
#include "queue.h"

typedef struct {
	t_i2c    		*p_i2c;       	//!< I2C driver
	uint32_t        edid_segment;   //!< address offset of edid
	uint32_t        edid_first;     //!< first edid segment read
} t_adv9889;


int adv9889_drv_init(t_adv9889* handle, t_i2c* p_i2c);
int adv9889_irq_handler(t_adv9889* handle, t_queue* event_queue);

int adv9889_drv_get_edid(t_adv9889* handle, uint8_t* mem);

#endif /* ADV9889_DRV_H_ */
