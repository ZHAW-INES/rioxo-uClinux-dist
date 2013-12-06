#ifndef TLV320AIC23B_DRV_H_
#define TLV320AIC23B_DRV_H_

#include "i2c_drv.h"

/* where you can set an amplifier gain */
enum AIC23B_AMPLIFIER       {AIC23B_AMPLIFIER_LINE_IN_L, AIC23B_AMPLIFIER_LINE_IN_R, AIC23B_AMPLIFIER_MIC,
                            AIC23B_AMPLIFIER_HEADPH_L, AIC23B_AMPLIFIER_HEADPH_R};
/* what you can mute */
enum AIC23B_MUTELINE        {AIC23B_MUTELINE_LINE_IN_L, AIC23B_MUTELINE_LINE_IN_R, AIC23B_MUTELINE_MIC,
                            AIC23B_MUTELINE_DAC};
/* the signal routed to the ADC */
enum AIC23B_ADC_INPUT       {AIC23B_ADC_INPUT_LINE, AIC23B_ADC_INPUT_MIC};
/* I2S interface input bit with */
enum AIC23B_I2S_INPUT_WITH  {AIC23B_I2S_INPUT_WITH_16BIT, AIC23B_I2S_INPUT_WITH_20BIT,
                            AIC23B_I2S_INPUT_WITH_24BIT, AIC23B_I2S_INPUT_WITH_32BIT};
/* I2S interface format: DSP mode, I2S mode, left justified, right justified */
enum AIC23B_I2S_FORMAT      {AIC23B_I2S_FORMAT_DSP, AIC23B_I2S_FORMAT_I2S, AIC23B_I2S_FORMAT_LEFT, AIC23B_I2S_FORMAT_RIGHT};

#define AIC23B_REG_CNT	16       /* we cache the reset or reserved registers as well*/
#define AIC23B_ADDR_RX  (0x01a<<1)
#define AIC23B_ADDR_TX  (0x01b<<1)
#define AIC23B_OSC_FREQ (12288000)

typedef struct {
	t_i2c                   *i2c_master;                   //!< I2C driver
	uint8_t                 devaddr;                       //!< device address on i2c
	bool                    available;                     //!< codec available
	uint16_t                registers[AIC23B_REG_CNT];     //!< keep copy of register content because they are write only
	bool                    master;                        //!< interfaces running in master or slave mode
} t_tlv320aic23b;


int aic23b_init(t_tlv320aic23b *codec, t_i2c *i2c_master, uint8_t devaddr, bool master);
int aic23b_set_gain(t_tlv320aic23b *codec, enum AIC23B_AMPLIFIER amp, int gain);
int aic23b_set_mute(t_tlv320aic23b *codec, enum AIC23B_MUTELINE ml, bool mute);
int aic23b_set_loop(t_tlv320aic23b *codec, bool loop);
int aic23b_set_sidetone(t_tlv320aic23b *codec, bool sidetone, int attenuation);
int aic23b_set_adc_input(t_tlv320aic23b *codec, enum AIC23B_ADC_INPUT in);
int aic23b_set_adc_hp(t_tlv320aic23b *codec, bool enabled);
int aic23b_set_power_down(t_tlv320aic23b *codec, bool down, bool adc);
int aic23b_set_if_format(t_tlv320aic23b *codec, bool master, bool lrswap, bool lrphase,
						enum AIC23B_I2S_INPUT_WITH in_bit_length, enum AIC23B_I2S_FORMAT format);
int aic23b_set_fs(t_tlv320aic23b *codec, uint32_t fs);
int aic23b_set_if_acitve(t_tlv320aic23b *codec, bool active);

#endif /* TLV320AIC23B_DRV_H_ */
