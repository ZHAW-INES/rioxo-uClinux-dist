/*
 * led_drv_instructions.h
 *
 *  Created on: 27.07.2011
 *      Author: buan
 */

#ifndef LED_DRV_INSTRUCTIONS_H_
#define LED_DRV_INSTRUCTIONS_H_


/* instruction codes */
#define ETHERNET_ACTIVE                 (0x00000001)
#define ETHERNET_INACTIVE               (0x00000002)
#define DVI_IN_CONNECTED_NO_AUDIO       (0x00000003)
#define DVI_IN_CONNECTED_WITH_AUDIO     (0x00000004)
#define DVI_IN_DISCONNECTED             (0x00000005)
#define DVI_OUT_CONNECTED_NO_AUDIO      (0x00000006)
#define DVI_OUT_CONNECTED_WITH_AUDIO    (0x00000007)
#define DVI_OUT_DISCONNECTED            (0x00000008)
#define FIRMWARE_START                  (0x00000009)
#define NO_STREAM_ACTIVE                (0x0000000A)
#define STREAM_ACTIVE                   (0x0000000B)
#define STREAM_ERROR_HDMI_IN            (0x0000000C)
#define STREAM_ERROR_HDMI_OUT           (0x0000000D)
#define IDENTIFICATION_ON               (0x0000000E)
#define IDENTIFICATION_OFF              (0x0000000F)


#endif /* LED_DRV_INSTRUCTIONS_H_ */
