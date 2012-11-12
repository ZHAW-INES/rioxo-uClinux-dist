/*
 * led_drv_instructions.h
 *
 *  Created on: 27.07.2011
 *      Author: buan
 */

#ifndef LED_DRV_INSTRUCTIONS_H_
#define LED_DRV_INSTRUCTIONS_H_


/* instruction codes */

// Main-Board
#define ETHERNET_ACTIVE                 (0x00000001)
#define ETHERNET_INACTIVE               (0x00000002)
#define NO_STREAM_ACTIVE                (0x00000003)
#define STREAM_ACTIVE                   (0x00000004)

// HDMI-Board
#define DVI_IN_CONNECTED_NO_AUDIO       (0x00000005)
#define DVI_IN_CONNECTED_WITH_AUDIO     (0x00000006)
#define DVI_IN_DISCONNECTED_VRB         (0x00000007)
#define DVI_IN_DISCONNECTED_VTB         (0x00000008)
#define DVI_OUT_CONNECTED_NO_AUDIO      (0x00000009)
#define DVI_OUT_CONNECTED_WITH_AUDIO    (0x0000000A)
#define DVI_OUT_DISCONNECTED_VRB        (0x0000000B)
#define DVI_OUT_DISCONNECTED_VTB        (0x0000000C)
#define STREAM_ERROR_HDMI_IN            (0x0000000D)
#define STREAM_ERROR_HDMI_OUT           (0x00000012)

// SDI-Board
#define SDI_IN_CONNECTED_NO_AUDIO       (0x00000014)
#define SDI_IN_CONNECTED_WITH_AUDIO     (0x0000001B)
#define SDI_IN_DISCONNECTED             (0x00000015)
#define SDI_LOOP_ON_NO_AUDIO            (0x00000016)
#define SDI_LOOP_ON_WITH_AUDIO          (0x0000001C)
#define SDI_LOOP_OFF                    (0x00000017)
#define SDI_OUT_NO_AUDIO                (0x00000018)
#define SDI_OUT_WITH_AUDIO              (0x00000019)
#define SDI_OUT_OFF                     (0x0000001A)

// Mixed
#define FIRMWARE_START                  (0x00000011)    //do not change this (value is used in webinterface)
#define IDENTIFICATION_ON               (0x0000000E)    //do not change this (value is used in webinterface)
#define IDENTIFICATION_OFF              (0x0000000F)    //do not change this (value is used in webinterface)
#define CONFIGURE_VRB                   (0x00000013)
#define CONFIGURE_VTB                   (0x00000010)

#endif /* LED_DRV_INSTRUCTIONS_H_ */
