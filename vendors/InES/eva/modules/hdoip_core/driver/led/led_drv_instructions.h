/*
 * led_drv_instructions.h
 *
 *  Created on: 27.07.2011
 *      Author: buan
 */

#ifndef LED_DRV_INSTRUCTIONS_H_
#define LED_DRV_INSTRUCTIONS_H_


/* instruction codes */
#define ETHERNET_ACTIVE         (0x00000001)
#define ETHERNET_INACTIVE       (0x00000002)
#define DVI_IN_CONNECTED        (0x00000003)
#define DVI_IN_DISCONNECTED     (0x00000004)
#define DVI_OUT_CONNECTED       (0x00000005)
#define DVI_OUT_DISCONNECTED    (0x00000006)
#define FIRMWARE_START          (0x00000007)
#define NO_STREAM_ACTIVE        (0x00000008)
#define STREAM_ACTIVE           (0x00000009)
#define STREAM_ERROR_HDMI_IN    (0x0000000A)
#define STREAM_ERROR_HDMI_OUT   (0x0000000B)
#define IDENTIFICATION_ON       (0x0000000C)
#define IDENTIFICATION_OFF      (0x0000000D)


#endif /* LED_DRV_INSTRUCTIONS_H_ */
