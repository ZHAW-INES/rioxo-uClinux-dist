/*
 * i2c_hal.c
 *
 *  Created on: 20.08.2010
 *      Author: alda
 */
#include "i2c_hal.h"

#define i2c_cr(p, x)			HOI_WR32((p), I2C_OFF_CR, (x))
#define i2c_sr(p, x)			(HOI_RD32((p), I2C_OFF_SR) & (x))
#define i2c_wdata(p, x) 		HOI_WR32((p), I2C_OFF_DATA, (x))
#define i2c_rdata(p, x) 		(x) = HOI_RD32((p), I2C_OFF_DATA)


/** execute a i2c command
 *
 * this code locks until the command is executed
 *
 * @param p base address of i2c master
 * @param x the command
 */
static inline void i2c_cmd(void* p, int8_t x)
{
	i2c_cr(p, x);
	while(i2c_sr(p, I2C_SR_TIP));
}


/** execute a i2c write command
 *
 * is it is the last data, add a stop command
 * this code locks until the command is executed
 *
 * @param p base address of i2c master
 * @param last true when no more data follows
 */
static inline void i2c_wr(void* p, bool last)
{
    if (!last) {
        i2c_cmd(p, I2C_CR_WR);
    } else {
    	i2c_cmd(p, I2C_CR_WR | I2C_CR_STO);
    }
}


/** execute a i2c read command
 *
 * is it is the last data, add a stop/nack command
 * this code locks until the command is executed
 *
 * @param p base address of i2c master
 * @param last true when no more data follows
 */
static inline void i2c_rd(void* p, bool last)
{
	if (!last) {
		i2c_cmd(p, I2C_CR_RD);
	} else {
		i2c_cmd(p, I2C_CR_RD | I2C_CR_NACK | I2C_CR_STO);
	}
}

/** Initialize i2c master
 *
 * @param p base address of i2c master
 * @param prescale prescale value for clock generation (f = fsys / prescale / 5)
 */
int i2c_init(void* p, uint16_t prescale)
{
	HOI_WR32(p, I2C_OFF_CTR, 0);
	i2c_set_prescale(p, prescale);
	HOI_WR32(p, I2C_OFF_CTR, I2C_CTR_EN);

	return ERR_I2C_SUCCESS;
}

/** write a sequence of bytes to a i2c slave
 *
 * @param p base address of i2c master
 * @param address device address
 * @param size number of bytes to be transfered
 * @param data array pointer
 * @return error code
 */
int i2c_write(void* p, uint8_t address, size_t size, uint8_t* data)
{
    // Address
    i2c_wdata(p, address & 0xfe);
    i2c_cmd  (p, I2C_CR_STA | I2C_CR_WR);

    // no device?
    if (i2c_sr(p, I2C_SR_NACK)) {
    	i2c_cmd(p, I2C_CR_STO);
        return ERR_I2C_NO_DEVICE;
    }

    // transfer data
    for (;size;size--,data++) {

    	i2c_wdata(p, *data);
    	i2c_wr	 (p, (size == 1));

        // no ack
        if (i2c_sr(p, I2C_SR_NACK)) {
        	i2c_cmd(p, I2C_CR_STO);
            return ERR_I2C_DATA_NACK;
        }

    }

    return ERR_I2C_SUCCESS;
}

/** read a sequence of bytes from a i2c slave
 *
 * @param p base address of i2c master
 * @param address device address
 * @param size number of bytes to be transfered
 * @param data array pointer
 * @return error code
 */
int i2c_read(void* p, uint8_t address, size_t size, uint8_t* data)
{
    // Address
    i2c_wdata(p, (address & 0xfe) | 0x01);
    i2c_cmd  (p, I2C_CR_STA | I2C_CR_WR);

    // no device?
    if (i2c_sr(p, I2C_SR_NACK)) {
    	i2c_cmd(p, I2C_CR_STO);
        return ERR_I2C_NO_DEVICE;
    }

    // transfer data
    for (;size;size--,data++) {

        i2c_rd   (p, (size == 1));
        i2c_rdata(p, *data);

    }

    return ERR_I2C_SUCCESS;
}

/** write a sequence of bytes to a i2c slave register
 *
 * @param p base address of i2c master
 * @param device device address
 * @param address register address
 * @param size number of bytes to be transfered
 * @param data array pointer
 * @return error code
 */
int i2c_register_write(void* p, uint8_t device, uint8_t address, size_t size, uint8_t* data)
{
    // Device Address
    i2c_wdata(p, device & 0xfe);
    i2c_cmd  (p, I2C_CR_STA | I2C_CR_WR);

    // no device?
    if (i2c_sr(p, I2C_SR_NACK)) {
    	i2c_cmd(p, I2C_CR_STO);
        return ERR_I2C_NO_DEVICE;
    }

    // Register Address
    i2c_wdata(p, address);
    i2c_cmd  (p, I2C_CR_WR);

    // no address ack?
    if (i2c_sr(p, I2C_SR_NACK)) {
    	i2c_cmd(p, I2C_CR_STO);
        return ERR_I2C_DATA_NACK;
    }

    // transfer data
    for (;size;size--,data++) {

    	i2c_wdata(p, *data);
    	i2c_wr	 (p, (size == 1));

        // no ack
        if (i2c_sr(p, I2C_SR_NACK)) {
        	i2c_cmd(p, I2C_CR_STO);
            return ERR_I2C_DATA_NACK;
        }

    }

    return ERR_I2C_SUCCESS;
}

/** read a sequence of bytes from a i2c slave register
 *
 * @param p base address of i2c master
 * @param device device address
 * @param address register address
 * @param size number of bytes to be transfered
 * @param data array pointer
 * @return error code
 */
int i2c_register_read(void* p, uint8_t device, uint8_t address, size_t size, uint8_t* data)
{
    // Device Address
    i2c_wdata(p, device & 0xfe);
    i2c_cmd  (p, I2C_CR_STA | I2C_CR_WR);

    // no device?
    if (i2c_sr(p, I2C_SR_NACK)) {
    	i2c_cmd(p, I2C_CR_STO);
        return ERR_I2C_NO_DEVICE;
    }

    // Register Address
    i2c_wdata(p, address);
    i2c_cmd  (p, I2C_CR_WR);

    // no address ack?
    if (i2c_sr(p, I2C_SR_NACK)) {
    	i2c_cmd(p, I2C_CR_STO);
        return ERR_I2C_DATA_NACK;
    }

    // Device Address / Repeated start
    i2c_wdata(p, (device & 0xfe) | 0x01);
    i2c_cmd  (p, I2C_CR_STA | I2C_CR_WR);

    // no device?
    if (i2c_sr(p, I2C_SR_NACK)) {
    	i2c_cmd(p, I2C_CR_STO);
        return ERR_I2C_NO_DEVICE;
    }

    // transfer data
    for (;size;size--,data++) {

        i2c_rd   (p, (size == 1));
        i2c_rdata(p, *data);

    }

    return ERR_I2C_SUCCESS;
}
