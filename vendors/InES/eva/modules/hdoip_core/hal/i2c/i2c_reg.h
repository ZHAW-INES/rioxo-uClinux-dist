/*
 * i2c_reg.h
 *
 *  Created on: 20.08.2010
 *      Author: alda
 */

#ifndef I2C_REG_H_
#define I2C_REG_H_

#define I2C_OFF_PRESCALE_LSB	0
#define I2C_OFF_PRESCALE_MSB  	4
#define I2C_OFF_CTR           	8
#define I2C_OFF_DATA          	12
#define I2C_OFF_CR            	16
#define I2C_OFF_SR            	16

#define I2C_CTR_EN        		0x80
#define I2C_CTR_IEN       		0x40

#define I2C_CR_STA       		0x80
#define I2C_CR_STO       		0x40
#define I2C_CR_RD        		0x20
#define I2C_CR_WR        		0x10
#define I2C_CR_NACK      		0x08
#define I2C_CR_IACK      		0x01

#define I2C_SR_NACK     		0x80
#define I2C_SR_BUSY       		0x40
#define I2C_SR_AL         		0x20
#define I2C_SR_TIP        		0x02
#define I2C_SR_IF         		0x01


#endif /* I2C_REG_H_ */
