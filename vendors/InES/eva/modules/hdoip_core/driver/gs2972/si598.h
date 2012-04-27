/*
 * si598.h
 *
 *  Created on: 17.02.2012
 *      Author: buan
 */

#ifndef SI598_H_
#define SI598_H_

#include "i2c_drv.h"

void set_si598_output_register(uint8_t hs_div, uint8_t n1, uint64_t rfreq);
void set_si598_small_frequency_change(uint8_t hs_div_i, uint8_t n1_i, uint64_t rfreq_i);
void set_si598_output_frequency (uint8_t frequency);
void si598_clock_control_init (t_i2c* i2c, void* ptr_vio);
void si598_clock_control_activate (void);
void si598_clock_control_deactivate (void);
void si598_clock_control_handler (void);


#endif /* SI598_H_ */
