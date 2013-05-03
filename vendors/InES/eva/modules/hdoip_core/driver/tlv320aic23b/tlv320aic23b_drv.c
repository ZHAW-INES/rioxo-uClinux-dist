#include "tlv320aic23b.h"
#include "tlv320aic23b_drv.h"

static int write_reg(t_tlv320aic23b *codec, uint32_t reg, uint16_t value) {
	uint8_t data[2];

	codec->registers[reg] = value & 0x01ff;
	codec->registers[reg] |= (reg << 9) & 0xfe00;
	data[0] = (uint8_t) (codec->registers[reg] & 0x00FF);
	data[1] = (uint8_t) ((codec->registers[reg] >> 8) & 0x00FF);

	i2c_drv_wreg8(codec->i2c_master, codec->devaddr, data[1], data[0]);
	return (int) i2c_drv_get_error(codec->i2c_master);
}

static inline uint16_t read_reg(t_tlv320aic23b *codec, uint32_t reg) {
	return codec->registers[reg] & 0x01ff;
}

static inline int write_reg_mask(t_tlv320aic23b *codec, uint32_t reg, uint16_t mask, uint16_t new_value) {
	uint16_t val = read_reg(codec, reg);
	val &= ~mask;	/* zero all bits likely to change */
	new_value &= mask;
	val |= new_value;
	return write_reg(codec, reg, val);
}

/**
 * write the codec's reset register
 *
 * @param codec 	device instance 
 * */
static int aic23b_reset(t_tlv320aic23b *codec) {
	int i=0;
	int err;
	const uint16_t	REG_RST_VALUES[AIC23B_REG_CNT] = {
		0x0097,
		0x0097,
		0x00F9,
		0x00F9,
		0x000A,
		0x0008,
		0x0007,
		0x0001,
		0x0020,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000
	};

	err = write_reg(codec, TI_RESET, 0);
	if (err != SUCCESS)
		return err;
		
	for (i=0; i < AIC23B_REG_CNT; i++) {
		codec->registers[i] = REG_RST_VALUES[i];
	}
	
	return SUCCESS;
}

/**
 * init device structure and codec settings
 *
 * @param codec			device instance
 * @param i2c_master	instance of i2c master to access cocecs control regs
 * @param devaddr		codec's i2c address
 * @param master		true to run the data interface (I2S) in master mode,
 * 						false for slave mode
 * */
int aic23b_init(t_tlv320aic23b *codec, t_i2c *i2c_master, uint8_t devaddr, bool master) {
	int result;

    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| TLV3230AIC23B-Driver: Initialize Audio-Board     |");
    REPORT(INFO, "+--------------------------------------------------+");
		
	/* init device struct */
	codec->i2c_master = i2c_master;
	codec->devaddr =  devaddr;
	codec->available = false;
	codec->master = master;
	memset(&codec->registers, 0, sizeof(codec->registers));

	/* init state of codec */
	result = aic23b_reset(codec);
	if (result != SUCCESS)
		return result;
	result = aic23b_set_mute(codec, AIC23B_MUTELINE_LINE_IN_L, true);
	if (result != SUCCESS)
		return result;
	result = aic23b_set_mute(codec, AIC23B_MUTELINE_LINE_IN_R, true);
	if (result != SUCCESS)
		return result;
	result = aic23b_set_mute(codec, AIC23B_MUTELINE_MIC, true);
	if (result != SUCCESS)
		return result;
	result = aic23b_set_mute(codec, AIC23B_MUTELINE_DAC, true);
	if (result != SUCCESS)
		return result;
	result = aic23b_set_loop(codec, false);
	if (result != SUCCESS)
		return result;
	result = aic23b_set_if_format(codec, master, false, false, AIC23B_I2S_INPUT_WITH_32BIT, AIC23B_I2S_FORMAT_I2S);
	if (result != SUCCESS)
		return result;
	result = aic23b_set_adc_hp(codec, true);
	if (result != SUCCESS)
		return result;
	result = aic23b_set_power_down(codec, false, false);
	if (result != SUCCESS)
		return result;
    result = aic23b_set_power_down(codec, false, true);
	if (result != SUCCESS)
		return result;

	codec->available = true;
	return SUCCESS;
}

/**
 * set gain of selected amplifier stage
 *
 * @param codec	device instance
 * @param amp	on which amplifier to set the gain to
 * @param gain	line input gain in range 0 to 31 -> -34.5dB..+12dB in 1.5dB steps
 *                 0  -> -34.5 dB
 *                 23 ->   0 dB
 *                 31 -> +12 dB
 *              headphone driver gain in range -73 to 6 -> -73dB..+6dB in 1dB steps (-73 = mute)
 *              microphone input gain is 0 or 20 -> 0dB or 20dB (boost)
 * */
int aic23b_set_gain(t_tlv320aic23b *codec, enum AIC23B_AMPLIFIER amp, int gain) {
	uint16_t reg;
	
	/* check range */
	if ((amp == AIC23B_AMPLIFIER_LINE_IN_L) || (amp == AIC23B_AMPLIFIER_LINE_IN_R)) {
		if ((gain < 0) || (gain > 31)) {
			REPORT(ERROR, "tried to set gain out of range\n");
			return ERR_GENERAL;
		}
		reg = (uint16_t)gain;
		reg &= 0x001F;
	}
	if ((amp == AIC23B_AMPLIFIER_HEADPH_L) || (amp == AIC23B_AMPLIFIER_HEADPH_R)) {
		if ((gain < -73) || (gain > 6)) {
			REPORT(ERROR, "tried to set gain out of range\n");
			return ERR_GENERAL;
		}
		reg = (uint16_t) (gain + 121);
		reg &= 0x007F;

		reg |= TI_LEFT_HP_VC_LZC;	/* enable volume change on zero cross */
	}
	if (amp == AIC23B_AMPLIFIER_MIC) {
		if ((gain != 0) && (gain != 20)) {
			REPORT(ERROR, "tried to set gain out of range\n");
			return ERR_GENERAL;
		}
		if (gain == 0)
			reg = 0;
		else
			reg = TI_ANALOG_AP_MICB;
	}
	
	/* set registers */
	switch (amp) {
		case AIC23B_AMPLIFIER_LINE_IN_L:
			return write_reg_mask(codec, TI_LEFT_LINEIN_VC, TI_LEFT_LINEIN_VC_LIV_M, reg);
		case AIC23B_AMPLIFIER_LINE_IN_R:
			return write_reg_mask(codec, TI_RIGHT_LINEIN_VC, TI_RIGHT_LINEIN_VC_RIV_M, reg);
		case AIC23B_AMPLIFIER_MIC:
			return write_reg_mask(codec, TI_ANALOG_AP, TI_ANALOG_AP_MICB, reg);
		case AIC23B_AMPLIFIER_HEADPH_L:
			return write_reg_mask(codec, TI_LEFT_HP_VC, TI_LEFT_HP_VC_LHV_M | TI_LEFT_HP_VC_LZC, reg);
		case AIC23B_AMPLIFIER_HEADPH_R:
			return write_reg_mask(codec, TI_RIGHT_HP_VC, TI_RIGHT_HP_VC_RHV_M | TI_RIGHT_HP_VC_RZC, reg);
		default:
			REPORT(ERROR, "passed wrong amplifier type\n");
			return ERR_GENERAL;
	}
}

/**
 * mute a channel
 *
 * @param codec		device instance
 * @param ml		the channel (source) to mute/unmute
 * @param mute		true to mute, false to unmute
 * */
int aic23b_set_mute(t_tlv320aic23b *codec, enum AIC23B_MUTELINE ml, bool mute) {
	uint16_t reg;
	
	switch (ml) {
		case AIC23B_MUTELINE_LINE_IN_L:
			reg = mute ? TI_LEFT_LINEIN_VC_LIM : 0;
			write_reg_mask(codec, TI_LEFT_LINEIN_VC, TI_LEFT_LINEIN_VC_LIM, reg);
		case AIC23B_MUTELINE_LINE_IN_R:
			reg = mute ? TI_RIGHT_LINEIN_VC_RIM : 0;
			return write_reg_mask(codec, TI_RIGHT_LINEIN_VC, TI_RIGHT_LINEIN_VC_RIM, reg);
		case AIC23B_MUTELINE_MIC:
			reg = mute ?  TI_ANALOG_AP_MICM : 0;
			return write_reg_mask(codec, TI_ANALOG_AP, TI_ANALOG_AP_MICM, reg);
		case AIC23B_MUTELINE_DAC:
			reg = mute ?  TI_DIGITAL_AP_DACM : 0;
			return write_reg_mask(codec, TI_DIGITAL_AP, TI_DIGITAL_AP_DACM, reg);
		default:
			REPORT(ERROR, "passed wrong muteline type\n");
			return ERR_GENERAL;
	}
}

/**
 * enable/disable loopback of input signal to output
 *
 * @param codec			device instance
 * @param loop			true to loop from input, false to use DAC
 * */
int aic23b_set_loop(t_tlv320aic23b *codec, bool loop) {
	uint16_t reg, mask;
	
	if (loop) {
		/* enable bypass and disable dac */
		reg = TI_ANALOG_AP_BYP;
		mask = TI_ANALOG_AP_BYP | TI_ANALOG_AP_DAC;
	}
	else {
		/* disable bypass and enable dac */
		reg = TI_ANALOG_AP_DAC;
		mask = TI_ANALOG_AP_BYP | TI_ANALOG_AP_DAC;
	}
	return write_reg_mask(codec, TI_ANALOG_AP, mask, reg);
}

/**
 * Add microphone signal to lineout and headphone out as a sidetone.
 *
 * This is useful for telephony and headset applications.
 *
 * @param codec			device instance
 * @param sidetone		true to enable sidetone insertion, false to disable
 * @param attenuation	Attenuation of mic signal. Be careful to not exceed
 * 						maximum signal level when added with DAC or LINE IN.
 * 						Allowed values are 0, 6, 9, 12, 18 dB
 * */
int aic23b_set_sidetone(t_tlv320aic23b *codec, bool sidetone, int attenuation) {
	uint16_t reg, mask;

	if (sidetone) {
		reg = TI_ANALOG_AP_STE;	/* enable */
		mask = TI_ANALOG_AP_STE | TI_ANALOG_AP_STA_M;
		
		switch (attenuation) {
			case 0: 	reg |= TI_ANALOG_AP_STA_0DB; break;
			case 6: 	reg |= TI_ANALOG_AP_STA_6DB; break;
			case 9:		reg |= TI_ANALOG_AP_STA_9DB; break;
			case 12:	reg |= TI_ANALOG_AP_STA_12DB; break;
			case 18:	reg |= TI_ANALOG_AP_STA_18DB; break;
			default:
				REPORT(ERROR, "passed wrong sidetone attenuation -> use -18dB \n");
				reg |= TI_ANALOG_AP_STA_18DB;
		}
	}
	else {
		reg = 0;
		mask = TI_ANALOG_AP_STE;
	}
	return write_reg_mask(codec, TI_ANALOG_AP, mask, reg);
}

int aic23b_set_adc_input(t_tlv320aic23b *codec, enum AIC23B_ADC_INPUT in) {
	uint16_t reg;
	if (in == AIC23B_ADC_INPUT_MIC)
		reg = TI_ANALOG_AP_INSEL;
	else
		reg = 0;

	return write_reg_mask(codec, TI_ANALOG_AP, TI_ANALOG_AP_INSEL, reg);
}

/**
 * Enable/Disable digital HP filter
 *
 * @param codec		device instance
 * @param enabled	true to enable HP-filter
 * */
int aic23b_set_adc_hp(t_tlv320aic23b *codec, bool enabled) {
	uint16_t reg;
	if (enabled)
		reg = TI_DIGITAL_AP_ADCHP;
	else
		reg = 0;

	return write_reg_mask(codec, TI_DIGITAL_AP, TI_DIGITAL_AP_ADCHP, reg);
}

/**
 * Enable/Disable power down mode
 *
 * @param codec		device instance
 * @param down		true to set codec in power down mode, false to power up
 * */
int aic23b_set_power_down(t_tlv320aic23b *codec, bool down, bool adc) {
	uint16_t reg, mask;

	/* we dont power individual blocks, we just shut all up or down */
	if (down)
		if (adc)    //ADC Configuration
            reg =   TI_POWER_DOWN_OUT \
                  | TI_POWER_DOWN_DAC
                  | TI_POWER_DOWN_CLK;// \
                  | TI_POWER_DOWN_OSC;// \
                //reg = 0;//TI_POWER_DOWN_CLK;
                    /*CLKOUT off*/
                    /*HEAD & LINEOUT off*/
                    /*DAC off*/

        else        //DAC Configuration
            reg =   TI_POWER_DOWN_ADC \ 
                  | TI_POWER_DOWN_MIC;// \
                  | TI_POWER_DOWN_CLK \
                  | TI_POWER_DOWN_OSC;// \
                    | TI_POWER_DOWN_OUT \
                  | TI_POWER_DOWN_DAC;// \
                  | TI_POWER_DOWN_MIC \
                  | TI_POWER_DOWN_LINE;     //dont power down LINE, unknown error

                //reg = 0;//TI_POWER_DOWN_LINE;
    
                    /*CLKOUT off*/
                    /*OSC (XTO) off*/
                    /*ADC off*/
                    /*MIC IN off*/
                    /*LINE IN off*/
     
	else
		reg = 0;    //power ON for all

	mask = TI_POWER_DOWN_ALL;
	return write_reg_mask(codec, TI_POWER_DOWN, mask, reg);
}

int aic23b_set_if_format(t_tlv320aic23b *codec, bool master, bool lrswap, bool lrphase,
						enum AIC23B_I2S_INPUT_WITH in_bit_length, enum AIC23B_I2S_FORMAT format) {
	uint16_t reg = 0, mask;

	mask = TI_DIGITAL_AI_ALL;
	
	if (master)
		reg |= TI_DIGITAL_AI_MASTER;

	if (lrswap)
		reg |= TI_DIGITAL_AI_LRSWAP;

	if (lrphase)
		reg |= TI_DIGITAL_AI_LRP;

	switch (in_bit_length) {
		case AIC23B_I2S_INPUT_WITH_16BIT: reg |= TI_DIGITAL_AI_IWL_16; break;
		case AIC23B_I2S_INPUT_WITH_20BIT: reg |= TI_DIGITAL_AI_IWL_20; break;
		case AIC23B_I2S_INPUT_WITH_24BIT: reg |= TI_DIGITAL_AI_IWL_24; break;
		case AIC23B_I2S_INPUT_WITH_32BIT: reg |= TI_DIGITAL_AI_IWL_32; break;
		default:
			REPORT(ERROR, "passed wrong I2S bit lenght -> use 32 bit \n");
			reg |= TI_DIGITAL_AI_IWL_32;
	}

	switch (format) {
		case AIC23B_I2S_FORMAT_DSP:   reg |= TI_DIGITAL_AI_FOR_DSP; break;
		case AIC23B_I2S_FORMAT_I2S:   reg |= TI_DIGITAL_AI_FOR_I2S; break;
		case AIC23B_I2S_FORMAT_LEFT:  reg |= TI_DIGITAL_AI_FOR_LA; break;
		case AIC23B_I2S_FORMAT_RIGHT: reg |= TI_DIGITAL_AI_FOR_RA; break;
		default:
			REPORT(ERROR, "passed wrong I2S format -> use I2S mode \n");
			reg |= TI_DIGITAL_AI_FOR_I2S;
	}

	return write_reg_mask(codec, TI_DIGITAL_AI, mask, reg);
}

int aic23b_set_fs(t_tlv320aic23b *codec, uint32_t fs) {
	uint16_t reg = 0, mask;
	const uint16_t SR[4] = {TI_SRC_SR_0, TI_SRC_SR_1, TI_SRC_SR_2, TI_SRC_SR_3};
	const uint16_t BOSR  = TI_SRC_BOSR;

	mask = TI_SRC_ALL;

	/* we don't set clock divider */
	reg |= 0; /* TI_SRC_CLKOUT_DIV2 */
	reg |= 0; /* TI_SRC_CLKIN_DIV2 */

	/* sampling rate depending on mclk */
	switch (AIC23B_OSC_FREQ) {
		case 12000000:
			/* enable USB mode */
			reg |= TI_SRC_USB;
			switch (fs) {
				case 32000: reg |=         SR[2] | SR[1];                break;
				case 44100: reg |= SR[3] |                         BOSR; break;
				case 48000: reg |= 0;                                    break;
				case 88200: reg |= SR[3] | SR[2] | SR[1] | SR[0] | BOSR; break;
				case 96000: reg |=         SR[2] | SR[1] | SR[0];        break;
				default:
					REPORT(ERROR, "passed wrong sampling frequency: %d Hz \n", fs);
					return ERR_GENERAL;
			}
			break;
		/* The following mclk frequencies support either 44.1kHz or 48kHz but not both of them.
		 */
		case 12288000:
                        /* enable normal mode */
                        reg |= TI_SRC_NORMAL;
                        switch (fs) {
                            case 32000: reg |=         SR[2] | SR[1];                    break;
                            case 48000: reg |= 0;                                        break;
                            case 96000: reg |=         SR[2] | SR[1] | SR[0];            break;
                            default:
                                    REPORT(ERROR, "fs = %d Hz not allowed with MCLK = %d \n", fs, AIC23B_OSC_FREQ); break;
                                    return ERR_GENERAL;
                    }
                    break;
		case 11289600:
			REPORT(WARNING, "mclk = AIC23B_MCLK_11_2896MHZ not implemented !!!\n");
			return ERR_GENERAL;
		case 18432000:
			REPORT(WARNING, "mclk = AIC23B_MCLK_18_432MHZ not implemented !!!\n");
			return ERR_GENERAL;
		case 16934400:
			REPORT(WARNING, "mclk = AIC23B_MCLK_16_9344MHZ not implemented !!!\n");
			return ERR_GENERAL;
		default:
			REPORT(ERROR, "wrong mclk !!!\n");
			return ERR_GENERAL;
	}
	return write_reg_mask(codec, TI_SRC, mask, reg);
}

int aic23b_set_if_acitve(t_tlv320aic23b *codec, bool active) {
	uint16_t reg;

	if (active)
		reg = TI_DIGITAL_ACTIVATE_EN;
	else
		reg = 0;

	return write_reg_mask(codec, TI_DIGITAL_ACTIVATE, TI_DIGITAL_ACTIVATE_EN, reg);
}

