/*
 * adv9889_drv.c
 *
 *  Created on: 23.08.2010
 *      Author: alda
 */
#include "adv9889_drv.h"

static inline void adv9889_write(t_adv9889* handle, uint8_t a, uint8_t x) {
	i2c_drv_wreg8(handle->p_i2c, ADV9889_ADDRESS, a, x);
}

static inline int adv9889_read(t_adv9889* handle, uint8_t a) {
	return i2c_drv_rreg8(handle->p_i2c, ADV9889_ADDRESS, a);
}

int adv9889_drv_hdcp_on(t_adv9889* handle) 
{   
    adv9889_drv_av_mute(handle);
    handle->repeater = 0;
    handle->bksv_cnt = 0;
    handle->hdcp_cnt = 0; 
    handle->hdcp_state = HDCP_INIT; 
    handle->hdcp_en = true;
    handle->hdcp_wait_cnt = 0;
    
    return SUCCESS;
}

int adv9889_drv_hdcp_off(t_adv9889* handle)
{
    uint8_t reg;
    
    reg = adv9889_read(handle, ADV9889_OFF_MODE);
    adv9889_write(handle, ADV9889_OFF_MODE, reg & ~(ADV9889_MODE_HDCP | ADV9889_MODE_ENCRYPT));
    
    handle->hdcp_en = false;
    handle->hdcp_wait_cnt = 0;
    handle->hdcp_state = HDCP_OFF;
    REPORT(INFO, "[HDMI OUT] HDCP disabled");
	
    return SUCCESS;
}

void adv9889_drv_default_reg(t_adv9889* handle)
{
    adv9889_write(handle, 0x0a, 0x00);
    adv9889_write(handle, 0x98, 0x07);
    adv9889_write(handle, 0x9c, 0x38);
    adv9889_write(handle, 0x9d, 0x61);
    adv9889_write(handle, 0x9f, 0x70);
    adv9889_write(handle, 0xa2, 0x87);
    adv9889_write(handle, 0xa3, 0x87);
    adv9889_write(handle, 0xbb, 0xff);
}

int adv9889_drv_init(t_adv9889* handle, t_i2c* p_i2c, t_vio* p_vio)
{
    int status;

    REPORT(INFO, ">> ADV9889-Driver: Initialize HDMI-TX");

    handle->p_i2c           = p_i2c;
    handle->p_vio           = p_vio;
    handle->edid_segment    = 0x00;
    handle->edid_first      = 0;
    handle->av_mute         = 0;
    handle->hdcp_state      = HDCP_OFF;
    handle->hdcp_cnt        = 0;
    handle->hdcp_en         = false;
    handle->hdcp_wait_cnt   = 0;
    handle->repeater        = 0;
    handle->bksv_cnt        = 0;
    handle->edid_timeout    = 0;

	// shut down...
    adv9889_write(handle, ADV9889_OFF_PWR_DOWN, ADV9889_PWR_DOWN_ON);
	adv9889_write(handle, ADV9889_OFF_SYSTEM, ADV9889_SYSTEM_DOWN);

    // must be written for proper operation [AD9889B Programmer Guide 24.5.2007]
    adv9889_drv_default_reg(handle);

    // Interrupt enable
    adv9889_write(handle, ADV9889_OFF_INTEN1, ADV9889_INT1_ON | ADV9889_INT1_EDID);
    adv9889_write(handle, ADV9889_OFF_INTEN2, ADV9889_INT2_HDCP_ERR | ADV9889_INT2_BKSV);

    // try power-up
    status = adv9889_read(handle, ADV9889_OFF_STATUS);
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
    
    handle->hdcp_state = HDCP_OFF;
    adv9889_write(handle, ADV9889_OFF_PWR_DOWN, ADV9889_PWR_DOWN_ON);
    adv9889_write(handle, ADV9889_OFF_SYSTEM, ADV9889_SYSTEM_DOWN);

    // Interrupt enable
    adv9889_write(handle, ADV9889_OFF_INTEN1, ADV9889_INT1_ON);
    adv9889_write(handle, ADV9889_OFF_INTEN2, ADV9889_INT2_HDCP_ERR | ADV9889_INT2_BKSV);

    return SUCCESS;
}

int adv9889_drv_powerup(t_adv9889* handle)
{
    handle->edid_segment    = 0x00;
    handle->edid_first      = 1;
    handle->hdcp_state      = HDCP_OFF;

    adv9889_write(handle, ADV9889_OFF_PWR_DOWN, ADV9889_PWR_DOWN_ON);

    // Start operation
    adv9889_write(handle, ADV9889_OFF_SYSTEM, ADV9889_SYSTEM_UP);
    adv9889_write(handle, ADV9889_OFF_FORMAT, ADV9889_FORMAT_SET_AVMUTE);

    // Interrupt enable
    adv9889_write(handle, ADV9889_OFF_INTEN1, ADV9889_INT1_ON | ADV9889_INT1_EDID | ADV9889_INT1_VS);
    adv9889_write(handle, ADV9889_OFF_INTEN2, ADV9889_INT2_HDCP_ERR | ADV9889_INT2_BKSV);

    return SUCCESS;
}

int adv9889_drv_hdmi(t_adv9889* handle)
{
    uint8_t *ext = 0, *cur, *stop;
    int is_hdmi = 0;

    for (int i=0;(i<handle->edid[126])&&!ext;i++) {
        if (handle->edid[128+128*i] == 0x02) {
            // CEA-861 extansion found
            ext = &handle->edid[128+128*i];
        }
    }

    // no CEA-861 -> can't be HDMI
    if (!ext) return 0;

    cur = ext + 4;
    stop = ext + ext[2];

    while ((cur<stop) && !is_hdmi) {
        if ((cur[0] & 0xe0) == VENDOR_BLOCK) {
            // vendor block
            if ((cur[1] == HDMI_IDENTIFIER_1) &&
                (cur[2] == HDMI_IDENTIFIER_2) &&
                (cur[3] == HDMI_IDENTIFIER_3)) {
                is_hdmi = 1;
            }
        }
        cur += ((cur[0] & 0x1f) + 1);
    }

    return is_hdmi;
}

int adv9889_drv_boot(t_adv9889* handle)
{
    adv9889_drv_default_reg(handle);
    adv9889_write(handle, ADV9889_OFF_I2S, ADV9889_I2S_STD);
    adv9889_write(handle, ADV9889_OFF_AUDIO, ADV9889_AUDIO_I2S);
    adv9889_write(handle, ADV9889_OFF_SETTINGS, ADV9889_ACCURACY_1000PPM);

    // Selected Chip is AD9889 (internal HDCP keys)
    adv9889_write(handle, ADV9889_OFF_CLOCK, ADV9889_CLOCK_POS_EDGE | ADV9889_CLOCK_9389);
    adv9889_write(handle, ADV9889_OFF_POLARITY, (ADV9889_POLARITY_DEFAULT | ADV9889_POLARITY_VSYNC_I | ADV9889_POLARITY_HSYNC_I));

    adv9889_write(handle, ADV9889_OFF_INFO2, ADV9889_INFO2_AAR_SAME); 

    // HDMI configuration
    if (adv9889_drv_hdmi(handle)) {
        adv9889_write(handle, ADV9889_OFF_MODE, ADV9889_MODE_HDMI);
        REPORT(INFO, "adv9889 activate HDMI");
    } else {
        REPORT(INFO, "adv9889 activate DVI");
    }

    // configure audio
    adv9889_drv_setup_audio(handle, handle->audio_cnt, handle->audio_fs, handle->audio_width);

    // AV mute off
    adv9889_write(handle, ADV9889_OFF_PWR_DOWN, ADV9889_PWR_DOWN_OFF);
    adv9889_write(handle, ADV9889_OFF_FORMAT, ADV9889_FORMAT_CLR_AVMUTE);

    return SUCCESS;
}

int adv9889_drv_setup_audio(t_adv9889* handle, int ch, int fs, int width)
{
    uint32_t m = ADV9889_I2S_STD;
    uint8_t fs_code = 0;
    uint8_t wd_code = 0;

    handle->audio_cnt = ch;
    handle->audio_fs = fs;
    handle->audio_width = width;

    adv9889_write(handle, ADV9889_OFF_AUD_CHCNT, ADV9889_AUD_CHANNEL(ch));

    if (ch > 0) m |= ADV9889_I2S_EN0;
    if (ch > 2) m |= ADV9889_I2S_EN1;
    if (ch > 4) m |= ADV9889_I2S_EN2;
    if (ch > 6) m |= ADV9889_I2S_EN3;
    adv9889_write(handle, ADV9889_OFF_I2S, m);

    m = 128*fs/1000;
    adv9889_write(handle, ADV9889_OFF_N_2, (m>>16)&0x0f);
    adv9889_write(handle, ADV9889_OFF_N_1, (m>> 8)&0xff);
    adv9889_write(handle, ADV9889_OFF_N_0, (m>> 0)&0xff);

    switch (fs) {
        case 32000 :  fs_code |= 0x30; break;
        case 44100 :  fs_code |= 0x00; break;
        case 48000 :  fs_code |= 0x20; break;
        case 88200 :  fs_code |= 0x80; break;
        case 96000 :  fs_code |= 0xA0; break;
        case 176400 : fs_code |= 0xC0; break;
        case 192000 : fs_code |= 0xE0; break;
    }
    adv9889_write(handle, ADV9889_OFF_I2S_FS, fs_code);

    switch (width) {
        case 16 : wd_code |= 0x02; break;
        case 17 : wd_code |= 0x0C; break;
        case 18 : wd_code |= 0x04; break;
        case 19 : wd_code |= 0x08; break;
        case 20 : wd_code |= 0x0A; break;
        case 21 : wd_code |= 0x0D; break;
        case 22 : wd_code |= 0x07; break;
        case 23 : wd_code |= 0x09; break;
        case 24 : wd_code |= 0x0B; break;
    }
    adv9889_write(handle, ADV9889_OFF_I2S_WORD_LENGTH, wd_code);

    return SUCCESS;
}

int adv9889_drv_av_mute(t_adv9889* handle)
{
    uint8_t tmp;

    /* ADV9889B AV mute set */
    tmp = adv9889_read(handle, ADV9889_OFF_FORMAT);
    adv9889_write(handle, ADV9889_OFF_FORMAT, (tmp | ADV9889_FORMAT_SET_AVMUTE) & (~ADV9889_FORMAT_CLR_AVMUTE));

    /* Enable general Control Packet (Rx should know that a/v is muted)*/
    tmp = adv9889_read(handle, ADV9889_OFF_CONTROL_PACKET);
    adv9889_write(handle, ADV9889_OFF_CONTROL_PACKET, tmp | ADV9889_ENABLE_CTRL_PKT);
 
  /* Color space converter (VIO) => black picture */
    vio_drv_set_black_output(handle->p_vio);
 
    /* Audio input: I2S => SPDIF */ 
    tmp = adv9889_read(handle, ADV9889_OFF_AUDIO);
    adv9889_write(handle, ADV9889_OFF_AUDIO, tmp | ADV9889_AUDIO_SPDIF);

    handle->av_mute = AV_MUTED;
    REPORT(INFO, "[HDMI OUT] Audio & Video mute");
    
    return SUCCESS;
}

int adv9889_drv_av_unmute(t_adv9889* handle)
{
    uint8_t tmp;

    /* AV mute clear */
    tmp = adv9889_read(handle, ADV9889_OFF_FORMAT);
    adv9889_write(handle, ADV9889_OFF_FORMAT, (tmp | ADV9889_FORMAT_CLR_AVMUTE) & (~ADV9889_FORMAT_SET_AVMUTE));

    /* Color space converter (VIO) => normal operation */
    vio_drv_clr_black_output(handle->p_vio);
    
    /* Audio input: SPDIF => I2S */ 
    tmp = adv9889_read(handle, ADV9889_OFF_AUDIO);
    adv9889_write(handle, ADV9889_OFF_AUDIO, tmp & ~ADV9889_AUDIO_SPDIF);

    handle->av_mute = AV_UNMUTED;
    REPORT(INFO, "[HDMI OUT] Audio & Video unmute");
    
    return SUCCESS;
}

static void adv9889_read_bksv(t_adv9889* handle) 
{
    handle->bksv[0] = adv9889_read(handle, ADV9889_OFF_BKSV0);
    handle->bksv[1] = adv9889_read(handle, ADV9889_OFF_BKSV1);
    handle->bksv[2] = adv9889_read(handle, ADV9889_OFF_BKSV2);
    handle->bksv[3] = adv9889_read(handle, ADV9889_OFF_BKSV3);
    handle->bksv[4] = adv9889_read(handle, ADV9889_OFF_BKSV4);
}

int adv9889_drv_handler(t_adv9889* handle, t_queue* event_queue)
{
    if(handle->hdcp_state != HDCP_OFF) {
        switch(handle->hdcp_state) {
            case HDCP_NO_REPEATER:      /* TODO Compare BKSVs with Revocation List */
                                        if((adv9889_read(handle, ADV9889_OFF_HDCP_STATE) & 0xF) == 4) {
                                            handle->hdcp_state = HDCP_VID_ACTIVE; 
                                        }
                                        break;

            case HDCP_REPEATER_WAIT:    if((adv9889_read(handle, ADV9889_OFF_HDCP_STATE) & 0xF) == 4) {
                                            /* If HDMI TX is part of a repeater store BSTATUS info from EDID
                                               memory 1st time this state is reached */
                                            if(handle->bksv_cnt == 0) {
                                                i2c_drv_rreg8b(handle->p_i2c, ADV9889_ADDRESS_EDID, 0xF9, &(handle->bstatus[0]), 1);
                                                i2c_drv_rreg8b(handle->p_i2c, ADV9889_ADDRESS_EDID, 0xFA, &(handle->bstatus[1]), 1);
                                            }
 
                                            /* TODO Compare BKSVs with Revocation List */
                                            handle->hdcp_state = HDCP_VID_ACTIVE;
                                        }
                                        break;

            case HDCP_VID_ACTIVE:       /* TODO if HDMI Tx is part of a repeater send DEPTH and DEVICE_COUNT to receiver */
                                        adv9889_drv_av_unmute(handle);
                                        handle->hdcp_state = HDCP_VID_ACTIVE_LOOP;

            case HDCP_VID_ACTIVE_LOOP:  handle->hdcp_cnt++;
                                        if(handle->hdcp_cnt >= HDCP_CHECK_LINK_INT) {
                                            handle->hdcp_cnt = 0;
                                            // HDCP link not okay?
                                            if((adv9889_read(handle, ADV9889_OFF_MODE_STATUS) & ADV9889_MSTATUS_ENCRYPTED) == 0) {
                                                REPORT(INFO, "[HDMI OUT] HDCP link isn't okay");
                                                adv9889_drv_av_mute(handle);
                                                adv9889_drv_hdcp_off(handle);
                                            }
                                        }
                                        break;

            default:                    break;
        }
    }

    // restart chip if EDID cannot be read
    if(handle->edid_timeout != 0) {
        handle->edid_timeout--;
    }
    if(handle->edid_timeout == 1) {
        adv9889_drv_powerdown(handle);
        adv9889_drv_powerup(handle);
        handle->edid_timeout = EDID_TIMEOUT;
    }

    return SUCCESS;
}

int adv9889_irq_handler(t_adv9889* handle, t_queue* event_queue)
{
    int irq = adv9889_read(handle, ADV9889_OFF_IRQ1);
    int irq2 = adv9889_read(handle, ADV9889_OFF_IRQ2);
    uint8_t tmp;

    if (irq & ADV9889_INT1_ON) {
        int status = adv9889_read(handle, ADV9889_OFF_STATUS);
        if ((status & ADV9889_STATUS_ON) == ADV9889_STATUS_ON) {
            REPORT(INFO, "adv9889 activated");
            adv9889_drv_powerup(handle);
            // init HDCP if enabled
            if (handle->hdcp_en) {
                adv9889_drv_hdcp_on(handle);
            }
            handle->edid_timeout = EDID_TIMEOUT;
        } else {
            REPORT(INFO, "adv9889 deactivated");
       
            adv9889_drv_powerdown(handle);
            queue_put(event_queue, E_ADV9889_CABLE_OFF);
        }
    }
    if (irq & ADV9889_INT1_EDID) {
        // read edid...
        i2c_drv_rreg8b(handle->p_i2c, ADV9889_ADDRESS_EDID, 0, &handle->edid[(handle->edid_segment*256)&0x100], 256);
        handle->edid_segment++;
        if (handle->edid_segment == 1) {
            // we support a maximum of 3 extension blocks for now!
            if (handle->edid[126] > 3) handle->edid[126] = 3;
        }

        if (handle->edid_segment < ((handle->edid[126]+1)/2)) {
            // start read next edid  segment
            adv9889_write(handle, ADV9889_OFF_EDID_SEG, handle->edid_segment);
        } else {
            if (handle->edid_first) {
                // read first time all edid segment
                handle->edid_first = 0;
                REPORT(INFO, "adv9889 config");
                adv9889_drv_boot(handle);
                handle->edid_timeout = 0;
                queue_put(event_queue, E_ADV9889_CABLE_ON);
            }
        }
    }

    // is used to enable HDCP after some vsync pulses
    if (irq & ADV9889_INT1_VS) {
        if (handle->hdcp_en) {
            if (handle->hdcp_wait_cnt == 5) {
                // enable HDCP
                tmp = adv9889_read(handle, ADV9889_OFF_MODE);
                adv9889_write(handle, ADV9889_OFF_MODE, tmp | ADV9889_MODE_HDCP | ADV9889_MODE_ENCRYPT);
                REPORT(INFO, "[HDMI OUT] HDCP enabled");
            }
            if (handle->hdcp_wait_cnt < 6){
                handle->hdcp_wait_cnt ++;
            }
        } 
    }

    if(handle->hdcp_state != HDCP_OFF) {
        if (irq2 & ADV9889_INT2_HDCP_ERR) {
            tmp = adv9889_read(handle, ADV9889_OFF_HDCP_STATE); 
            REPORT(INFO, "[HDMI OUT] HDCP error : %d (state : %d)", (tmp&0xF0)>>4, (tmp&0xF));
        }
    }

    if (irq2 & ADV9889_INT2_BKSV) {
        switch(handle->hdcp_state) {
            case HDCP_INIT:             adv9889_read_bksv(handle);
                                        handle->bksv_cnt = 0; 
                                        if(adv9889_read(handle, ADV9889_OFF_HDCP) & ADV9889_HDCP_REPEATER) {
                                           handle->hdcp_state = HDCP_REPEATER_WAIT; 
                                           handle->repeater = 1;
                                        }  else {
                                           handle->hdcp_state = HDCP_NO_REPEATER;  
                                           handle->repeater = 0; 
                                        }
                                        break;

            case HDCP_REPEATER_WAIT:    handle->hdcp_state = HDCP_REPEATER;
                                        break;

            case HDCP_REPEATER:         /* If HDMI TX is part of a repeater store BSTATUS info from EDID
                                           memory 1st time this state is reached */
                                        if(handle->bksv_cnt == 0) {
                                            i2c_drv_rreg8b(handle->p_i2c, ADV9889_ADDRESS_EDID, 0xF9, &(handle->bstatus[0]), 1);
                                            i2c_drv_rreg8b(handle->p_i2c, ADV9889_ADDRESS_EDID, 0xFA, &(handle->bstatus[1]), 1);
                                        }
                                        if((adv9889_read(handle, ADV9889_OFF_HDCP_STATE) & 0xF) == 4) {
                                            handle->hdcp_state = HDCP_VID_ACTIVE;
                                            break;
                                        }
                                        /* Check Number of BKSVs available (0xc7[6:0]) */
                                        handle->bksv_cnt = (adv9889_read(handle, ADV9889_OFF_BKSV_CNT) & 0x7F);
                                        /* Read BKSVs from EDID memory */
                                        if(handle->bksv_cnt >= 13) {
                                            i2c_drv_rreg8b(handle->p_i2c, ADV9889_ADDRESS_EDID, 0, &(handle->bksv[5]), 13*5);
                                        } else {
                                            i2c_drv_rreg8b(handle->p_i2c, ADV9889_ADDRESS_EDID, 0, &(handle->bksv[5]), handle->bksv_cnt*5);
                                        }
                                        /* TODO Compare BKSVs with Revocation List */
                                        handle->hdcp_state = HDCP_REPEATER_WAIT; 
                                        break;

            default:                    break;
        }    
    }

    adv9889_write(handle, ADV9889_OFF_IRQ1, irq);
    adv9889_write(handle, ADV9889_OFF_IRQ2, irq2);

    return SUCCESS;
}

int adv9889_drv_get_edid(t_adv9889* handle, uint8_t* mem)
{
    //i2c_drv_rreg8b(handle->p_i2c, ADV9889_ADDRESS_EDID, 0, mem, 256);
    memcpy(mem, handle->edid, 256);

    return SUCCESS;
}

