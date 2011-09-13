/*
 * io_expander_tx.h
 *
 *  Created on: 09.09.2011
 *      Author: buan
 */

#ifndef IO_EXPANDER_TX_H_
#define IO_EXPANDER_TX_H_

#include "i2c_drv.h"

//I2C address
#define IO_TX_ADDRESS       (0x48)

//command bytes
#define IO_TX_IN0           (0x00)
#define IO_TX_IN1           (0x01)
#define IO_TX_INVRT0        (0x02)
#define IO_TX_INVRT1        (0x03)
#define IO_TX_BKEN0         (0x04)
#define IO_TX_BKEN1         (0x05)
#define IO_TX_PUPD0         (0x06)
#define IO_TX_PUPD1         (0x07)
#define IO_TX_CFG0          (0x08)
#define IO_TX_CFG1          (0x09)
#define IO_TX_OUT0          (0x0A)
#define IO_TX_OUT1          (0x0B)
#define IO_TX_MSK0          (0x0C)
#define IO_TX_MSK1          (0x0D)
#define IO_TX_INTS0         (0x0E)
#define IO_TX_INTS1         (0x0F)

// i/o pins port 0
#define TX_AUDIO_1_EN       (0x020)  // out
#define TX_AUDIO_2_EN       (0x040)  // out
#define TX_SMPTE_BYPASS     (0x080)  // out

// i/o pins port 1
#define TX_STANDBY          (0x101)  // out
#define TX_SDO_EN           (0x102)  // out
#define TX_ANC_BLANK        (0x104)  // out
#define TX_LOCKED           (0x108)  // in
#define TX_IOPROC_EN        (0x110)  // out
#define TX_RATE_SEL_0       (0x120)  // out
#define TX_RATE_SEL_1       (0x140)  // out
#define TX_DVB_ASI          (0x180)  // out


// basic i/o write / read functions
static inline uint8_t io_exp_read_tx(t_i2c* handle, uint8_t command)
{
    i2c_drv_write(handle, IO_TX_ADDRESS, command);

    return i2c_drv_read(handle, IO_TX_ADDRESS);
}

static inline void io_exp_write_tx(t_i2c* handle, uint8_t command, uint8_t data)
{
    i2c_drv_wreg8(handle, IO_TX_ADDRESS, command, data);
}


//-------------------------------------
static inline void init_io_exp_tx(t_i2c* handle)
{
    // set polarity of i/o-pins
    io_exp_write_tx(handle, IO_TX_CFG0, 0x00);
    io_exp_write_tx(handle, IO_TX_CFG1, ((0x00 | TX_LOCKED) & 0xFF));
}

static inline void set_io_exp_tx_pin(t_i2c* handle, uint16_t set_mask)
{
    uint8_t tmp;
    
    if (set_mask & 0xF00) {
        tmp = io_exp_read_tx(handle, IO_TX_OUT1);
        io_exp_write_tx(handle, IO_TX_OUT1, (tmp | (set_mask & 0xFF)));
    } else {
        tmp = io_exp_read_tx(handle, IO_TX_OUT0);
        io_exp_write_tx(handle, IO_TX_OUT0, (tmp | (set_mask & 0xFF)));
    }
}

static inline void clr_io_exp_tx_pin(t_i2c* handle, uint16_t clr_mask)
{
    uint8_t tmp;
    
    if (clr_mask & 0xF00) {
        tmp = io_exp_read_tx(handle, IO_TX_OUT1);
        io_exp_write_tx(handle, IO_TX_OUT1, (tmp & ~(clr_mask & 0xFF)));
    } else {
        tmp = io_exp_read_tx(handle, IO_TX_OUT0);
        io_exp_write_tx(handle, IO_TX_OUT0, (tmp & ~(clr_mask & 0xFF)));
    }
}

static inline uint8_t read_io_exp_tx_pin(t_i2c* handle, uint16_t read_mask)
{
    uint8_t tmp;

    if (read_mask & 0xF00) {
        tmp = io_exp_read_tx(handle, IO_TX_IN1);
    } else {
        tmp = io_exp_read_tx(handle, IO_TX_IN0);
    }

    return (tmp & (read_mask & 0xFF));
}


#endif /* IO_EXPANDER_TX_H_ */
