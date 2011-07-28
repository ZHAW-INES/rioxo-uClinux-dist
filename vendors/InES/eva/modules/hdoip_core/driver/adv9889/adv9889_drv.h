/*
 * adv9889_drv.h
 *
 *  Created on: 20.08.2010
 *      Author: alda
 */

#ifndef ADV9889_DRV_H_
#define ADV9889_DRV_H_

#include "i2c_drv.h"
#include "vio_drv.h"
#include "adv9889_reg.h"
#include "queue.h"

#define ADV9889_INT1_ON     (ADV9889_INT1_MSEN|ADV9889_INT1_HPD)
#define ADV9889_STATUS_ON   (ADV9889_STATUS_MSEN|ADV9889_STATUS_HPD)
#define HDMI_IDENTIFIER_1   (0x03)
#define HDMI_IDENTIFIER_2   (0x0C)
#define HDMI_IDENTIFIER_3   (0x00)
#define VENDOR_BLOCK        (0x60)
#define AV_MUTED            (0x01)
#define AV_UNMUTED          (0x00)
#define HDCP_CHECK_LINK_INT (0x32)      /* 50 intervall */
#define EDID_TIMEOUT        (0x28)      /* Timeout if EDID cannot be read */

enum {
    HDCP_OFF = 0x00,
    HDCP_INIT = 0x01,
    HDCP_NO_REPEATER = 0x02,
    HDCP_VID_ACTIVE = 0x03, 
    HDCP_VID_ACTIVE_LOOP = 0x04,
    HDCP_REPEATER = 0x05,
    HDCP_REPEATER_WAIT = 0x06 
};

typedef struct {
	t_i2c    		*p_i2c;       	//!< I2C driver
    t_vio           *p_vio;
	uint32_t        edid_segment;   //!< address offset of edid
	uint32_t        edid_first;     //!< first edid segment read
	uint32_t        audio_cnt;
	uint32_t        audio_fs;
    uint32_t        audio_width;
	uint8_t         edid[512];      //!< edid + up to three segments
    int             av_mute;
    int             hdcp_state;
    int             hdcp_cnt;
    bool            repeater;
    uint8_t         bksv_cnt;
    uint8_t         bksv[14*5];
    uint8_t         bstatus[2];
    uint32_t        edid_timeout;
} t_adv9889;


int adv9889_drv_init(t_adv9889* handle, t_i2c* p_i2c, t_vio* vio);
int adv9889_drv_setup_audio(t_adv9889* handle, int ch, int fs, int width);
int adv9889_irq_handler(t_adv9889* handle, t_queue* event_queue);
int adv9889_drv_powerup(t_adv9889* handle);
int adv9889_drv_get_edid(t_adv9889* handle, uint8_t* mem);
int adv9889_drv_hdcp_on(t_adv9889* handle); 
int adv9889_drv_hdcp_off(t_adv9889* handle);
int adv9889_drv_av_mute(t_adv9889* handle);
int adv9889_drv_av_unmute(t_adv9889* handle);
int adv9889_drv_handler(t_adv9889* handle, t_queue* event_queue);


#endif /* ADV9889_DRV_H_ */
