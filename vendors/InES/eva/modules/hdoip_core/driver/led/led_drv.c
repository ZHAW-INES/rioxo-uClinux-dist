
#include "led_drv.h"


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
int led_drv_init(t_led* handle, t_i2c* p_vid_i2c, t_i2c* p_aud_i2c, void* p_led)
{
    handle->p_vid_i2c       = p_vid_i2c;
    handle->p_aud_i2c       = p_aud_i2c;
    handle->p_led           = p_led;
    handle->counter_1s      = 0;
    handle->counter_3s      = 0;
    handle->toggle_mask_1s  = 0;
    handle->toggle_mask_3s  = 0;
    handle->led_set         = 0;

    led_drv_set_status(handle, FIRMWARE_START);

    return SUCCESS;
}

/* LED handler for flashing leds
 *
 * @param handle    pointer to handle of LED driver
 * @return nothing
 */
void led_drv_handler(t_led* handle)
{ 
    uint8_t led_status;

    // timer ~1 sec
    if (handle->counter_1s == (LED_T_1_SEC/LED_T_HANDLE)) {
        handle->counter_1s = 0;
            // toggle leds on mainboard
            led_status = LED_READ_REG;
            led_status = led_status^(handle->toggle_mask_1s & 0xFF);
            LED_SET_REG(led_status);
            // toggle leds on hdmi-board
            led_drv_tgl(handle, ((handle->toggle_mask_1s >> 8) & 0xFF));
    } else {
        handle->counter_1s++; 
    }

    // timer ~3 sec
    if (handle->counter_3s == (LED_T_3_SEC/LED_T_HANDLE)) {
        handle->counter_3s = 0;
            // toggle leds on mainboard
            led_status = LED_READ_REG;
            led_status = led_status^(handle->toggle_mask_3s & 0xFF);
            LED_SET_REG(led_status);
            // toggle leds on hdmi-board
            led_drv_tgl(handle, ((handle->toggle_mask_3s >> 8) & 0xFF));
    } else {
        handle->counter_3s++; 
    }

    // clear
    if (handle->clr_mask) {
        LED_SET_REG(LED_READ_REG | (handle->clr_mask & 0xFF));
        led_drv_clr(handle, handle->clr_mask >> 8);
        handle->clr_mask = 0;
    }

    // set
    if (handle->set_mask) { 
        LED_SET_REG(LED_READ_REG & (~(handle->set_mask & 0xFF)));
        led_drv_set(handle, handle->set_mask >> 8);
        handle->set_mask = 0;
    }
}

/* Set LED on, off or blinking
 *
 * @param handle    pointer to handle of LED driver
 * @param led       code of LED
 * @param status    code of LED-status
 * @return nothing
 */
void led_drv_control_set(t_led* handle, uint32_t led, uint32_t status) 
{
    switch (status){
        case LED_ON          :  handle->set_mask |= led;
                                handle->toggle_mask_1s &= (~led);
                                handle->toggle_mask_3s &= (~led);
                                break;

        case LED_OFF         :  handle->clr_mask |= led;
                                handle->toggle_mask_1s &= (~led);
                                handle->toggle_mask_3s &= (~led);
                                break;

        case LED_FLASHING_1S :  handle->toggle_mask_1s |= led;
                                handle->toggle_mask_3s &= (~led);
                                break;

        case LED_FLASHING_3S :  handle->toggle_mask_1s &= (~led);
                                handle->toggle_mask_3s |= led;
    }
}

/* set led to a specific state
 *
 * @param handle        pointer to handle of LED driver
 * @param instruction   instruction code
 * @return nothing
 */
int led_drv_set_status(t_led* handle, uint32_t instruction)
{
    switch (instruction){
        case ETHERNET_ACTIVE                :   led_drv_control_set(handle, LED_ETH_STATUS,     LED_ON);
                                                led_drv_control_set(handle, LED_POWER_GREEN,    LED_FLASHING_1S);
                                                break;

        case ETHERNET_INACTIVE              :   led_drv_control_set(handle, LED_ETH_STATUS,     LED_OFF);
                                                led_drv_control_set(handle, LED_POWER_GREEN,    LED_FLASHING_3S);
                                                break;

        case DVI_IN_CONNECTED_NO_AUDIO      :   led_drv_control_set(handle, LED_VID_IN_GREEN,   LED_FLASHING_3S);
                                                led_drv_control_set(handle, LED_VID_IN_RED,     LED_OFF);
                                                break;

        case DVI_IN_CONNECTED_WITH_AUDIO    :   led_drv_control_set(handle, LED_VID_IN_GREEN,   LED_ON);
                                                led_drv_control_set(handle, LED_VID_IN_RED,     LED_OFF);
                                                break;

        case DVI_IN_DISCONNECTED            :   led_drv_control_set(handle, LED_VID_IN_GREEN,   LED_OFF);
                                                led_drv_control_set(handle, LED_VID_IN_RED,     LED_OFF);
                                                break;

        case DVI_OUT_CONNECTED_NO_AUDIO     :   led_drv_control_set(handle, LED_VID_OUT_GREEN,  LED_FLASHING_3S);
                                                led_drv_control_set(handle, LED_VID_OUT_RED,    LED_OFF);
                                                break;

        case DVI_OUT_CONNECTED_WITH_AUDIO   :   led_drv_control_set(handle, LED_VID_OUT_GREEN,  LED_ON);
                                                led_drv_control_set(handle, LED_VID_OUT_RED,    LED_OFF);
                                                break;

        case DVI_OUT_DISCONNECTED           :   led_drv_control_set(handle, LED_VID_OUT_GREEN,  LED_OFF);
                                                led_drv_control_set(handle, LED_VID_OUT_RED,    LED_OFF);
                                                break;

        case FIRMWARE_START                 :   led_drv_control_set(handle, LED_BOOT,           LED_ON);
                                                led_drv_control_set(handle, LED_ETH_STATUS,     LED_OFF);
                                                led_drv_control_set(handle, LED_POWER_GREEN,    LED_FLASHING_3S);
                                                led_drv_control_set(handle, LED_POWER_RED,      LED_OFF);
                                                led_drv_control_set(handle, LED_VID_IN_RED,     LED_OFF);
                                                led_drv_control_set(handle, LED_VID_IN_GREEN,   LED_OFF);
                                                led_drv_control_set(handle, LED_VID_OUT_RED,    LED_OFF);
                                                led_drv_control_set(handle, LED_VID_OUT_GREEN,  LED_OFF);
                                                break;

        case NO_STREAM_ACTIVE               :   led_drv_control_set(handle, LED_POWER_GREEN,    LED_FLASHING_1S);
                                                break;

        case STREAM_ACTIVE                  :   led_drv_control_set(handle, LED_POWER_GREEN,    LED_ON);
                                                break;

        case STREAM_ERROR_HDMI_IN           :   led_drv_control_set(handle, LED_VID_IN_GREEN,   LED_OFF);
                                                led_drv_control_set(handle, LED_VID_IN_RED,     LED_ON);
                                                break;

        case STREAM_ERROR_HDMI_OUT          :   led_drv_control_set(handle, LED_VID_OUT_GREEN,  LED_OFF);
                                                led_drv_control_set(handle, LED_VID_OUT_RED,    LED_ON);
                                                break;

        case IDENTIFICATION_ON              :   led_drv_control_set(handle, LED_POWER_RED,      LED_FLASHING_1S);
                                                led_drv_control_set(handle, LED_VID_IN_RED,     LED_FLASHING_1S);
                                                led_drv_control_set(handle, LED_VID_OUT_RED,    LED_FLASHING_1S);
                                                break;

        case IDENTIFICATION_OFF             :   led_drv_control_set(handle, LED_POWER_RED,      LED_OFF);
                                                led_drv_control_set(handle, LED_VID_IN_RED,     LED_OFF);
                                                led_drv_control_set(handle, LED_VID_OUT_RED,    LED_OFF);
                                                break;

        default                             :   break;
    }
}
