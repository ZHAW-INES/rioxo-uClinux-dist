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


void set_si598_output_frequency (t_si598 *handle, uint32_t frequency, int advcnt) 
{
    switch (frequency) {
        case (13500000):    if (advcnt == 1) {
                                handle->n1     = 0x1B;  // 27MHz (Clock is divided in half in gateware)
                                handle->hs_div = 0x03;
                                handle->rfreq  = ((uint64_t)27000000 * ((uint64_t)handle->hs_div + (uint64_t)4) * ((uint64_t)handle->n1 + (uint64_t)1) << 28) / handle->fxtal;
                                handle->mode   = SI598_MODE_SD;
                            } else {
                                handle->n1     = 0x37;  // 13.5MHz
                                handle->hs_div = 0x03;
                                handle->rfreq  = ((uint64_t)13500000 * ((uint64_t)handle->hs_div + (uint64_t)4) * ((uint64_t)handle->n1 + (uint64_t)1) << 28) / handle->fxtal;
                                handle->mode   = SI598_MODE_SD;
                            }
                            break;
        case (74250000):    handle->n1     = 0x09;  // 74.25MHz
                            handle->hs_div = 0x03;
                            handle->rfreq  = ((uint64_t)74250000 * ((uint64_t)handle->hs_div + (uint64_t)4) * ((uint64_t)handle->n1 + (uint64_t)1) << 28) / handle->fxtal;
                            handle->mode   = SI598_MODE_HD;
                            break;
        case (74175824):    handle->n1     = 0x09;  // 74.25MHz / 1.001
                            handle->hs_div = 0x03;
                            handle->rfreq  = ((uint64_t)74175824 * ((uint64_t)handle->hs_div + (uint64_t)4) * ((uint64_t)handle->n1 + (uint64_t)1) << 28) / handle->fxtal;
                            handle->mode   = SI598_MODE_HD;
                            break;
        case (148500000):   handle->n1     = 0x03;  // 148.5MHz
                            handle->hs_div = 0x05;
                            handle->rfreq  = ((uint64_t)148500000 * ((uint64_t)handle->hs_div + (uint64_t)4) * ((uint64_t)handle->n1 + (uint64_t)1) << 28) / handle->fxtal;
                            handle->mode   = SI598_MODE_3G;
                            break;
        case (148351648):   handle->n1     = 0x03;  // 148.5MHz / 1.001
                            handle->hs_div = 0x05;
                            handle->rfreq  = ((uint64_t)148351648 * ((uint64_t)handle->hs_div + (uint64_t)4) * ((uint64_t)handle->n1 + (uint64_t)1) << 28) / handle->fxtal;
                            handle->mode   = SI598_MODE_3G;
                            break;
        default:            handle->n1     = 0x00;
                            handle->hs_div = 0x00;
                            handle->rfreq  = 0x0000000000;
                            handle->mode   = 0;
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
    int      tmp;

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

        // use 2 different clock controllers: one for SD and one for HD/3G (because of jitter reduction)
        if (handle->mode == SI598_MODE_SD) {

            // 2 step control (+/-50ppm clock offset)
            if ((clk_offset > 100) || (clk_offset < -100)) {
                handle->frequency_change = true;
            }

            // measure image offset change in 1 sec
            if (handle->speed_count > SI598_CONTROL_SAMPLES_PER_SECOND) {
                handle->speed_count = 0;
                tmp = handle->speed;
                handle->speed = handle->speed_tmp / SI598_CONTROL_SAMPLES_PER_SECOND;
                handle->speed_result = (handle->speed - tmp);
                handle->speed_tmp = 0;
            } else {
                handle->speed_count ++;
                handle->speed_tmp += image_offset_signed;; //clk_offset; //image_offset_signed;
            }

            rfreq_control = handle->rfreq_control_old;

            if (handle->frequency_change) {
                if (clk_offset > 0) {
                    if (rfreq_control < handle->rfreq - (handle->rfreq/1000000*SI598_CONTROL_MAX_PPM)) {      // max. 50 PPM
                        handle->frequency_change = false;
                    } else {
                        if (!((handle->speed_result < -20) && (rfreq_control < handle->rfreq))) {
                            rfreq_control = handle->rfreq_control_old - SI598_CONTROL_MAX_STEP;
                        }
                    }
                } else {
                    if (rfreq_control > handle->rfreq + (handle->rfreq/1000000*SI598_CONTROL_MAX_PPM)) {      // max. 50 PPM
                        handle->frequency_change = false;
                    } else {
                        if (!((handle->speed_result > 20) && (rfreq_control > handle->rfreq))) {
                            rfreq_control = handle->rfreq_control_old + SI598_CONTROL_MAX_STEP;
                        }
                    }
                }
            }

        } else {

            // integrator (delimit to max 50ppm frequency change)
            if ((handle->control_i < ((int64_t)(handle->rfreq/1000000*SI598_CONTROL_MAX_PPM)<<32)) && (clk_offset > 0)) {
                handle->control_i += ((((int64_t)clk_offset)<<32) * SI598_CONTROL_I * SI598_CONTROL_SAMPLING_RATE);
            }
            if ((handle->control_i > -((int64_t)(handle->rfreq/1000000*SI598_CONTROL_MAX_PPM)<<32)) && (clk_offset < 0)) {
                handle->control_i += ((((int64_t)clk_offset)<<32) * SI598_CONTROL_I * SI598_CONTROL_SAMPLING_RATE);
            }

            // PI controller
            rfreq_control = handle->rfreq - ((SI598_CONTROL_P * clk_offset) + (handle->control_i>>32));

            // delimit frequency change to reduce jitter
            if (rfreq_control > (handle->rfreq_control_old + SI598_CONTROL_MAX_STEP)) {
                rfreq_control = (handle->rfreq_control_old + SI598_CONTROL_MAX_STEP);
            }
            if (rfreq_control < (handle->rfreq_control_old - SI598_CONTROL_MAX_STEP)) {
                rfreq_control = (handle->rfreq_control_old - SI598_CONTROL_MAX_STEP);
            }
        }

        // debug messages
        //printk("\n si598: %i  ", image_offset_signed);
        //printk("%i |", (int32_t)(handle->control_i >> 32));
        //printk(" %02x", (rfreq_control >> 32));
        //printk("%08x  ", rfreq_control);

        // write new frequency to SI598
        if (handle->rfreq_control_old != rfreq_control) {
            set_si598_small_frequency_change(handle, rfreq_control);
        }

        handle->rfreq_control_old = rfreq_control;
    }
}

