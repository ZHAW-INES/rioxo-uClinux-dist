
#include "adv7441a_drv.h"

static t_adv7441a *adv7441a_handle; /* needed for adv7441a_proc_video_in() */
/* Register => value conversion arrays */
const __u32 adv7441a_audio_fs_conv[] = {44100, 0 ,48000, 32000, 0, 0, 0, 0, 88200, 0, 96000, 0, 176000, 0, 192000, 0};
const __u32 adv7441a_audio_bit_conv[2][8] = {{0, 16, 18, 0, 19, 20, 17, 0},{0, 20, 22, 0, 23, 24, 21, 0}};

static int adv7441a_drv_aud_pll_reset(t_adv7441a* handle)
{
	REPORT(INFO, "[HDMI IN] Audio PLL reset\n");
	adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_5A, ADV7441A_BIT_AUDIO_PLL_RESET); /* reset audio pll */

	return ERR_ADV7441A_SUCCESS;
}


int adv7441a_drv_dumb_reg(t_adv7441a* handle)
{
    int i;

    printk("\n\nkernel time;register map;map addr;offset;value\n");
    for(i=0; i<0xFF; i++) {
        printk(";User map;0x%02x;0x%02x;0x%02x\n",ADV7441A_I2C_USR_MAP,i,adv7441a_usr_map_read(handle, i));
    }

    for(i=0x40; i<0x9F; i++) {
        printk(";User map 1;0x%02x;0x%02x;0x%02x\n",ADV7441A_I2C_USR_MAP1,i,adv7441a_usr_map1_read(handle, i));
    }

    for(i=0xEA; i<0xF5; i++) {
      printk(";User map 2;0x%02x;0x%02x;0x%02x\n",ADV7441A_I2C_USR_MAP2,i,adv7441a_usr_map2_read(handle, i));
    }

    for(i=0x3C; i<0x9D; i++) {
      printk(";User map 3;0x%02x;0x%02x;0x%02x\n",ADV7441A_I2C_USR_MAP3,i,adv7441a_usr_map3_read(handle, i));
    }

    for(i=0x00; i<0xFF; i++) {
      printk(";HDMI Map;0x%02x;0x%02x;0x%02x\n",ADV7441A_I2C_HDMI_MAP,i,adv7441a_hdmi_map_read(handle, i));
    }
    return ERR_ADV7441A_SUCCESS;
}
/** Executes a software reset
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */
int adv7441a_sw_reset(t_adv7441a* handle) 
{
	handle->status = handle->status & ~ADV7441A_STATUS_ACTIVE;
	adv7441a_usr_map_write(handle, ADV7441A_REG_POWER_MANAGEMENT, ADV7441A_BIT_RES);
	mdelay(5);
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
		tmp = adv7441a_usr_map_read(handle, ADV7441A_REG_OUTPUT_CONTROL);
		adv7441a_usr_map_write(handle, ADV7441A_REG_OUTPUT_CONTROL, tmp & ~(ADV7441A_BIT_TOD)); /* output drivers active */
		tmp = adv7441a_usr_map_read(handle, ADV7441A_REG_EXTENDED_OUTPUT_CONTROL);
		adv7441a_usr_map_write(handle, ADV7441A_REG_EXTENDED_OUTPUT_CONTROL, tmp | ADV7441A_BIT_TIM_OE); /* timing signals enabled */ 
		adv7441a_usr_map_write(handle, ADV7441A_REG_POWER_MANAGEMENT, 0x00);	/* power down mode off */ 
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
		tmp = adv7441a_usr_map_read(handle, ADV7441A_REG_OUTPUT_CONTROL);
		adv7441a_usr_map_write(handle, ADV7441A_REG_OUTPUT_CONTROL, tmp | ADV7441A_BIT_TOD); /* output drivers tri-stated */
		tmp = adv7441a_usr_map_read(handle, ADV7441A_REG_EXTENDED_OUTPUT_CONTROL);
		adv7441a_usr_map_write(handle, ADV7441A_REG_EXTENDED_OUTPUT_CONTROL, tmp & (~ADV7441A_BIT_TIM_OE)); /* timing signals tri-stated */
		adv7441a_usr_map_write(handle, ADV7441A_REG_POWER_MANAGEMENT, ADV7441A_BIT_PWRDN | ADV7441A_BIT_PDBP);	/* power down mode on */ 
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
		tmp = adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_1DH);
		adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_1DH, tmp | ADV7441A_BIT_MUTE_AUDIO);	
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
		tmp = adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_1DH);
		adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_1DH, tmp & ~ADV7441A_BIT_MUTE_AUDIO);	
		handle->aud_st.mute = 0;
	}
	return ERR_ADV7441A_SUCCESS;
}

int adv7441a_enable_portA(t_adv7441a* handle)
{
    uint8_t tmp;

    tmp = adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_01H);
    adv7441a_ksv_map_write(handle, ADV7441A_REG_CTRL_BITS, ADV7441A_BIT_EDID_A_ENABLE); // EDID enabled
    adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_01H, tmp & ~(ADV7441A_BIT_CLOCK_TERM_PORT_A));

    vio_hdp_reset(handle->p_vio->p_vio);

    return ERR_ADV7441A_SUCCESS;
}

int adv7441a_disable_portA(t_adv7441a* handle)
{
    uint8_t tmp;

    vio_drv_set_hpd((handle->p_vio), false);

    tmp = adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_01H);
    adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_01H, tmp | ADV7441A_BIT_CLOCK_TERM_PORT_A);
    adv7441a_ksv_map_write(handle, ADV7441A_REG_CTRL_BITS, 0); // EDID disabled

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

	adv7441a_disable_portA(handle);

	for(i=0 ; i<0xFF ; i++) {	
		if(i!=0x7F) { // not checksum field
			adv7441a_edid_map_write(handle, i, edid[i]);
		}	
	}

	adv7441a_enable_portA(handle);

	return ERR_ADV7441A_SUCCESS;
}

/** Initialize ADV7441A core driver
 *
 * @param handle pointer to the adv7441a handle
 * @param p_i2c pointer to the i2c controller
 * @param edid pointer to the EDID file
 * @return error code
 */
int adv7441a_drv_init(t_adv7441a* handle, t_i2c* p_i2c, t_vio* p_vio, char* edid)
{
    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| ADV7441A-Driver: Initialize HDMI-RX              |");
    REPORT(INFO, "+--------------------------------------------------+");

    handle->p_i2c = p_i2c;
    handle->p_vio = p_vio;
    adv7441a_handle = handle;
    handle->status = 0; // clear status

    //adv7441a_disable_portA(handle);

    adv7441a_sw_reset(handle);

    /* ************************************** *
        User map (0x42)
     * ************************************** */

    /* Extended output control (0x04) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_EXTENDED_OUTPUT_CONTROL,
        ADV7441A_BIT_RANGE | /* reset value */
        ADV7441A_BIT_EN_SFL_PIN | /* reset value */
        ADV7441A_BIT_BL_C_VBI | /* reset value */
        ADV7441A_BIT_TIM_OE |
        ADV7441A_BIT_INT2_EN);

    /* VID_STD Register (0x06) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_VIDEO_STANDARD, 0x02);

    /* Automatic setting of the PLL (charge pump current & VCO range) (0x47) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_AUTOPLL_PARM_CTRL, ADV7441A_BIT_CALC_VCO_RANGE_EN | ADV7441A_BIT_CALC_PLL_QPUMP_EN);

    /* SET ADC[0-3]_SW to 0 => ADCs not connected */
    adv7441a_usr_map_write(handle, ADV7441A_REG_ADC_SWITCH_1, 0x00); /* (0xC3) ADC0 & ADC 1 */
    adv7441a_usr_map_write(handle, ADV7441A_REG_ADC_SWITCH_2, 0x00); /* (0xC4) ADC2 */
    adv7441a_usr_map_write(handle, ADV7441A_REG_AFE_CONTROL_1, 0x00); /* (0xF3) ADC3 */

    /* Power down ADC[0-3] (0x3A) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_ADC_CONTROL,
        ADV7441A_BIT_PDN_ADC0 |
        ADV7441A_BIT_PDN_ADC1 |
        ADV7441A_BIT_PDN_ADC2 |
        ADV7441A_BIT_PDN_ADC3);

    /* -------------------------------- ADI recommend write (user map) -------------------------------- */
    /* Set output format and enable output pins (0x03)
        0x00 : 10 bit 4:2:2
        0x04 : 20 bit 4:2:2
        0x08 : 16 bit 4:2:2
        0x0A : 8  bit 4:2:2 ITU-R BT.656
        0x10 : 30 bit 4:4:4
        0x14 : 24 bit 4:4:4 */
    adv7441a_usr_map_write(handle, ADV7441A_REG_OUTPUT_CONTROL, 0x10);

    /* PRIM_MODE Register (0x05)
        0x04 : HDMI SD
        0x05 : HDMI ED / HD
        0x06 : HDMI VGA to SXGA */
    adv7441a_usr_map_write(handle, ADV7441A_REG_PRIMARY_MODE, 0x06);

    /* Set crystal mode and LLC output active (0x1D) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_VERTICAL_SCALE_VALUE_1, ADV7441A_BIT_EN28XTAL);

    /* Sets polarity (LLC1, FIELD, VS, HS) (0x37) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_POLARITY, 0x01);

    /* Set CP output selection register (0x6B) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_CP_OUTPUT_SELECTION, 0x02 | ADV7441A_BIT_DE_OUT_SEL |
        ADV7441A_BIT_VS_OUT_SEL |
        ADV7441A_BIT_HS_OUT_SEL);

    /* Set CSC Coeff to automatic mode (CSC_23) (0x68) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_CSC_23, 0xF0 | ADV7441A_BIT_RGB_OUT);

    /* NEW! Enable HDMI and Analog in (0xBA) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_DPP_CP_105, 0xA0);

    /* Set digital fine clamp setting for HDMI Mode (0xC8) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_CP_CLAMP_POS_HS_CTRL_4, 0x08);

    /* Set CP AV controll register (0x7B) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_CP_AV_CONTROL, 0x04 | ADV7441A_BIT_AV_BLANK_EN | ADV7441A_BIT_DE_WITH_AVCODE);

    /* 0x7C - 0x7E => reset value */

    /* Drive strength for HS, VS, f and clock set to maximum (0xF4) */
    adv7441a_usr_map_write(handle, ADV7441A_REG_DRIVE_STRENGTH, 0x3F);

    /* ************************************** *
        User map 2 (0x62)
     * ************************************** */

    /* Set equalizer settings */
    adv7441a_usr_map2_write(handle, 0xF0, 0x10);
    adv7441a_usr_map2_write(handle, 0xF1, 0x0F);
    adv7441a_usr_map2_write(handle, 0xF4, 0x20);


    /* ************************************** *
        HDMI map
     * ************************************** */

    /* Set Mute mask (disable all) */
    adv7441a_hdmi_map_write(handle,ADV7441A_REG_MUTE_MASK_19_16, 0x00); /* (0x14) */
    adv7441a_hdmi_map_write(handle,ADV7441A_REG_MUTE_MASK_15_8, 0x00); /* (0x15) */
    adv7441a_hdmi_map_write(handle,ADV7441A_REG_MUTE_MASK_7_0, 0x00); /* (0x16) */

    /* HDMI Port select => Port A (0x00) */
    adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_00H, 0x00);

    /* Set CTS change Threshold (0x10) */
    adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_10H, 0x1F);

    /* Disable audio pll, MCLK fs, MCLK N (0x1C) */
    //adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_1CH, 0x20); /* PLL enable write 0x60, disable write 0x20 */
    adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_1CH, 0x20); /* PLL enable write 0x60, disable write 0x20 */


    /* Audio Mute options and conversion mode (0x1D) */
    adv7441a_hdmi_map_write(handle, ADV7441A_REG_REGISTER_1DH, ADV7441A_BIT_UP_CONVERSION_MODE);

    /* Reset audio PLL */
    adv7441a_drv_aud_pll_reset(handle);

    /* ************************************** *
        Repeater map 1 (0x66)
     * ************************************** */

    /* ADV7441A has been implemented in a HDMI receiver mode
     * Disabling Enhanced Link Verification feature (HDCP) */
    adv7441a_ksv_map_write(handle, ADV7441A_REG_BCAPS, 0x80);

    /* ************************************** *
        User map 1 (0x46)
     * ************************************** */

    /* Int 1 & int 2 configuration */
    adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_CONFIGURATION_0, 0xD1); /* active until cleared, active low */
    adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_CONFIGURATION_1, 0xC1);

    adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_MASKB_1,0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_MASKB_2,0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_MASKB_3,0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_MASKB_4,0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_1, 0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_2, ADV7441A_BIT_HDMI_ENCRPT_MB1 | ADV7441A_BIT_AUDIO_C_PCKT_MB1 | ADV7441A_BIT_AUDIO_PLL_LCK_MB1);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_3, ADV7441A_BIT_VIDEO_PLL_LCK_MB1 | ADV7441A_BIT_V_LOCKED_MB1 | ADV7441A_BIT_TMDS_CLK_A_MB1);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_4, 0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_5, ADV7441A_BIT_CTS_PASS_THRS_M1);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_MASKB_6, ADV7441A_BIT_NEW_SAMP_RT_MB1);

    adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_2_MASKB_1,0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_2_MASKB_2,0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_2_MASKB_3,0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_INTERRUPT_2_MASKB_4,0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_1, 0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_2, 0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_3, 0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_4, 0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_5, 0x00);
    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT2_MASKB_6, 0x00);

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
	uint8_t tmds_freq =  adv7441a_hdmi_map_read(handle,ADV7441A_REG_TMDSFREQ);

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
	REPORT(INFO, "HDMI TMDS equalizer settings set");
	return ERR_ADV7441A_SUCCESS;
}

/** Reads all audio parameters and save it in the handle 
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */
//static
int adv7441a_get_audio_timing(t_adv7441a* handle)
{ 
	uint8_t tmp;
	uint8_t tmp_bit_width, tmp_mute, tmp_channel_cnt;
	uint32_t tmp_fs;

	tmp = adv7441a_hdmi_map_read(handle,ADV7441A_REG_CHANNEL_STATUS_DATA_3);
	tmp_fs = adv7441a_audio_fs_conv[tmp & 0xF];

	tmp = adv7441a_hdmi_map_read(handle,ADV7441A_REG_CHANNEL_STATUS_DATA_4);
	tmp_bit_width = adv7441a_audio_bit_conv[tmp&0x1][((tmp & 0x1E)>>1)];

	tmp = adv7441a_usr_map1_read(handle,ADV7441A_REG_HDMI_RAW_STATUS_3);
	tmp_mute = (tmp & ADV7441A_BIT_INTERNAL_MUTE_RAW) >> 5;

	tmp = adv7441a_hdmi_map_read(handle,ADV7441A_REG_CHANNEL_STATUS_DATA_2);
	tmp_channel_cnt = ((tmp>>4) & 0x0F) + 1;

	// default value for audio word size
	if(tmp_bit_width == 0) {
	    tmp_bit_width = 16;
	}

    if((tmp_fs == 0) || (tmp_bit_width < 8) || (tmp_bit_width > 32)) { // paramter not in range
        return ERR_ADV7441A_AUD_PARAM_NOT_VALID;
    } else if((tmp_bit_width == handle->aud_st.bit_width) &&    // parameter not changed
              (tmp_mute == handle->aud_st.mute) &&
              (tmp_channel_cnt == handle->aud_st.channel_cnt) &&
              (tmp_fs = handle->aud_st.fs)) {
        return ERR_ADV7441A_AUD_PARAM_NOT_CHANGED;
    } else { // capture parameter
        handle->aud_st.bit_width = tmp_bit_width;
        handle->aud_st.mute = tmp_mute;
        handle->aud_st.channel_cnt = tmp_channel_cnt;
        handle->aud_st.fs = tmp_fs;
	    return ERR_ADV7441A_SUCCESS;
    }

}

/** Reads all video parameters and save it in the handle
 *
 * @param handle pointer to the adv7441a handle
 * @return error code
 */
int adv7441a_get_video_timing(t_adv7441a* handle) 
{
    handle->vid_st.interlaced =  (adv7441a_usr_map_read(handle, ADV7441A_REG_RB_STANDARD_IDENT_1) & ADV7441A_BIT_STDI_INTLCD)  && ADV7441A_BIT_STDI_INTLCD;  // 1 = interlaced timing detected
    handle->vid_st.vsync_pol  = !((adv7441a_usr_map_read(handle, ADV7441A_REG_CP_HVF_CONTROL_1) & ADV7441A_BIT_PIN_INV_VS)     && ADV7441A_BIT_PIN_INV_VS);  // 1 = high active
    handle->vid_st.hsync_pol  = !((adv7441a_usr_map_read(handle, ADV7441A_REG_CP_HVF_CONTROL_1) & ADV7441A_BIT_PIN_INV_HS)     && ADV7441A_BIT_PIN_INV_HS);  // 1 = high active
    handle->vid_st.field_pol  =  (adv7441a_usr_map_read(handle, ADV7441A_REG_CP_HVF_CONTROL_1) & ADV7441A_BIT_PIN_INV_F)       && ADV7441A_BIT_PIN_INV_F;    // 1 = high is odd field

    // Horizontal Measurements
    if (adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_07H) & ADV7441A_BIT_DE_REGEN_FILTER_LOCKED) // Horizontal measurements are valid
    {
        handle->vid_st.h_total_line_width      = (((adv7441a_hdmi_map_read(handle, ADV7441A_REG_TOTAL_LINE_WIDTH_MSB)  & 0x0F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_TOTAL_LINE_WIDTH));
        handle->vid_st.h_line_width            = (((adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_07H)          & 0x0F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_LINE_WIDTH));
        handle->vid_st.h_front_porch_width     = (((adv7441a_hdmi_map_read(handle, ADV7441A_REG_HSYNC_FRONT_PORCH_MSB) & 0x03) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_HSYNC_FRONT_PORCH));
        handle->vid_st.h_sync_width            = (((adv7441a_hdmi_map_read(handle, ADV7441A_REG_HSYNC_PULSE_WIDTH_MSB) & 0x03) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_HSYNC_PULSE_WIDTH));
        handle->vid_st.h_back_porch_width      = (((adv7441a_hdmi_map_read(handle, ADV7441A_REG_HSYNC_BACK_PORCH_MSB)  & 0x03) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_HSYNC_BACK_PORCH));
    }
    else
    {
        return ERR_ADV7441A_VID_PARAM_NOT_VALID;
    }    

    // Vertical Measurements (all units are number of lines)
    if (adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_07H) & ADV7441A_BIT_VERT_FILTER_LOCKED) // Vertical measurements are valid
    {
        // Field 0
        handle->vid_st.f0_total_height         = (((adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD0_TOTAL_HEIGHT_MSB)     & 0x1F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD0_TOTAL_HEIGHT));
        handle->vid_st.f0_height               = (((adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD_0_HEIGHT_MSB)          & 0x0F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD_0_HEIGHT)); //>>1;
        handle->vid_st.f0_front_porch_width    = ((((adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD0_VS_FRONT_PORCH_MSB)  & 0x1F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD0_VS_FRONT_PORCH))>>1);
        handle->vid_st.f0_vs_pulse_width       = ((((adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD0_VS_PULSE_WIDTH_MSB)  & 0x1F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD0_VS_PULSE_WIDTH))>>1);
        handle->vid_st.f0_back_porch_width     = ((((adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD0_VS_BACK_PORCH_MSB)   & 0x1F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD0_VS_BACK_PORCH))>>1);

        // Field 1
        if (handle->vid_st.interlaced) {
            handle->vid_st.f1_total_height         = (((adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD1_TOTAL_HEIGHT_MSB)    & 0x0F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD1_TOTAL_HEIGHT));
            handle->vid_st.f1_height               = (((adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_0BH)               & 0x0F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD_1_HEIGHT)); //>>1;
            handle->vid_st.f1_front_porch_width    = ((((adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD1_VS_FRONT_PORCH_MSB) & 0x1F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD1_VS_FRONT_PORCH))>>1);
            handle->vid_st.f1_vs_pulse_width       = ((((adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD1_VS_PULSE_WIDTH_MSB) & 0x1F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD1_VS_PULSE_WIDTH))>>1);
            handle->vid_st.f1_back_porch_width     = ((((adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD1_VS_BACK_PORCH_MSB)  & 0x1F) << 8) + adv7441a_hdmi_map_read(handle, ADV7441A_REG_FIELD1_VS_BACK_PORCH))>>1);
        }
        else {
            handle->vid_st.f1_total_height         = 0;
            handle->vid_st.f1_height               = 0;
            handle->vid_st.f1_front_porch_width    = 0;
            handle->vid_st.f1_vs_pulse_width       = 0;
            handle->vid_st.f1_back_porch_width     = 0;
        }

    }
    else
    {
        return ERR_ADV7441A_VID_PARAM_NOT_VALID;
    }

	// Change vsync pin to output field signal if interlaced timing is measured 
    if (handle->vid_st.interlaced) {
	    adv7441a_usr_map_write(handle, ADV7441A_REG_CP_OUTPUT_SELECTION, 0x02 | ADV7441A_BIT_DE_OUT_SEL | ADV7441A_BIT_HS_OUT_SEL);
        adv7441a_usr_map_write(handle, ADV7441A_REG_CP_HVF_CONTROL_1, ADV7441A_BIT_PIN_INV_HS | ADV7441A_BIT_PIN_INV_VS | ADV7441A_BIT_PIN_INV_F);

    }
    else {
        adv7441a_usr_map_write(handle, ADV7441A_REG_CP_OUTPUT_SELECTION, 0x02 | ADV7441A_BIT_DE_OUT_SEL | ADV7441A_BIT_VS_OUT_SEL | ADV7441A_BIT_HS_OUT_SEL);
        adv7441a_usr_map_write(handle, ADV7441A_REG_CP_HVF_CONTROL_1, ADV7441A_BIT_PIN_INV_HS | ADV7441A_BIT_PIN_INV_VS);

    }

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
	uint8_t hdmi_status2, hdmi_status3, hdmi_status5, hdmi_status6;
	uint8_t hdmi_raw2, hdmi_raw3;
    uint8_t int2_clr = 0, int3_clr = 0, int5_clr = 0, int6_clr = 0, reg_04 = 0;

	hdmi_status2 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_INT_STATUS_2);	
	hdmi_raw2 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_RAW_STATUS_2);
	hdmi_status3 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_INT_STATUS_3);	
	hdmi_raw3 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_RAW_STATUS_3);
	hdmi_status5 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_INT_STATUS_5);
	hdmi_status6 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_INT_STATUS_6);

	/* TMDS clock changed */
    if(hdmi_status3 & ADV7441A_BIT_TMDS_CLK_A_ST) {
        int3_clr |= ADV7441A_BIT_TMDS_CLK_A_CLR;
		if(hdmi_raw3 & ADV7441A_BIT_TMDS_CLK_A_RAW) {
			/* Set TMDS equalizer settings */
            REPORT(INFO, "[HDMI IN] TMDS clock active on Port A\n");
            queue_put(event_queue, E_ADV7441A_CONNECT);
		} else {
            REPORT(INFO, "[HDMI IN] no TMDS clock active\n");
        }

	}

    /* Video PLL changed */
	if((hdmi_status3 & ADV7441A_BIT_VIDEO_PLL_LCK_ST) != 0) {
        int3_clr |= ADV7441A_BIT_VIDEO_PLL_LCK_CLR;
		if((hdmi_raw3 & ADV7441A_BIT_VIDEO_PLL_LCK_RAW) != 0){
			//REPORT(INFO, "[HDMI IN] Video PLL locked\n");
		} else {
    		//REPORT(INFO, "[HDMI IN] Video PLL unlocked\n");
		}	
	}

	/* Audio PLL changed */
    if((hdmi_status2 & ADV7441A_BIT_AUDIO_PLL_LCK_ST) != 0) {
        int2_clr |= ADV7441A_BIT_AUDIO_PLL_LCK_CLR;
        if((hdmi_raw2 & ADV7441A_BIT_AUDIO_PLL_LCK_RAW) != 0) {
            if((handle->status & ADV7441A_STATUS_AUDIO) == 0) {
                handle->status = handle->status | ADV7441A_STATUS_AUDIO;
                adv7441a_audio_unmute(handle);
                queue_put(event_queue, E_ADV7441A_NEW_AUDIO);
            }
			REPORT(INFO, "[HDMI IN] Audio PLL locked\n");
        } else {
    		REPORT(INFO, "[HDMI IN] Audio PLL unlocked\n");
            handle->status = handle->status & ~ADV7441A_STATUS_AUDIO;
            adv7441a_audio_mute(handle);
            REPORT(INFO, "[HDMI IN] audio has no data (audio muted)\n");
            queue_put(event_queue, E_ADV7441A_NO_AUDIO);
        }
    }

    /* IRQ only valid under following conditions 
     *  TMDS clock active
     *  Video PLL locked
     */
    if(((hdmi_raw3 & ADV7441A_BIT_TMDS_CLK_A_RAW) != 0) && ((hdmi_raw3 & ADV7441A_BIT_VIDEO_PLL_LCK_RAW) != 0)) {
        /* Encryption detection */
        if(hdmi_status2 & ADV7441A_BIT_HDMI_ENCRPT_ST) {
            int2_clr |= ADV7441A_BIT_HDMI_ENCRPT_CLR;
            if(hdmi_raw2 & ADV7441A_BIT_HDMI_ENCRPT_RAW) {
                handle->status = handle->status | ADV7441A_STATUS_ENCRYPTED;
                REPORT(INFO, "[HDMI IN] link is encrypted\n");
                queue_put(event_queue, E_ADV7441A_HDCP);

                reg_04 = adv7441a_hdmi_map_read(handle, ADV7441A_REG_REGISTER_04H);
                if(((reg_04 & ADV7441A_BIT_HDCP_KEY_ERROR) > 0) || ((reg_04 & ADV7441A_BIT_HDCP_KEYS_READ) == 0)) {
                    REPORT(INFO, "[HDMI IN] HDCP key error\n");
                    queue_put(event_queue, E_ADV7441A_HDCP_KEY_ERR);
                }
            } else {
                handle->status = handle->status & ~ADV7441A_STATUS_ENCRYPTED;
                REPORT(INFO, "[HDMI IN] link is not encrypted\n");
                queue_put(event_queue, E_ADV7441A_NO_HDCP);
            }
        }

        /* vertical synch filter status changed */
        if((hdmi_status3 & ADV7441A_BIT_V_LOCKED_ST) != 0) { 
            int3_clr |= ADV7441A_BIT_V_LOCKED_CLR;
            if((hdmi_raw3 & ADV7441A_BIT_V_LOCKED_RAW) != 0) {

                if(hdmi_raw3 & ADV7441A_BIT_HDMI_MODE_RAW) {
                    REPORT(INFO, "[HDMI IN] Chip in HDMI mode\n");
                } else {
                    REPORT(INFO, "[HDMI IN] Chip in DVI mode\n");
                }

			    handle->status = handle->status | ADV7441A_STATUS_CONNECTION;
                REPORT(INFO, "[HDMI IN] vertical synch filter has locked\n");
			    REPORT(INFO, "[HDMI IN] link on port A established\n");

			    queue_put(event_queue, E_ADV7441A_NEW_RES);
            } else {
                REPORT(INFO, "[HDMI IN] vertical synch filter not locked\n");
            }
        }

        /* Audio Clock Regeneration (ACR) packet received */
        if(hdmi_status2 & ADV7441A_BIT_AUDIO_C_PCKT_ST) {
            REPORT(INFO, "[HDMI IN] Audio clock regeneration (ACR) packet received\n");
        	adv7441a_drv_aud_pll_reset(handle);
         	int2_clr |= ADV7441A_BIT_AUDIO_C_PCKT_CLR;
        }

        /* CTS changes by more than threshold */
        if(hdmi_status5 & ADV7441A_BIT_CTS_PASS_THRSH_ST) {
            int5_clr |= ADV7441A_BIT_CTS_PASS_THRSH_CL;
            REPORT(INFO, "[HDMI IN] CTS changes by more than threshold\n");
            adv7441a_drv_aud_pll_reset(handle);
        }

        if(hdmi_status6 & ADV7441A_BIT_NEW_SAMP_RT_ST) {
            int6_clr |= ADV7441A_BIT_NEW_SAMP_RT_CL;

            /* if new audio parameters or audio not started */
            if((adv7441a_get_audio_timing(handle) == ERR_ADV7441A_SUCCESS) ||
               ((handle->status & ADV7441A_STATUS_AUDIO) == 0)){
                REPORT(INFO, "[HDMI IN] audio sampling rate has changed (audio unmuted)\n");
                REPORT(INFO, "[HDMI IN] Audio fs = %d Hz", handle->aud_st.fs);
                REPORT(INFO, "[HDMI IN] Audio width = %d Bit", handle->aud_st.bit_width);
                REPORT(INFO, "[HDMI IN] Audio count = %d", handle->aud_st.channel_cnt);
                REPORT(INFO, "[HDMI IN] Audio mute = %d\n", handle->aud_st.mute);

                if((handle->status & ADV7441A_STATUS_AUDIO) != 0) {
                    queue_put(event_queue, E_ADV7441A_NO_AUDIO);
                }

                handle->status = handle->status | ADV7441A_STATUS_AUDIO;
                adv7441a_audio_unmute(handle);
                queue_put(event_queue, E_ADV7441A_NEW_AUDIO);
            }
        }

    } else {
        if((handle->status & ADV7441A_STATUS_CONNECTION) != 0) {
                    handle->status = handle->status & ~ADV7441A_STATUS_CONNECTION;
                    REPORT(INFO, "[HDMI IN] link on port A lost!\n");
                    queue_put(event_queue, E_ADV7441A_NC);
                    //queue_put(event_queue, E_ADV7441A_NO_AUDIO);
        }

        // Clear invalide interrupts if no TMDS clock is detected
        int2_clr |= ADV7441A_BIT_AUDIO_C_PCKT_CLR | ADV7441A_BIT_HDMI_ENCRPT_CLR;
        int3_clr |= ADV7441A_BIT_V_LOCKED_CLR;
        int5_clr |= ADV7441A_BIT_CTS_PASS_THRSH_CL;
        int6_clr |= ADV7441A_BIT_NEW_SAMP_RT_CL;
    }


    /* Clear interrupts */
    if(int2_clr)    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_CLR_2, int2_clr);
    if(int3_clr)    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_CLR_3, int3_clr);
    if(int5_clr)    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_CLR_5, int5_clr);
    if(int6_clr)    adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_CLR_6, int6_clr);

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
	uint8_t hdmi_status4, hdmi_raw3;

	hdmi_raw3 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_RAW_STATUS_3);
	hdmi_status4 = adv7441a_usr_map1_read(handle, ADV7441A_REG_HDMI_INT_STATUS_4);
	adv7441a_usr_map1_write(handle, ADV7441A_REG_HDMI_INT_CLR_4, 0xFF); 		/* clear hdmi int 4 */

    /* IRQ only valid under following conditions 
     *  TMDS clock active
     *  Video PLL locked
     */  
    if(((hdmi_raw3 & ADV7441A_BIT_TMDS_CLK_A_RAW) != 0) && ((hdmi_raw3 & ADV7441A_BIT_VIDEO_PLL_LCK_RAW) != 0)) {

    	if(hdmi_status4 & ADV7441A_BIT_NEW_AVI_INFO_ST) {
    		REPORT(INFO,"[HDMI IN] New AVI infoframe received\n");
    	}
		
    	if(hdmi_status4 & ADV7441A_BIT_NEW_AUDIO_INFO_ST) {
    		REPORT(INFO,"[HDMI IN] New AUDIO infoframe received\n");
    	}

	    if(hdmi_status4 & ADV7441A_BIT_NEW_SPD_INFO_ST) {
    		REPORT(INFO,"[HDMI IN] New SPD infoframe received\n");
    	}

    	if(hdmi_status4 & ADV7441A_BIT_NEW_MS_INFO_ST) {
    		REPORT(INFO,"[HDMI IN] New MS infoframe received\n");
    	}

	    if(hdmi_status4 & ADV7441A_BIT_NEW_ACP_PCKT_ST) {
    		REPORT(INFO,"[HDMI IN] New ACP infoframe received\n");
    	}

	    if(hdmi_status4 & ADV7441A_BIT_NEW_ISRC1_PCKT_ST) {
    		REPORT(INFO,"[HDMI IN] New ISRC1 infoframe received\n");
    	}

	    if(hdmi_status4 & ADV7441A_BIT_NEW_ISRC2_PCKT_ST) {
    		REPORT(INFO,"[HDMI IN] New ISRC2 infoframe received\n");
    	}

	    if(hdmi_status4 & ADV7441A_BIT_NEW_GAMUT_MDATA_ST) {
    		REPORT(INFO,"[HDMI IN] New GAMUT infoframe received\n");
    	}
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
		len += sprintf(buf+len,"\n[%03d] ADV7441A HDMI width             : %d\n",cnt++,adv7441a_handle->vid_st.h_line_width);
		len += sprintf(buf+len,"[%03d] ADV7441A HDMI field0            : %d\n",cnt++,adv7441a_handle->vid_st.f0_height);
		len += sprintf(buf+len,"[%03d] ADV7441A HDMI field1            : %d\n",cnt++,adv7441a_handle->vid_st.f1_height);
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
