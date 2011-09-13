/*
 * io_expander_rx.h
 *
 *  Created on: 09.09.2011
 *      Author: buan
 */

#ifndef IO_EXPANDER_RX_H_
#define IO_EXPANDER_RX_H_

#include "i2c_drv.h"

//I2C address
#define IO_RX_ADDRESS       (0x46)

//command bytes
#define IO_RX_IN0           (0x00)
#define IO_RX_IN1           (0x01)
#define IO_RX_INVRT0        (0x02)
#define IO_RX_INVRT1        (0x03)
#define IO_RX_BKEN0         (0x04)
#define IO_RX_BKEN1         (0x05)
#define IO_RX_PUPD0         (0x06)
#define IO_RX_PUPD1         (0x07)
#define IO_RX_CFG0          (0x08)
#define IO_RX_CFG1          (0x09)
#define IO_RX_OUT0          (0x0A)
#define IO_RX_OUT1          (0x0B)
#define IO_RX_MSK0          (0x0C)
#define IO_RX_MSK1          (0x0D)
#define IO_RX_INTS0         (0x0E)
#define IO_RX_INTS1         (0x0F)

// i/o pins port 0
#define RX_LED_1            (0x001) // out
#define RX_LED_2            (0x002) // out
#define RX_LED_3            (0x004) // out
#define RX_LED_4            (0x008) // out
#define RX_IOPROC_EN        (0x020) // out
#define RX_LOOP_SD          (0x040) // out
#define RX_AUDIO_EN         (0x080) // out


// i/o pins port 1
#define RX_STANDBY          (0x101)  // out
#define RX_RC_BYP           (0x102)  // out
#define RX_SDO_EN           (0x104)  // out
#define RX_STAT_3           (0x108)  // in
#define RX_STAT_4           (0x110)  // in
#define RX_STAT_5           (0x120)  // in
#define RX_SMPTE_BYPASS     (0x140)  // in
#define RX_DVB_ASI          (0x180)  // in


// basic i/o write / read functions
static inline uint8_t io_exp_read_rx(t_i2c* handle, uint8_t command)
{
    i2c_drv_write(handle, IO_RX_ADDRESS, command);

    return i2c_drv_read(handle, IO_RX_ADDRESS);
}

static inline void io_exp_write_rx(t_i2c* handle, uint8_t command, uint8_t data)
{
    i2c_drv_wreg8(handle, IO_RX_ADDRESS, command, data);
}


//-------------------------------------
static inline void init_io_exp_rx(t_i2c* handle)
{
    // set polarity of i/o-pins
    io_exp_write_rx(handle, IO_RX_CFG0, 0x00);
    io_exp_write_rx(handle, IO_RX_CFG1, ((0x00 | RX_STAT_3| RX_STAT_4 | RX_STAT_5 | RX_SMPTE_BYPASS | RX_DVB_ASI) & 0xFF));
}

static inline void set_io_exp_rx_pin(t_i2c* handle, uint16_t set_mask)
{
    uint8_t tmp;
    
    if (set_mask & 0xF00) {
        tmp = io_exp_read_rx(handle, IO_RX_OUT1);
        io_exp_write_rx(handle, IO_RX_OUT1, (tmp | (set_mask & 0xFF)));
    } else {
        tmp = io_exp_read_rx(handle, IO_RX_OUT0);
        io_exp_write_rx(handle, IO_RX_OUT0, (tmp | (set_mask & 0xFF)));
    }
}

static inline void clr_io_exp_rx_pin(t_i2c* handle, uint16_t clr_mask)
{
    uint8_t tmp;
    
    if (clr_mask & 0xF00) {
        tmp = io_exp_read_rx(handle, IO_RX_OUT1);
        io_exp_write_rx(handle, IO_RX_OUT1, (tmp & ~(clr_mask & 0xFF)));
    } else {
        tmp = io_exp_read_rx(handle, IO_RX_OUT0);
        io_exp_write_rx(handle, IO_RX_OUT0, (tmp & ~(clr_mask & 0xFF)));
    }
}

static inline uint8_t read_io_exp_rx_pin(t_i2c* handle, uint16_t read_mask)
{
    uint8_t tmp;

    if (read_mask & 0xF00) {
        tmp = io_exp_read_rx(handle, IO_RX_IN1);
    } else {
        tmp = io_exp_read_rx(handle, IO_RX_IN0);
    }

    return (tmp & (read_mask & 0xFF));
}


#endif /* IO_EXPANDER_RX_H_ */
