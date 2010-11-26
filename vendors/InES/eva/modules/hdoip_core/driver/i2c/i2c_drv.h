/*
 * i2c_drv.h
 *
 *  Created on: 20.08.2010
 *      Author: alda
 */

#ifndef I2C_DRV_H_
#define I2C_DRV_H_

#include "i2c_hal.h"
#include "i2c_str.h"


typedef struct {
    void*    		p;        	//!< I2C Hardware pointer
    spinlock_t		sem;		//!< I2C Hardware access
    uint32_t        error;      //!< I2C error code
} t_i2c;


int i2c_drv_init(t_i2c* handle, void* p, int freq);


static inline void i2c_drv_lock(t_i2c* handle)
{
	spin_lock(&handle->sem);
}

static inline void i2c_drv_unlock(t_i2c* handle)
{
	spin_unlock(&handle->sem);
}


static inline void i2c_drv_wreg8(t_i2c* handle, uint8_t device, uint8_t address, uint8_t data)
{
	int ret;
	if ((ret = i2c_register_write(handle->p, device, address, 1, &data))) {
	    handle->error = ret;
		I2C_REPORT_ERROR(ret, device, address);
	}
}

static inline int i2c_drv_wreg8b(t_i2c* handle, uint8_t device, uint8_t address, uint8_t* data, size_t size)
{
    int ret;
    if ((ret = i2c_register_write(handle->p, device, address, size, data))) {
        handle->error = ret;
        I2C_REPORT_ERROR(ret, device, address);
    }
    return ret;
}

static inline uint8_t i2c_drv_rreg8(t_i2c* handle, uint8_t device, uint8_t address)
{
	uint8_t data;
	int ret;

	if ((ret = i2c_register_read(handle->p, device, address, 1, &data))) {
	    handle->error = ret;
	    I2C_REPORT_ERROR(ret, device, address);
	}

	return data;
}

static inline int i2c_drv_rreg8b(t_i2c* handle, uint8_t device, uint8_t address, uint8_t* data, size_t size)
{
    int ret;
    if ((ret = i2c_register_read(handle->p, device, address, size, data))) {
        handle->error = ret;
        I2C_REPORT_ERROR(ret, device, address);
    }
    return ret;
}

static inline uint32_t i2c_drv_get_error(t_i2c* handle)
{
    uint32_t err = handle->error;
    handle->error = SUCCESS;
    return err;
}


#endif /* I2C_DRV_H_ */
