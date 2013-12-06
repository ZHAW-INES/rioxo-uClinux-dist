/*
 * led_drv.h
 *
 *  Created on: 22.03.2011
 *      Author: stth
 */

#ifndef LED_DRV_H_
#define LED_DRV_H_

#include "std.h"
#include "i2c_drv.h"
#include "led_drv_instructions.h"
#include "bdt_drv.h"
#include "hoi_msg.h"
#include "io_expander_rx.h"

/* LED board */
#define BOARD_ID_OFFSET         (4)
#define BOARD_ID                (0x1)
#define LED_I2C_BOARD_ADDR      (0x0000004A)

/* LED flashing constants */
#define LED_T_HANDLE            (30)
#define LED_T_1_SEC             (500)
#define LED_T_3_SEC             (1500)

/* LED Status*/
#define LED_ON                  (0x00000000)
#define LED_OFF                 (0x00000001)
#define LED_FLASHING_1S         (0x00000002)
#define LED_FLASHING_3S         (0x00000003)

/* Mainboard (code 0x00 - 0xFF)*/
#define LED_READ_REG            (HOI_RD32((handle->p_led), 0x00))
#define LED_BOARD_ID            (HOI_RD32((handle->p_led), BOARD_ID_OFFSET))
#define LED_SET_REG(m)		    (HOI_WR32((handle->p_led), 0x00, (m)))
#define LED_MOTHER_RNG          (0x000000FF)

#define LED_ETH_STATUS          (0x00000004)
#define LED_POWER_GREEN         (0x00000008)
#define LED_POWER_RED           (0x00000010)
#define LED_BOOT                (0x00000020)

/* Video-Board Miscellaneous */
#define LED_VIDEO_BIT_SHIFT      (0x00000008)
#define LED_VIDEO_RNG            (0x000FFFFF)

/* HDMI-Board  (code 0x100 - 0xFFFF) */
#define LED_I2C_HDMI_ADDR        (0x00000044)
#define LED_HDMI_MASK            (0x0000000F)
#define LED_HDMI_RNG             (0x0000000F)

#define LED_HDMI_OUT_ORANGE      (0x00000300)
#define LED_HDMI_OUT_GREEN       (0x00000100)
#define LED_HDMI_OUT_RED         (0x00000200)
#define LED_HDMI_IN_ORANGE       (0x00000C00)
#define LED_HDMI_IN_GREEN        (0x00000400)
#define LED_HDMI_IN_RED          (0x00000800)

/* SDI-Board (code 0x10000) - ... */
#define LED_I2C_SDI_IN_ADDR      (0x00000046)
#define LED_I2C_SDI_OUT_ADDR     (0x00000044)
#define LED_SDI_IN_MASK          (0x0000000F)
#define LED_SDI_OUT_MASK         (0x0000000F)
#define LED_SDI_IN_RNG           (0x00000F00)
#define LED_SDI_OUT_RNG          (0x0000F000)
#define LED_SDI_IN_BIT_SHIFT     (0x00000008)
#define LED_SDI_OUT_BIT_SHIFT    (0x0000000C)

#define LED_SDI_IN_ORANGE        (0x00030000)
#define LED_SDI_IN_GREEN         (0x00010000)
#define LED_SDI_IN_RED           (0x00020000)
#define LED_SDI_LOOP_ORANGE      (0x000C0000)
#define LED_SDI_LOOP_GREEN       (0x00040000)
#define LED_SDI_LOOP_RED         (0x00080000)

#define LED_SDI_OUT2_ORANGE      (0x00300000)
#define LED_SDI_OUT2_GREEN       (0x00100000)
#define LED_SDI_OUT2_RED         (0x00200000)
#define LED_SDI_OUT1_ORANGE      (0x00C00000)
#define LED_SDI_OUT1_GREEN       (0x00400000)
#define LED_SDI_OUT1_RED         (0x00800000)

// LEDs on LED board
#define LED_I2C_BOARD_ADDR       (0x0000004A)
#define LED_BOARD_MASK           (0x0000000F)
#define LED_BOARD_RNG            (0x000F0000)
#define LED_BOARD_BIT_SHIFT      (0x00000010)

#define LED_BOARD_AUDIO_ORANGE   (0x03000000)
#define LED_BOARD_AUDIO_GREEN    (0x01000000)
#define LED_BOARD_AUDIO_RED      (0x02000000)
#define LED_BOARD_VIDEO_ORANGE   (0x0C000000)
#define LED_BOARD_VIDEO_GREEN    (0x04000000)
#define LED_BOARD_VIDEO_RED      (0x08000000)

typedef struct {
    void        *p_led;
    t_i2c       *p_vid_i2c;
    t_i2c       *p_aud_i2c;
    uint32_t    device_id;
    uint32_t    led_set;
    uint32_t    counter_1s;
    uint32_t    counter_3s;
    uint32_t    toggle_mask_1s;
    uint32_t    toggle_mask_3s;
    uint32_t    set_mask;
    uint32_t    clr_mask;
} t_led;

int led_drv_init(t_led* handle, t_i2c* p_vid_i2c, t_i2c* p_aud_i2c, void* p_led, t_bdt* handle_bdt);
void led_drv_handler(t_led* handle);
void led_drv_set_status(t_led* handle, uint32_t instruction, bool vrb);

#endif /* LED_DRV_H_ */
