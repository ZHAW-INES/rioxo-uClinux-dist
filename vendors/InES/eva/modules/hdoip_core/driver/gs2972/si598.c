/*
 * si598.c
 *
 *  Created on: 07.02.2012
 *      Author: buan
 */

#include "i2c_drv.h"
#include "si598.h"
#include "vio_reg.h"


void set_si598_output_register(t_si598 *handle)
{ 
    // calculate register values
    uint8_t tmp_1 = (((handle->hs_div << 5) & 0xE0) | ((handle->n1 >> 2) & 0x1F));
    uint8_t tmp_2 = (((handle->n1 << 6) & 0xC0) | (((uint8_t)(handle->rfreq >> 32)) & 0x3F));
    uint8_t tmp_3 = (((uint8_t)(handle->rfreq >> 24)) & 0xFF);
    uint8_t tmp_4 = (((uint8_t)(handle->rfreq >> 16)) & 0xFF);
    uint8_t tmp_5 = (((uint8_t)(handle->rfreq >> 8)) & 0xFF);
    uint8_t tmp_6 = (((uint8_t)handle->rfreq) & 0xFF);

    // freeze pll settings so they can be modified
    si598_write(handle->i2c_ptr, SI598_FREEZE, 0x10);

    // write new pll setting
    si598_write(handle->i2c_ptr, SI598_HS_N1, tmp_1);
    si598_write(handle->i2c_ptr, SI598_N1_REF_FREQ_0, tmp_2);
    si598_write(handle->i2c_ptr, SI598_REF_FREQ_1, tmp_3);
    si598_write(handle->i2c_ptr, SI598_REF_FREQ_2, tmp_4);
    si598_write(handle->i2c_ptr, SI598_REF_FREQ_3, tmp_5);
    si598_write(handle->i2c_ptr, SI598_REF_FREQ_4, tmp_6);

    // unfreeze pll -> new output frequency is available
    si598_write(handle->i2c_ptr, SI598_FREEZE, 0x00);

    // assert new frequency change bit
    si598_write(handle->i2c_ptr, SI598_NEW_FREQ, 0x40);
}

void set_si598_small_frequency_change(t_si598 *handle, uint64_t rfreq_i)
{ 
    // calculate register values
    //uint8_t tmp_1 = (((handle->hs_div << 5) & 0xE0) | ((handle->n1 >> 2) & 0x1F));
    uint8_t tmp_2 = (((handle->n1 << 6) & 0xC0) | (((uint8_t)(rfreq_i >> 32)) & 0x3F));
    uint8_t tmp_3 = (((uint8_t)(rfreq_i >> 24)) & 0xFF);
    uint8_t tmp_4 = (((uint8_t)(rfreq_i >> 16)) & 0xFF);
    uint8_t tmp_5 = (((uint8_t)(rfreq_i >> 8)) & 0xFF);
    uint8_t tmp_6 = (((uint8_t)rfreq_i) & 0xFF);

    // freeze the "M" value
    si598_write(handle->i2c_ptr, SI598_NEW_FREQ, 0x20);

    // write new pll setting
    //si598_write(handle->i2c_ptr, SI598_HS_N1, tmp_1);
    si598_write(handle->i2c_ptr, SI598_N1_REF_FREQ_0, tmp_2);
    si598_write(handle->i2c_ptr, SI598_REF_FREQ_1, tmp_3);
    si598_write(handle->i2c_ptr, SI598_REF_FREQ_2, tmp_4);
    si598_write(handle->i2c_ptr, SI598_REF_FREQ_3, tmp_5);
    si598_write(handle->i2c_ptr, SI598_REF_FREQ_4, tmp_6);

    // unfreeze the "M" value
    si598_write(handle->i2c_ptr, SI598_NEW_FREQ, 0x00);
}


void set_si598_output_frequency (t_si598 *handle, uint8_t frequency) 
{
    switch (frequency) {
        case (13):  handle->n1     = 0x5B;
                    handle->hs_div = 0x00;
                    handle->rfreq  = ((uint64_t)13500000 * ((uint64_t)handle->hs_div + (uint64_t)4) * ((uint64_t)handle->n1 + (uint64_t)1) << 28) / handle->fxtal;   // rfreq  ~ 0x0871A781FE
                    //handle->delta  = (uint64_t)SI598_CONTROL_MAX_PPM * handle->rfreq / (uint64_t)1000000;
                    break;
        case (27):  handle->n1     = 0x1B;
                    handle->hs_div = 0x03;
                    handle->rfreq  = ((uint64_t)27000000 * ((uint64_t)handle->hs_div + (uint64_t)4) * ((uint64_t)handle->n1 + (uint64_t)1) << 28) / handle->fxtal;   // rfreq  ~ 0x0871A781FE
                    //handle->delta  = (uint64_t)SI598_CONTROL_MAX_PPM * handle->rfreq / (uint64_t)1000000;
                    break;
        case (74):  handle->n1     = 0x09;
                    handle->hs_div = 0x03;
                    handle->rfreq  = ((uint64_t)74250000 * ((uint64_t)handle->hs_div + (uint64_t)4) * ((uint64_t)handle->n1 + (uint64_t)1) << 28) / handle->fxtal;   // rfreq  ~ 0x084B0FA8D1
                    //handle->delta  = (uint64_t)SI598_CONTROL_MAX_PPM * handle->rfreq / (uint64_t)1000000;
                break;
        case (148): handle->n1     = 0x03;
                    handle->hs_div = 0x05;
                    handle->rfreq  = ((uint64_t)148500000 * ((uint64_t)handle->hs_div + (uint64_t)4) * ((uint64_t)handle->n1 + (uint64_t)1) << 28) / handle->fxtal;  // rfreq  ~ 0x0887B6473D
                    //handle->delta  = (uint64_t)SI598_CONTROL_MAX_PPM * handle->rfreq / (uint64_t)1000000;
                break;
        default:    handle->n1     = 0x00;
                    handle->hs_div = 0x00;
                    handle->rfreq  = 0x0000000000;
                    //handle->delta  = 0x0000000000;
    }

    set_si598_output_register(handle);
}

void si598_clock_control_init (t_si598 *handle, t_i2c* i2c, void* ptr_vio)
{
    uint8_t t1, t2, t3, t4, t5, t6, hs_cal, n1_cal;
    uint64_t rfreq_cal;
    handle->i2c_ptr = i2c;
    handle->p_vio = ptr_vio;
    handle->active = false;

    // load factory default values before calibrate device
    si598_write(handle->i2c_ptr, SI598_NEW_FREQ, 0x01);
    while ((si598_read(handle->i2c_ptr, SI598_NEW_FREQ) & 0x01) == 0x01);

    // calibrate fXTAL
    t1 = si598_read(handle->i2c_ptr, SI598_HS_N1);
    t2 = si598_read(handle->i2c_ptr, SI598_N1_REF_FREQ_0);
    t3 = si598_read(handle->i2c_ptr, SI598_REF_FREQ_1);
    t4 = si598_read(handle->i2c_ptr, SI598_REF_FREQ_2);
    t5 = si598_read(handle->i2c_ptr, SI598_REF_FREQ_3);
    t6 = si598_read(handle->i2c_ptr, SI598_REF_FREQ_4);

    hs_cal = (t1 >> 5);
    n1_cal =  (((t1 << 2) & 0x7C) | (t2 >> 6));
    rfreq_cal = ((uint64_t)t6)    | ((uint64_t)t5<<8)    | ((uint64_t)t4<<16)    | ((uint64_t)t3<<24)    | ((((uint64_t)t2 & 0x3F)<<32));

    handle->fxtal =((((uint64_t)F_OUT_ORIG * ((uint64_t)hs_cal+(uint64_t)4) * ((uint64_t)n1_cal+(uint64_t)1)) << 28) / rfreq_cal);
}

void si598_clock_control_activate (t_si598 *handle)
{
    handle->active = true;
    handle->mean_1 = 0;
    handle->mean_2 = 0;
    handle->control_i = 0;
    handle->rfreq_control_old = handle->rfreq;
    handle->startup_counter = 0;
    handle->control_i = 0;
}

void si598_clock_control_deactivate (t_si598 *handle)
{
    handle->active = false;
}

void si598_clock_control_handler (t_si598 *handle)
{
    uint64_t rfreq_control;
    uint32_t image_offset;
    int32_t  image_offset_signed;
    int32_t  clk_offset;

    if (handle->active) {

        // get freqency fault
        image_offset = HOI_RD32((handle->p_vio), VIO_OFF_TG_CAPTURE);

        // convert 24bit signed number into 32bit signed number
        if (image_offset & 0x00800000) {
            image_offset_signed = -(0x01000000 - image_offset);
        } else {
            image_offset_signed = image_offset;
        }

        // low pass filter
        handle->mean_1 = (handle->mean_1 * 4/5) + (((int64_t)image_offset_signed<<32) * 1/5);  //32.32 fractional
        handle->mean_2 = (handle->mean_2 * 49/50) + (handle->mean_1 * 1/50);
        clk_offset = (int32_t)(handle->mean_2 >> 32);

        // integrator (delimit to max 50ppm frequency change)
        if ((handle->control_i < ((int64_t)(handle->rfreq/1000000*SI598_CONTROL_MAX_PPM)<<32)) && (clk_offset > 0)) {
            handle->control_i += ((((int64_t)clk_offset)<<32) * SI598_CONTROL_I * SI598_CONTROL_SAMPLING_RATE);
        }
        if ((handle->control_i > -((int64_t)(handle->rfreq/1000000*SI598_CONTROL_MAX_PPM)<<32)) && (clk_offset < 0)) {
            handle->control_i += ((((int64_t)clk_offset)<<32) * SI598_CONTROL_I * SI598_CONTROL_SAMPLING_RATE);
        }

        // PI controller
        rfreq_control = handle->rfreq - (SI598_CONTROL_P * clk_offset + (handle->control_i>>32));




/*
        // 2 step control (+/-5ppm clock offset)
        if ((clk_offset > 100) || (clk_offset < -100)) {
            handle->frequency_change = true;
        }

        if (clk_offset < 0) {
            handle->too_fast = false;
        } else {
            handle->too_fast = true;
        }

        rfreq_control = handle->rfreq_control_old;

        if (handle->frequency_change) {
            if (handle->too_fast) {
                if (rfreq_control < handle->rfreq - (handle->rfreq/1000000*5)) {      // max. 5 PPM
                    handle->frequency_change = false;
                } else {
                    rfreq_control = handle->rfreq_control_old - 1000;
                }
            } else {
                if (rfreq_control > handle->rfreq + (handle->rfreq/1000000*5)) {      // max. 5 PPM
                    handle->frequency_change = false;
                } else {
                    rfreq_control = handle->rfreq_control_old + 1000;
                }
            }
        }
*/

        // delimit frequency change to reduce jitter
        if (rfreq_control > (handle->rfreq_control_old + SI598_CONTROL_MAX_STEP)) {
            rfreq_control = (handle->rfreq_control_old + SI598_CONTROL_MAX_STEP);
        }
        if (rfreq_control < (handle->rfreq_control_old - SI598_CONTROL_MAX_STEP)) {
            rfreq_control = (handle->rfreq_control_old - SI598_CONTROL_MAX_STEP);
        }

        // debug messages
        //printk("\n %i |  %i  |  ", image_offset_signed, clk_offset);
        //printk("%i ", (int32_t)(handle->control_i >> 32));

        // write new frequency to SI598
        if (handle->rfreq_control_old != rfreq_control) {
            set_si598_small_frequency_change(handle, rfreq_control);
        }

        handle->rfreq_control_old = rfreq_control;
    }
}

