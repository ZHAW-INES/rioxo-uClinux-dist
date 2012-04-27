/*
 * si598.c
 *
 *  Created on: 07.02.2012
 *      Author: buan
 */

#include "i2c_drv.h"
#include "si598.h"
#include "vio_reg.h"

// I2C address
#define SI598_ADDRESS       (0x55<<1)

// registers
#define SI598_HS_N1         (0x07)
#define SI598_N1_REF_FREQ_0 (0x08)
#define SI598_REF_FREQ_1    (0x09)
#define SI598_REF_FREQ_2    (0x0A)
#define SI598_REF_FREQ_3    (0x0B)
#define SI598_REF_FREQ_4    (0x0C)
#define SI598_NEW_FREQ      (0x87)
#define SI598_FREEZE        (0x89)

#define F_OUT_ORIG          (74250000)

// global constants
uint8_t  n1;
uint8_t  hs_div;
uint64_t rfreq;
uint64_t rfreq_control;
uint64_t rfreq_control_old;
uint64_t fxtal;
uint64_t delta;
bool active;
uint64_t mean_1;
uint64_t mean_2;
t_i2c*  i2c_ptr;
void*   p_vio;
int control_i;

// basic i/o write / read functions
static inline uint8_t si598_read(t_i2c* handle, uint8_t reg)
{
    i2c_drv_write(handle, SI598_ADDRESS, reg);

    return i2c_drv_read(handle, SI598_ADDRESS);
}

static inline void si598_write(t_i2c* handle, uint8_t reg, uint8_t data)
{
    i2c_drv_wreg8(handle, SI598_ADDRESS, reg, data);
}

//////////////////////////////////////////////////////////////////////////////////////////

void set_si598_output_register(uint8_t hs_div_i, uint8_t n1_i, uint64_t rfreq_i)
{ 
    // calculate register values
    uint8_t tmp_1 = (((hs_div_i << 5) & 0xE0) | ((n1_i >> 2) & 0x1F));
    uint8_t tmp_2 = (((n1_i << 6) & 0xC0) | (((uint8_t)(rfreq_i >> 32)) & 0x3F));
    uint8_t tmp_3 = (((uint8_t)(rfreq_i >> 24)) & 0xFF);
    uint8_t tmp_4 = (((uint8_t)(rfreq_i >> 16)) & 0xFF);
    uint8_t tmp_5 = (((uint8_t)(rfreq_i >> 8)) & 0xFF);
    uint8_t tmp_6 = (((uint8_t)rfreq_i) & 0xFF);

    // freeze pll settings so they can be modified
    si598_write(i2c_ptr, SI598_FREEZE, 0x10);

    // write new pll setting
    si598_write(i2c_ptr, SI598_HS_N1, tmp_1);
    si598_write(i2c_ptr, SI598_N1_REF_FREQ_0, tmp_2);
    si598_write(i2c_ptr, SI598_REF_FREQ_1, tmp_3);
    si598_write(i2c_ptr, SI598_REF_FREQ_2, tmp_4);
    si598_write(i2c_ptr, SI598_REF_FREQ_3, tmp_5);
    si598_write(i2c_ptr, SI598_REF_FREQ_4, tmp_6);

    // unfreeze pll -> new output frequency is available
    si598_write(i2c_ptr, SI598_FREEZE, 0x00);

    // assert new frequency change bit
    si598_write(i2c_ptr, SI598_NEW_FREQ, 0x40);
}

void set_si598_small_frequency_change(uint8_t hs_div_i, uint8_t n1_i, uint64_t rfreq_i)
{ 
    // calculate register values
    uint8_t tmp_1 = (((hs_div_i << 5) & 0xE0) | ((n1_i >> 2) & 0x1F));
    uint8_t tmp_2 = (((n1_i << 6) & 0xC0) | (((uint8_t)(rfreq_i >> 32)) & 0x3F));
    uint8_t tmp_3 = (((uint8_t)(rfreq_i >> 24)) & 0xFF);
    uint8_t tmp_4 = (((uint8_t)(rfreq_i >> 16)) & 0xFF);
    uint8_t tmp_5 = (((uint8_t)(rfreq_i >> 8)) & 0xFF);
    uint8_t tmp_6 = (((uint8_t)rfreq_i) & 0xFF);

    // freeze the "M" value
    si598_write(i2c_ptr, SI598_NEW_FREQ, 0x20);

    // write new pll setting
    si598_write(i2c_ptr, SI598_HS_N1, tmp_1);
    si598_write(i2c_ptr, SI598_N1_REF_FREQ_0, tmp_2);
    si598_write(i2c_ptr, SI598_REF_FREQ_1, tmp_3);
    si598_write(i2c_ptr, SI598_REF_FREQ_2, tmp_4);
    si598_write(i2c_ptr, SI598_REF_FREQ_3, tmp_5);
    si598_write(i2c_ptr, SI598_REF_FREQ_4, tmp_6);

    // unfreeze the "M" value
    si598_write(i2c_ptr, SI598_NEW_FREQ, 0x00);
}


void set_si598_output_frequency (uint8_t frequency) 
{
    switch (frequency) {
        case (13):  n1     = 0x5B;
                    hs_div = 0x00;
                    rfreq = ((uint64_t)13500000 * ((uint64_t)hs_div + (uint64_t)4) * ((uint64_t)n1 + (uint64_t)1) << 28) / fxtal;   // rfreq  ~ 0x0871A781FE
                    delta  = 0x0;
                    break;
        case (27):  n1     = 0x1B;
                    hs_div = 0x03;
                    rfreq = ((uint64_t)27000000 * ((uint64_t)hs_div + (uint64_t)4) * ((uint64_t)n1 + (uint64_t)1) << 28) / fxtal;   // rfreq  ~ 0x0871A781FE
                    delta  = 0x000790D79F;
                    break;
        case (74):  n1     = 0x09;
                    hs_div = 0x03;
                    rfreq = ((uint64_t)74250000 * ((uint64_t)hs_div + (uint64_t)4) * ((uint64_t)n1 + (uint64_t)1) << 28) / fxtal;   // rfreq  ~ 0x084B0FA8D1
                    delta  = 0x00076E417D;
                break;
        case (148): n1     = 0x03;
                    hs_div = 0x05;
                    rfreq = ((uint64_t)148500000 * ((uint64_t)hs_div + (uint64_t)4) * ((uint64_t)n1 + (uint64_t)1) << 28) / fxtal;  // rfreq  ~ 0x0887B6473D
                    delta  = 0x0007A49B21;
                break;
        default:    n1     = 0x00;
                    hs_div = 0x00;
                    rfreq  = 0x0000000000;
                    delta  = 0x0000000000;
    }

    set_si598_output_register(hs_div, n1, rfreq);
}

void si598_clock_control_init (t_i2c* i2c, void* ptr_vio)
{
    uint8_t t1, t2, t3, t4, t5, t6, hs_cal, n1_cal;
    uint64_t rfreq_cal;

    i2c_ptr = i2c;
    p_vio = ptr_vio;
    active = false;

    // load factory default values before calibrate device
    si598_write(i2c_ptr, SI598_NEW_FREQ, 0x01);
    while ((si598_read(i2c_ptr, SI598_NEW_FREQ) & 0x01) == 0x01);

    // calibrate fXTAL
    t1 = si598_read(i2c_ptr, SI598_HS_N1);
    t2 = si598_read(i2c_ptr, SI598_N1_REF_FREQ_0);
    t3 = si598_read(i2c_ptr, SI598_REF_FREQ_1);
    t4 = si598_read(i2c_ptr, SI598_REF_FREQ_2);
    t5 = si598_read(i2c_ptr, SI598_REF_FREQ_3);
    t6 = si598_read(i2c_ptr, SI598_REF_FREQ_4);

    hs_cal = (t1 >> 5);
    n1_cal =  (((t1 << 2) & 0x7C) | (t2 >> 6));
    rfreq_cal = ((uint64_t)t6)    | ((uint64_t)t5<<8)    | ((uint64_t)t4<<16)    | ((uint64_t)t3<<24)    | ((((uint64_t)t2 & 0x3F)<<32));

    fxtal =((((uint64_t)F_OUT_ORIG * ((uint64_t)hs_cal+(uint64_t)4) * ((uint64_t)n1_cal+(uint64_t)1)) << 28) / rfreq_cal);
}

void si598_clock_control_activate (void)
{
    active = true;
    mean_1 = 0;
    mean_2 = 0;
    control_i = 0;
    rfreq_control_old = rfreq;
}

void si598_clock_control_deactivate (void)
{
    active = false;
}


#define SI598_CONTROL_P         (20)
#define SI598_CONTROL_I         (50)
#define SI598_CONTROL_MAX_STEP  (100)

void si598_clock_control_handler (void)
{
    uint32_t image_offset;
    uint32_t clk_offset;
    bool direction;

    if (active) {

        image_offset = HOI_RD32((p_vio), VIO_OFF_TG_CAPTURE);
        // convert 24bit signed number into sign flag and value
        if (image_offset & 0x00800000) {
            direction = true;
            image_offset = 0x01000000 - image_offset;
        } else {
            direction = false;
        }

        // low pass filter
        mean_1 = (mean_1 * 4/5) + (((uint64_t)image_offset<<32) * 1/5);  //32.32 fractional
        mean_2 = (mean_2 * 49/50) + (mean_1 * 1/50);

        clk_offset = (uint32_t)(mean_2 >> 32);


        if (direction) {
            rfreq_control = rfreq + ((uint64_t)clk_offset * SI598_CONTROL_P) + control_i;     // too slow
            if (control_i < ((int)clk_offset * SI598_CONTROL_P)) {
                control_i += (int)clk_offset / SI598_CONTROL_I;
            }
        } else {
            rfreq_control = rfreq - ((uint64_t)clk_offset * SI598_CONTROL_P) + control_i;     // too fast
            if (control_i > -((int)clk_offset * SI598_CONTROL_P)) {
                control_i -= (int)clk_offset / SI598_CONTROL_I;
            }
        }
       
        //TODO: limit max step size
/*
        if (rfreq_control > rfreq_control_old + SI598_CONTROL_MAX_STEP) {
            rfreq_control = rfreq_control_old + SI598_CONTROL_MAX_STEP;
        }
        if (rfreq_control < rfreq_control_old - SI598_CONTROL_MAX_STEP) {
            rfreq_control = rfreq_control_old - SI598_CONTROL_MAX_STEP;
        }
*/

/*
            // Debug messages
            printk("\n d: %i o: %i  m: %i  c: %02x", direction, image_offset, clk_offset, (uint32_t)(rfreq_control>>32));
            printk("%08x", (uint32_t)rfreq_control);
            printk(" a: %i", (clk_offset * SI598_CONTROL_P));
            printk(" i: %i", control_i);
*/
/*
        if (direction) {
            printk("\ntoo slow - i: %i | p: %i | s: %i | o: %i", control_i, (clk_offset * SI598_CONTROL_P), control_i + (clk_offset * SI598_CONTROL_P), clk_offset);
        } else {
            printk("\ntoo fast - i: %i | p: %i | s: %i | o: %i", control_i, (clk_offset * SI598_CONTROL_P), control_i - (clk_offset * SI598_CONTROL_P), clk_offset);
        }
*/

        if (rfreq_control_old != rfreq_control) {
            // TODO: rfreq_control must be in range rfreq +/- delta
            set_si598_small_frequency_change(hs_div, n1, rfreq_control);
//            printk(" w");
        }
        rfreq_control_old = rfreq_control;
    }
}

