/*
 * i2c_drv.c
 *
 *  Created on: 20.08.2010
 *      Author: alda
 */

#include "i2c_drv.h"

/** Initialize i2c master
 *
 * @param p i2c base address
 * @param freq frequency in HZ of interface
 */
int i2c_drv_init(t_i2c* handle, void* p, int freq)
{
	handle->p = p;

	i2c_init(p, I2C_PRESCALE(SFREQ, freq));

	I2C_REPORT(p);

	return 0;
}
