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

/* Videoboard */
#define LED_VID_RNG             (0x000000FF)
#define LED_VID_OUT_1_ORANGE    (0x00000003)
#define LED_VID_OUT_1_GREEN     (0x00000001)
#define LED_VID_OUT_1_RED       (0x00000002)
#define LED_VID_OUT_2           (0x00000010)
#define LED_VID_IN_1_ORANGE     (0x0000000C)
#define LED_VID_IN_1_GREEN      (0x00000004)
#define LED_VID_IN_1_RED        (0x00000008)
#define LED_VID_IN_2            (0x00000020)
#define LED_VID_BOARD_1         (0x00000040)
#define LED_VID_BOARD_2         (0x00000080)

typedef struct {
    t_i2c       *p_vid_i2c;
    t_i2c       *p_aud_i2c;
    uint32_t    led_set;
} t_led;

int led_drv_init(t_led* handle, t_i2c* p_vid_i2c, t_i2c* p_aud_i2c);
int led_drv_tgl(t_led* handle, uint32_t mask);
int led_drv_clr(t_led* handle, uint32_t mask);
int led_drv_set(t_led* handle, uint32_t mask);

#endif /* LED_DRV_H_ */
