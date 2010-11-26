
#include "adv7441a_drv.h"

static t_adv7441a *adv7441a_handle; /* needed for adv7441a_proc_video_in() */
/* Register => value conversion arrays */
const __u32 adv7441a_audio_fs_conv[] = {44100, 0 ,48000, 32000, 0, 0, 0, 0, 88200, 0, 96000, 0, 176000, 0, 192000};
const __u32 adv7441a_audio_bit_conv[2][7] = {{0, 16, 18, 0, 19, 20, 17},{0, 20, 22, 0, 23, 24, 21}};

/** Executes a software reset 
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */
int adv7441a_sw_reset(t_adv7441a* handle) 
{
	handle->status = handle->status & ~ADV7441A_STATUS_ACTIVE;
	adv7441a_lock(handle);
	adv7441a_usr_map_write(handle, ADV7441A_REG_POWER_MANAGEMENT, ADV7441A_BIT_RES);
	//schedule_timeout(jiffies + ADV7441A_RESET_WAIT); /* wait 5ms */
	mdelay(5);
	adv7441a_unlock(handle);
	handle->status = handle->status | ADV7441A_STATUS_ACTIVE;
	return ERR_ADV7441A_SUCCESS;
}

/** Enables drivers and and get out of the power down mode
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */
int adv7441a_on(t_adv7441a* handle) 
{
	int tmp;	

	if((handle->status & ADV7441A_STATUS_ACTIVE) == 0) {
		adv7441a_lock(handle);
		tmp = adv7441a_usr_map_read(handle, ADV7441A_REG_OUTPUT_CONTROL);
		adv7441a_usr_map_write(handle, ADV7441A_REG_OUTPUT_CONTROL, tmp & ~(ADV7441A_BIT_TOD)); /* output drivers active */
		tmp = adv7441a_usr_map_read(handle, ADV7441A_REG_EXTENDED_OUTPUT_CONTROL);
		adv7441a_usr_map_write(handle, ADV7441A_REG_EXTENDED_OUTPUT_CONTROL, tmp | ADV7441A_BIT_TIM_OE); /* timing signals enabled */ 
		adv7441a_usr_map_write(handle, ADV7441A_REG_POWER_MANAGEMENT, 0x00);	/* power down mode off */ 
		adv7441a_unlock(handle);
		handle->status = handle->status | ADV7441A_STATUS_ACTIVE;
	}
	return ERR_ADV7441A_SUCCESS;
}

/** Disable drivers and puts adv7441a in power down mode
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */
int adv7441a_off(t_adv7441a* handle) 
{
	int tmp;
	
	if((handle->status & ADV7441A_STATUS_ACTIVE) != 0) {
		adv7441a_lock(handle);
		tmp = adv7441a_usr_map_read(handle, ADV7441A_REG_OUTPUT_CONTROL);
		adv7441a_usr_map_write(handle, ADV7441A_REG_OUTPUT_CONTROL, tmp | ADV7441A_BIT_TOD); /* output drivers tri-stated */
		tmp = adv7441a_usr_map_read(handle, ADV7441A_REG_EXTENDED_OUTPUT_CONTROL);
		adv7441a_usr_map_write(handle, ADV7441A_REG_EXTENDED_OUTPUT_CONTROL, tmp & (~ADV7441A_BIT_TIM_OE)); /* timing signals tri-stated */
		adv7441a_usr_map_write(handle, ADV7441A_REG_POWER_MANAGEMENT, ADV7441A_BIT_PWRDN | ADV7441A_BIT_PDBP);	/* power down mode on */ 
		adv7441a_unlock(handle);
		handle->status = handle->status & ~ADV7441A_STATUS_ACTIVE;
	}
	return ERR_ADV7441A_SUCCESS;
}

/** Mute the audio output
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */
int adv7441a_audio_mute(t_adv7441a* handle) 
{
	int tmp;

	if(handle->aud_st.mute != 1) {
		adv7441a_lock(handle);
		tmp = adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_1DH);
		adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_1DH, tmp | ADV7441A_BIT_MUTE_AUDIO);	
		adv7441a_unlock(handle);
		handle->aud_st.mute = 1;
	}
	return ERR_ADV7441A_SUCCESS;
}

/** Unmute the audio output
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */
int adv7441a_audio_unmute(t_adv7441a* handle) 
{
	int tmp;

	if(handle->aud_st.mute != 0) {	
		adv7441a_lock(handle);
		tmp = adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_1DH);
		adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_1DH, tmp & ~ADV7441A_BIT_MUTE_AUDIO);	
		adv7441a_unlock(handle);
		handle->aud_st.mute = 0;
	}
	return ERR_ADV7441A_SUCCESS;
}

/** Load EDID, calculates the checksums and enables EDID
 *
 * @param handle pointer to the adv7441a handle
 * @param edid pointer to the EDID file
 * @return error code
 */
int adv7441a_set_edid(t_adv7441a* handle, char* edid)
{
	int i;

	adv7441a_lock(handle);
	adv7441a_ksv_map_write(handle, ADV7441A_REG_CTRL_BITS, 0); /* EDID disabled */
	
	for(i=0 ; i<0xFF ; i++) {	
		if(i!=0x7F) { /* not checksum field */ 
			adv7441a_edid_map_write(handle, i, edid[i]);
		}	
	}

	/* EDID enabled & calculate checksums*/
	adv7441a_ksv_map_write(handle, ADV7441A_REG_CTRL_BITS, ADV7441A_BIT_EDID_A_ENABLE | ADV7441A_BIT_KSV_LIST_READY); 
	adv7441a_unlock(handle);
	return ERR_ADV7441A_SUCCESS;
}

/** Initialize ADV7441A core driver
 *
 * @param handle pointer to the adv7441a handle
 * @param p_i2c pointer to the i2c controller
 * @param edid pointer to the EDID file
 * @return error code
 */
int adv7441a_drv_init(t_adv7441a* handle, t_i2c* p_i2c, char* edid)
{

	REPORT(INFO, "+--------------------------------------------------+");
	REPORT(INFO, "| ADV7441A-Driver: Initialize HDMI-RX              |");
	REPORT(INFO, "+--------------------------------------------------+");

	handle->p_i2c = p_i2c;	
	adv7441a_handle = handle;
	handle->status = 0; // clear status

	adv7441a_sw_reset(handle);

	adv7441a_lock(handle);
	/* ************************************** * 
	    User map
	 * ************************************** */ 

	/* Extended output control */
	adv7441a_usr_map_write(handle, ADV7441A_REG_EXTENDED_OUTPUT_CONTROL, 
		ADV7441A_BIT_RANGE | /* reset value */
		ADV7441A_BIT_EN_SFL_PIN | /* reset value */ 
		ADV7441A_BIT_BL_C_VBI | /* reset value */ 
		ADV7441A_BIT_TIM_OE | 
		ADV7441A_BIT_INT2_EN);

	/* PRIM_MODE Register 0x05 (User Map)
		0x04 : HDMI SD
		0x05 : HDMI ED / HD
		0x06 : HDMI VGA to SXGA	*/
	adv7441a_usr_map_write(handle, ADV7441A_REG_PRIMARY_MODE, 0x06);

	/* VID_STD Register 0x06 (User Map) */
	adv7441a_usr_map_write(handle, ADV7441A_REG_VIDEO_STANDARD,0x00);

	/* Power down ADC[0-3] */ 
	adv7441a_usr_map_write(handle, ADV7441A_REG_ADC_CONTROL, 
		ADV7441A_BIT_PDN_ADC0 | 
		ADV7441A_BIT_PDN_ADC1 | 
		ADV7441A_BIT_PDN_ADC2 | 
		ADV7441A_BIT_PDN_ADC3); 

	/* SET ADC[0-3]_SW to 0	=> ADCs not connected */
	adv7441a_usr_map_write(handle, ADV7441A_REG_ADC_SWITCH_1, 0x00); /* ADC0 & ADC 1 */
	adv7441a_usr_map_write(handle, ADV7441A_REG_ADC_SWITCH_2, 0x00); /* ADC2 */
	adv7441a_usr_map_write(handle, ADV7441A_REG_AFE_CONTROL_1, 0x00); /* ADC3 */

	/* Set crystal mode and LLC output active */ 
	adv7441a_usr_map_write(handle, ADV7441A_REG_VERTICAL_SCALE_VALUE_1, ADV7441A_BIT_EN28XTAL);

	/* Set output format and enable output pins 
		0x00 : 10 bit 4:2:2
		0x04 : 20 bit 4:2:2
		0x08 : 16 bit 4:2:2
		0x0A : 8  bit 4:2:2 ITU-R BT.656 
		0x10 : 30 bit 4:4:4 
		0x14 : 24 bit 4:4:4 */
	adv7441a_usr_map_write(handle, ADV7441A_REG_OUTPUT_CONTROL, 0x10);
	
	/* Drive strength for HS, VS, f and clock set to maximum */ 
	adv7441a_usr_map_write(handle, ADV7441A_REG_DRIVE_STRENGTH, 0x3F);

		
	/* Set CSC Coeff to automatic mode (CSC_23) */ 
	adv7441a_usr_map_write(handle, ADV7441A_REG_CSC_23, 0xF0 | ADV7441A_BIT_RGB_OUT);

	/* Set CP AV controll register */ 
	adv7441a_usr_map_write(handle, ADV7441A_REG_CP_AV_CONTROL, ADV7441A_BIT_AV_BLANK_EN | ADV7441A_BIT_DE_WITH_AVCODE);	
	
	/* Set CP output selection register */ 
	adv7441a_usr_map_write(handle, ADV7441A_REG_CP_OUTPUT_SELECTION, 0x02 | ADV7441A_BIT_DE_OUT_SEL |
		ADV7441A_BIT_VS_OUT_SEL |
		ADV7441A_BIT_HS_OUT_SEL);

	/* ************************************** * 
	    User map 1
	 * ************************************** */

	/* Int 1 & int 2 configuration */ 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_CONFIGURATION_0, 0xD2); /* active until cleared, active low */ 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_CONFIGURATION_1, 0xC2); 

	adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_MASKB_1,0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_MASKB_2,0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_MASKB_3,0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_MASKB_4,0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_1, ADV7441A_BIT_AUDIO_INFO_MB1); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_2, ADV7441A_BIT_HDMI_ENCRPT_MB1); 
 	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_3, ADV7441A_BIT_V_LOCKED_MB1 | ADV7441A_BIT_TMDS_CLK_A_MB1); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_4, 0x00); 
 	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_5, 0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_6, 0x00); 

	adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_2_MASKB_1,0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_2_MASKB_2,0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_2_MASKB_3,0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_2_MASKB_4,0x00);
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_1, 0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_2, 0x00);
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_3, 0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_4, 0xFF);
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_5, 0x00); 
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_6, 0x00); 
	
	/* ************************************** * 
	    HDMI map
	 * ************************************** */
 
	/* HDMI Port select => Port A */ 
	adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_00H, 0x00);

	/* Set Mute mask (disable all) */
	adv7441a_hdmi_map_write(handle,ADV7441A_REG_MUTE_MASK_19_16, 0x00);
	adv7441a_hdmi_map_write(handle,ADV7441A_REG_MUTE_MASK_15_8, 0x00);
	adv7441a_hdmi_map_write(handle,ADV7441A_REG_MUTE_MASK_7_0, 0x00);
	
	/* Audio Mute options and conversion mode */ 
	adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_1DH, ADV7441A_BIT_UP_CONVERSION_MODE);

	/* Disable audio pll, MCLK fs, MCLK N */
	adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_1CH, 0x20);
 
	adv7441a_unlock(handle);

  	/* Set TMDS equalizer settings */ 
	adv7441a_set_tmds_equa(handle);
	
	/* Initialize EDID for port A */
	adv7441a_set_edid(handle, edid);

	return ERR_ADV7441A_SUCCESS;
}

/** Sets TMDS equalizer dependent on TMDS frequency 
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */ 
int adv7441a_set_tmds_equa(t_adv7441a* handle)
{
	uint8_t tmds_freq =  adv7441a_hdmi_map_read(adv7441a_handle,ADV7441A_REG_TMDSFREQ);

	adv7441a_lock(handle);
	if(tmds_freq < 160) {
		/* TMDS equalizer settings (TMDS frequencies < 160MHz) */
		adv7441a_usr_map2_write(handle, 0xF0, 0x10);
		adv7441a_usr_map2_write(handle, 0xF1, 0x0F);
		adv7441a_usr_map2_write(handle, 0xF4, 0x20);
	} else {
		/* TMDS equalizer settings (TMDS frequencies >= 160MHz) */
		adv7441a_usr_map2_write(handle, 0xF0, 0x30);
		adv7441a_usr_map2_write(handle, 0xF1, 0x0F);
		adv7441a_usr_map2_write(handle, 0xF4, 0x20);
	}
	adv7441a_unlock(handle);
	REPORT(INFO, "HDMI TMDS equalizer settings set");
	return ERR_ADV7441A_SUCCESS;
}

/** Reads all audio parameters and save it in the handle 
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */
static int adv7441a_get_audio_timing(t_adv7441a* handle)
{
	uint8_t tmp;
	adv7441a_lock(handle);

	tmp = adv7441a_hdmi_map_read(handle,ADV7441A_REG_CHANNEL_STATUS_DATA_3);
	handle->aud_st.fs = adv7441a_audio_fs_conv[tmp & 0xF];
	tmp = adv7441a_hdmi_map_read(handle,ADV7441A_REG_CHANNEL_STATUS_DATA_4);
	handle->aud_st.bit_width = adv7441a_audio_bit_conv[tmp&0x1][((tmp & 0x1E)>>1)];
	tmp = adv7441a_usr_map1_read(handle,ADV7441A_REG_HDMI_RAW_STATUS_3);
	handle->aud_st.mute = (tmp & ADV7441A_BIT_INTERNAL_MUTE_RAW) >> 5;	
	tmp = adv7441a_hdmi_map_read(handle,ADV7441A_REG_CHANNEL_STATUS_DATA_2);
	handle->aud_st.channel_cnt = ((tmp>>4) & 0x1F);

	adv7441a_unlock(handle);
	return ERR_ADV7441A_SUCCESS;
}

/** Reads all video parameters and save it in the handle
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */
static int adv7441a_get_video_timing(t_adv7441a* handle) 
{
	uint32_t tmp;

	adv7441a_lock(handle);
	tmp = adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_05H);
	handle->vid_st.vsync_pol = ((tmp & ADV7441A_BIT_DVI_VSYNC_POLARITY) != 0);
	handle->vid_st.hsync_pol = ((tmp & ADV7441A_BIT_DVI_HSYNC_POLARITY) != 0);
	handle->vid_st.line_width = adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_07H);
	handle->vid_st.line_width = ((handle->vid_st.line_width&0x0F)<<8) | adv7441a_hdmi_map_read(handle, ADV7441A_REG_LINE_WIDTH); 
	handle->vid_st.field0 = adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD_0_HEIGHT_MSB) & 0x0F;
	handle->vid_st.field0 = ((handle->vid_st.field0)<<8) | adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD_0_HEIGHT); 	
	handle->vid_st.field1 = adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_0BH) & 0x0F;
	handle->vid_st.field1 = ((handle->vid_st.field1)<<8) | adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD_1_HEIGHT);
	handle->vid_st.pixel_clk = adv7441a_hdmi_map_read(handle,ADV7441A_REG_TMDSFREQ);
	adv7441a_unlock(handle);

	return ERR_ADV7441A_SUCCESS;
}

/** IRQ1 interrupt handler 
 *
 * @param handle pointer to the adv7441a handle
 * @param event_queue pointer to the event queue
 * @return error code
 */
int adv7441a_irq1_handler(t_adv7441a* handle, t_queue* event_queue) 
{
	uint8_t hdmi_status1, hdmi_status2, hdmi_status3;
	uint8_t hdmi_raw1, hdmi_raw2, hdmi_raw3;

	adv7441a_lock(handle);
	hdmi_status1 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_INT_STATUS_1);	
	hdmi_raw1 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_RAW_STATUS_1);
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_CLR_1, 0xFF);		/* clear hdmi int 1 */
	hdmi_status2 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_INT_STATUS_2);	
	hdmi_raw2 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_RAW_STATUS_2);
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_CLR_2, 0xFF);		/* clear hdmi int 2 */
	hdmi_status3 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_INT_STATUS_3);	
	hdmi_raw3 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_RAW_STATUS_3);
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_CLR_3, 0xFF);		/* clear hdmi int 3 */
	adv7441a_unlock(handle);


	if(hdmi_status3 & ADV7441A_BIT_TMDS_CLK_A_ST) {
		if(hdmi_raw3 & ADV7441A_BIT_TMDS_CLK_A_RAW) {
			/* Set TMDS equalizer settings */ 
			adv7441a_set_tmds_equa(handle);	
		} 
	}
	
	if(hdmi_status3 & ADV7441A_BIT_V_LOCKED_ST) { /* Video PLL changed */	
		if(hdmi_raw3 & ADV7441A_BIT_V_LOCKED_ST) {
			adv7441a_get_video_timing(handle);
			handle->status = handle->status | ADV7441A_STATUS_CONNECTION;
			REPORT(INFO, "HDMI link on port A established  (%dx%d @ %d MHz)\n",handle->vid_st.line_width, handle->vid_st.field0, handle->vid_st.pixel_clk);
			queue_put(event_queue, E_ADV7441A_NEW_RES);
		} else {
			handle->status = handle->status & ~ADV7441A_STATUS_CONNECTION;
			REPORT(INFO, "HDMI link on port A lost!\n");
			queue_put(event_queue, E_ADV7441A_NC);
		}	
	}

	if(hdmi_status1 & ADV7441A_BIT_AUDIO_INFO_ST) { /* Audio status changed */
		if(hdmi_raw1 & ADV7441A_BIT_AUDIO_INFO_RAW) {
			handle->status = handle->status | ADV7441A_STATUS_AUDIO;
			REPORT(INFO, "HDMI audio sampling rate has changed (audio unmuted)\n");
			adv7441a_audio_unmute(handle);
			adv7441a_get_audio_timing(handle);	
			queue_put(event_queue, E_ADV7441A_NEW_AUDIO);
		} else {	
			handle->status = handle->status & ~ADV7441A_STATUS_AUDIO;
			adv7441a_audio_mute(handle);
			REPORT(INFO, "HDMI audio has no data (audio muted)\n");
			queue_put(event_queue, E_ADV7441A_NO_AUDIO);
		}
	}

	if(hdmi_status2 & ADV7441A_BIT_HDMI_ENCRPT_CLR) { /* Encryption enabled/disabled */
		if(hdmi_raw2 & ADV7441A_BIT_HDMI_ENCRPT_RAW) {	
			handle->status = handle->status | ADV7441A_STATUS_ENCRYPTED;
			REPORT(INFO, "HDMI link is encrypted\n");	
			queue_put(event_queue, E_ADV7441A_HDCP);
		} else {
			handle->status = handle->status & ~ADV7441A_STATUS_ENCRYPTED;
			REPORT(INFO, "HDMI link is not encrypted\n");
			queue_put(event_queue, E_ADV7441A_NO_HDCP);
		}
	}
	return ERR_ADV7441A_SUCCESS;
}

/** IRQ2 interrupt handler 
 *
 * @param handle pointer to the adv7441a handle
 * @param event_queue pointer to the event queue
 * @return error code
 */
int adv7441a_irq2_handler(t_adv7441a* handle, t_queue* event_queue)
{
	uint8_t hdmi_status4;

	adv7441a_lock(handle);
	hdmi_status4 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_INT_STATUS_4);
//	hdmi_raw4 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_RAW_STATUS_4);
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_CLR_4, 0xFF); 		/* clear hdmi int 4 */
	adv7441a_unlock(handle);

	
	if(hdmi_status4 & ADV7441A_BIT_NEW_AVI_INFO_ST) {
		REPORT(INFO,"New AVI infoframe received\n");
	}
		
	if(hdmi_status4 & ADV7441A_BIT_NEW_AUDIO_INFO_ST) {
		REPORT(INFO,"New AUDIO infoframe received\n");
	}

	if(hdmi_status4 & ADV7441A_BIT_NEW_SPD_INFO_ST) {
		REPORT(INFO,"New SPD infoframe received\n");
	}

	if(hdmi_status4 & ADV7441A_BIT_NEW_MS_INFO_ST) {
		REPORT(INFO,"New MS infoframe received\n");
	}

	if(hdmi_status4 & ADV7441A_BIT_NEW_ACP_PCKT_ST) {
		REPORT(INFO,"New ACP infoframe received\n");
	}

	if(hdmi_status4 & ADV7441A_BIT_NEW_ISRC1_PCKT_ST) {
		REPORT(INFO,"New ISRC1 infoframe received\n");
	}

	if(hdmi_status4 & ADV7441A_BIT_NEW_ISRC2_PCKT_ST) {
		REPORT(INFO,"New ISRC2 infoframe received\n");
	}

	if(hdmi_status4 & ADV7441A_BIT_NEW_GAMUT_MDATA_ST) {
		REPORT(INFO,"New GAMUT infoframe received\n");
	}	
	return ERR_ADV7441A_SUCCESS;
}

/** adv7441a core driver handler (does nothing atm) 
 *
 * @param handle pointer to the adv7441a handle
 * @param event_queue pointer to the event queue
 * @return error code
 */
int adv7441a_handler(t_adv7441a* handle, t_queue* event_queue) 
{
	return ERR_ADV7441A_SUCCESS;
}

/** Read functions for /proc/ file. Functionparameters are defined by Linux.
 *
 */
int adv7441a_proc_video_in(char *buf, char **start, off_t offset, int count, int *eof, void *data) 
{
	int len = 0, cnt = 0;

	len += sprintf(buf+len,"[%03d] ADV7441A on                     : %d\n",cnt++,(adv7441a_handle->status & ADV7441A_STATUS_ACTIVE)!=0);
	len += sprintf(buf+len,"[%03d] ADV7441A HDMI link              : %d\n",cnt++,(adv7441a_handle->status & ADV7441A_STATUS_CONNECTION)!=0);
	len += sprintf(buf+len,"[%03d] ADV7441A HDMI encrypted         : %d\n",cnt++,(adv7441a_handle->status & ADV7441A_STATUS_ENCRYPTED)!=0);
	len += sprintf(buf+len,"[%03d] ADV7441A HDMI audio             : %d\n",cnt++,(adv7441a_handle->status & ADV7441A_STATUS_AUDIO)!=0);

	if(adv7441a_handle->status & ADV7441A_STATUS_CONNECTION) {
		len += sprintf(buf+len,"\n[%03d] ADV7441A HDMI width             : %d\n",cnt++,adv7441a_handle->vid_st.line_width);
		len += sprintf(buf+len,"[%03d] ADV7441A HDMI field0            : %d\n",cnt++,adv7441a_handle->vid_st.field0);
		len += sprintf(buf+len,"[%03d] ADV7441A HDMI field1            : %d\n",cnt++,adv7441a_handle->vid_st.field1);
		len += sprintf(buf+len,"[%03d] ADV7441A HDMI pixel clk         : %d\n",cnt++,adv7441a_handle->vid_st.pixel_clk);
	}

	if(adv7441a_handle->status & ADV7441A_STATUS_AUDIO) {
		len += sprintf(buf+len,"\n[%03d] ADV7441A HDMI audio fs          : %d\n",cnt++,adv7441a_handle->aud_st.fs);
		len += sprintf(buf+len,"[%03d] ADV7441A HDMI audio bit         : %d\n",cnt++,adv7441a_handle->aud_st.bit_width);
		len += sprintf(buf+len,"[%03d] ADV7441A HDMI audio channel cnt : %d\n",cnt++,adv7441a_handle->aud_st.channel_cnt);
		len += sprintf(buf+len,"[%03d] ADV7441A HDMI audio muted       : %d\n",cnt++,adv7441a_handle->aud_st.mute);
	}

	
	*eof = 1;
 	return len;
}


