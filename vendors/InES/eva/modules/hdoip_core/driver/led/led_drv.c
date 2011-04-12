
#include "led_drv.h"

#define LED_I2C_VID_ADDR        (0x22<<1)

/* Toggles LEDs 
 *
 * @param handle    pointer to handle of LED driver
 * @param mask      toggle mask 
 * @return error code
 */
int led_drv_tgl(t_led* handle, uint32_t mask)
{
    handle->led_set = ((handle->led_set & mask) ^ mask) | (handle->led_set & ~mask);

    if(mask & LED_VID_RNG) {
        i2c_drv_write(handle->p_vid_i2c, LED_I2C_VID_ADDR, ~(handle->led_set & LED_VID_RNG));
    }

    return SUCCESS;
}

/* Clear LEDs
 *
 * @param handle    pointer to handle of LED driver
 * @param mask      clear mask
 * @return error code
 */
int led_drv_clr(t_led* handle, uint32_t mask)
{
    uint32_t update = (handle->led_set & mask);
    handle->led_set &= ~mask;

    /* Clear Videoboard LEDs */
    if(update & LED_VID_RNG) {
        i2c_drv_write(handle->p_vid_i2c, LED_I2C_VID_ADDR, ~(handle->led_set & LED_VID_RNG));
    }

    return SUCCESS;
}

/* Set LEDs
 *
 * @param handle    pointer to handle of LED driver
 * @param mask      set mask
 * @return error code
 */
int led_drv_set(t_led* handle, uint32_t mask)
{
    uint32_t update = (handle->led_set | mask) - handle->led_set;
    handle->led_set |= mask;

    /* Set Videoboard LEDs */
    if(update & LED_VID_RNG) {
        i2c_drv_write(handle->p_vid_i2c, LED_I2C_VID_ADDR, ~(handle->led_set & LED_VID_RNG));
    }

    return SUCCESS;
}

/* Initialize LED driver
 *
 * @param handle    pointer to handle of LED driver
 * @param p_vid_i2c pointer to handle of video i2c driver
 * @param p_aud_i2c pointer to handle of audio i2c driver
 * @return error code
 */
int led_drv_init(t_led* handle, t_i2c* p_vid_i2c, t_i2c* p_aud_i2c)
{
    handle->p_vid_i2c = p_vid_i2c;
    handle->p_aud_i2c = p_aud_i2c;

    handle->led_set = 0;
    led_drv_clr(handle, handle->led_set);

    return SUCCESS;
}
