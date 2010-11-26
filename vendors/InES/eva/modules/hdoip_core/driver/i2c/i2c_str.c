/*
 * i2c_str.c
 *
 *  Created on: 20.08.2010
 *      Author: alda
 */
#include "i2c_drv.h"

char* i2c_str_err(int err)
{
    switch(err) {
        case ERR_I2C_SUCCESS: return "success";
        case ERR_I2C_NO_DEVICE: return "no device responded";
        case ERR_I2C_DATA_NACK: return "slave did not acknowledge";
    }

    return "unknown error";
}

