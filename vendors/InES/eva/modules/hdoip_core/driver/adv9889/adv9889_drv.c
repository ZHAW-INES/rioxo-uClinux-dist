/*
 * adv9889_drv.c
 *
 *  Created on: 23.08.2010
 *      Author: alda
 */
#include "adv9889_drv.h"


static inline void adv9889_lock(t_adv9889* handle) {
	i2c_drv_lock(handle->p_i2c);
}

static inline void adv9889_unlock(t_adv9889* handle) {
	i2c_drv_unlock(handle->p_i2c);
}

static inline void adv9889_write(t_adv9889* handle, uint8_t a, uint8_t x) {
	i2c_drv_wreg8(handle->p_i2c, ADV9889_ADDRESS, a, x);
}

static inline int adv9889_read(t_adv9889* handle, uint8_t a) {
	return i2c_drv_rreg8(handle->p_i2c, ADV9889_ADDRESS, a);
}


int adv9889_drv_init(t_adv9889* handle, t_i2c* p_i2c)
{
    REPORT(INFO, ">> ADV9889-Driver: Initialize HDMI-TX");

    handle->p_i2c = p_i2c;

	adv9889_lock(handle);

	// shut down...
	adv9889_write(handle, ADV9889_OFF_SYSTEM, ADV9889_SYSTEM_DOWN);

	// must be written for proper operation [AD9889B Programmer Guide 24.5.2007]
	adv9889_write(handle, 0x98, 0x07);
	adv9889_write(handle, 0x9c, 0x38);
	adv9889_write(handle, 0xbb, 0xff);

	// setup AUDIO for I2S
	adv9889_write(handle, ADV9889_OFF_AUDIO, ADV9889_AUDIO_I2S);

	adv9889_write(handle, ADV9889_OFF_I2S, ADV9889_I2S_STD |
                                      ADV9889_I2S_EN0 |
                                      ADV9889_I2S_EN1 |
                                      ADV9889_I2S_EN2 |
                                      ADV9889_I2S_EN3);
	adv9889_write(handle, ADV9889_OFF_SETTINGS, ADV9889_ACCURACY_PITCHSHIFT);

	// HDMI configuration
	adv9889_write(handle, ADV9889_OFF_MODE, ADV9889_MODE_HDMI);

	// Interface operation
	adv9889_write(handle, ADV9889_OFF_CLOCK, ADV9889_CLOCK_POS_EDGE);

	// Start operation
	adv9889_write(handle, ADV9889_OFF_SYSTEM, ADV9889_SYSTEM_UP);

	adv9889_unlock(handle);

	return SUCCESS;
}
