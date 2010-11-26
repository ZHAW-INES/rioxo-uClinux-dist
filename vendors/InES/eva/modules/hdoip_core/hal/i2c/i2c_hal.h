/*
 * i2c_hal.h
 *
 *  Created on: 20.08.2010
 *      Author: alda
 */

#ifndef I2C_HAL_H_
#define I2C_HAL_H_

#include "stdhal.h"
#include "i2c_reg.h"

// Clock Frequency to Prescaler
#define I2C_PRESCALE(x, y)          ((uint16_t)((x) / (5*(y))) - 1)

static inline int16_t i2c_get_prescale(void* p)
{
	uint16_t tmp;
	tmp = HOI_RD32(p, I2C_OFF_PRESCALE_LSB) & 0xff;
	tmp = tmp | ((HOI_RD32(p, I2C_OFF_PRESCALE_MSB) & 0xff) << 8);
	return tmp;
}

static inline void i2c_set_prescale(void* p, int16_t prescale)
{
	HOI_WR32(p, I2C_OFF_PRESCALE_LSB, prescale & 0xff);
	HOI_WR32(p, I2C_OFF_PRESCALE_MSB, (prescale >> 8) & 0xff);
}


int i2c_init(void* p, uint16_t prescale);
int i2c_write(void* p, uint8_t address, size_t size, uint8_t* data);
int i2c_read(void* p, uint8_t address, size_t size, uint8_t* data);
int i2c_register_write(void* p, uint8_t device, uint8_t address, size_t size, uint8_t* data);
int i2c_register_read(void* p_i2c, uint8_t device, uint8_t address, size_t size, uint8_t* data);

#endif /* I2C_HAL_H_ */
