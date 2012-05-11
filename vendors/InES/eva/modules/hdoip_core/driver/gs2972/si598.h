/*
 * si598.h
 *
 *  Created on: 17.02.2012
 *      Author: buan
 */

#ifndef SI598_H_
#define SI598_H_

#include "i2c_drv.h"

/* I2C address                                   */
#define SI598_ADDRESS                       (0x55<<1)

/* registers                                     */
#define SI598_HS_N1                         (0x07)
#define SI598_N1_REF_FREQ_0                 (0x08)
#define SI598_REF_FREQ_1                    (0x09)
#define SI598_REF_FREQ_2                    (0x0A)
#define SI598_REF_FREQ_3                    (0x0B)
#define SI598_REF_FREQ_4                    (0x0C)
#define SI598_NEW_FREQ                      (0x87)
#define SI598_FREEZE                        (0x89)

/* native frequency of SI598                     */
#define F_OUT_ORIG                          74250000
/* P value of PI controller                      */
#define SI598_CONTROL_P                     100
/* integral value of PI controller               */
#define SI598_CONTROL_I                     2
/* max step size of RFREQ (bigger = more jitter) */
#define SI598_CONTROL_MAX_STEP              1000
/* samples per 1 second                          */
#define SI598_CONTROL_SAMPLES_PER_SECOND    36
/* sampling rate of clock control                */
#define SI598_CONTROL_SAMPLING_RATE         1/SI598_CONTROL_SAMPLES_PER_SECOND
/* clock accuracy                                */
#define SI598_CONTROL_MAX_PPM               50

#define SI598_MODE_SD                       0
#define SI598_MODE_HD                       1
#define SI598_MODE_3G                       2

typedef struct {
    uint8_t  n1;
    uint8_t  hs_div;
    uint64_t rfreq;
    uint64_t rfreq_control_old;
    uint64_t fxtal;
    bool     active;
    int64_t  mean_1;
    int64_t  mean_2;
    t_i2c*   i2c_ptr;
    void*    p_vio;
    int64_t  control_i;
    int      startup_counter;
    bool     frequency_change;
    int      mode;
    int      speed_result;
    int      speed;
    int      speed_tmp;
    int      speed_count;
} t_si598;


static inline uint8_t si598_read (t_i2c* handle, uint8_t reg)
{
    i2c_drv_write(handle, SI598_ADDRESS, reg);

    return i2c_drv_read(handle, SI598_ADDRESS);
}

static inline void si598_write (t_i2c* handle, uint8_t reg, uint8_t data)
{
    i2c_drv_wreg8(handle, SI598_ADDRESS, reg, data);
}


void set_si598_output_register (t_si598 *handle);
void set_si598_small_frequency_change (t_si598 *handle, uint64_t rfreq_i);
void set_si598_output_frequency (t_si598 *handle, uint32_t frequency);
void si598_clock_control_init (t_si598 *handle, t_i2c* i2c, void* ptr_vio);
void si598_clock_control_activate (t_si598 *handle);
void si598_clock_control_deactivate (t_si598 *handle);
void si598_clock_control_handler (t_si598 *handle);


#endif /* SI598_H_ */
