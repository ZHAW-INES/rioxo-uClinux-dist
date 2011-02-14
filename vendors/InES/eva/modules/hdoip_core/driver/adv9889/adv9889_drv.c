/*
 * adv9889_drv.c
 *
 *  Created on: 23.08.2010
 *      Author: alda
 */
#include "adv9889_drv.h"

#define ADV9889_INT1_ON     (ADV9889_INT1_MSEN|ADV9889_INT1_HPD)
#define ADV9889_STATUS_ON   (ADV9889_STATUS_MSEN|ADV9889_STATUS_HPD)

int adv9889_drv_powerup(t_adv9889* handle);

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

	// shut down...
	adv9889_write(handle, ADV9889_OFF_SYSTEM, ADV9889_SYSTEM_DOWN);

	// must be written for proper operation [AD9889B Programmer Guide 24.5.2007]
	adv9889_write(handle, 0x98, 0x07);
	adv9889_write(handle, 0x9c, 0x38);
	adv9889_write(handle, 0xbb, 0xff);

    // HDMI configuration
    adv9889_write(handle, ADV9889_OFF_MODE, ADV9889_MODE_HDMI);

    // Interface operation
    adv9889_write(handle, ADV9889_OFF_CLOCK, ADV9889_CLOCK_POS_EDGE);

    // Interrupt enable
    adv9889_write(handle, ADV9889_OFF_INTEN1, ADV9889_INT1_ON);

    // try power-up
    int status = adv9889_read(handle, ADV9889_OFF_STATUS);
    if ((status & ADV9889_STATUS_ON) == ADV9889_STATUS_ON) {
        REPORT(INFO, "adv9889 activated");
        adv9889_drv_powerup(handle);
        // read edid a new
        adv9889_write(handle, ADV9889_OFF_EDID_SEG, 0);
    }

	return SUCCESS;
}

int adv9889_drv_powerdown(t_adv9889* handle)
{
    // Interrupt enable
    adv9889_write(handle, ADV9889_OFF_INTEN1, ADV9889_INT1_ON);

    return SUCCESS;
}

int adv9889_drv_powerup(t_adv9889* handle)
{
    handle->edid_segment    = 0x00;
    handle->edid_first      = 1;

    // setup AUDIO for I2S
    adv9889_write(handle, ADV9889_OFF_AUDIO, ADV9889_AUDIO_I2S);

    adv9889_write(handle, ADV9889_OFF_I2S, ADV9889_I2S_STD |
                                      ADV9889_I2S_EN0 |
                                      ADV9889_I2S_EN1 |
                                      ADV9889_I2S_EN2 |
                                      ADV9889_I2S_EN3);
    adv9889_write(handle, ADV9889_OFF_SETTINGS, ADV9889_ACCURACY_PITCHSHIFT);

    // write SPD
    // ...

    // Interrupt enable
    adv9889_write(handle, ADV9889_OFF_INTEN1, ADV9889_INT1_ON | ADV9889_INT1_EDID);

    // Start operation
    adv9889_write(handle, ADV9889_OFF_SYSTEM, ADV9889_SYSTEM_UP);

    return SUCCESS;
}

int adv9889_irq_handler(t_adv9889* handle, t_queue* event_queue)
{
    int irq = adv9889_read(handle, ADV9889_OFF_IRQ1);

    if (irq & ADV9889_INT1_ON) {
        int status = adv9889_read(handle, ADV9889_OFF_STATUS);
        if ((status & ADV9889_STATUS_ON) == ADV9889_STATUS_ON) {
            REPORT(INFO, "adv9889 activated");
            adv9889_drv_powerup(handle);
        } else {
            REPORT(INFO, "adv9889 deactivated");
            adv9889_drv_powerdown(handle);
            queue_put(event_queue, E_ADV9889_CABLE_OFF);
        }
    }
    if (irq & ADV9889_INT1_EDID) {
        if (handle->edid_first) {
            handle->edid_first = 0;
            queue_put(event_queue, E_ADV9889_CABLE_ON);
        } else {
            queue_put(event_queue, E_ADV9889_NEXT_EDID);
        }
    }

    adv9889_write(handle, ADV9889_OFF_IRQ1, irq);

    return SUCCESS;
}

int adv9889_drv_get_edid(t_adv9889* handle, uint8_t* mem)
{
    // wait for edid-valid
    // ...

    i2c_drv_rreg8b(handle->p_i2c, ADV9889_ADDRESS_EDID, 0, mem, 256);

    return SUCCESS;
}

