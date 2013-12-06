
#include "asi_hal.h"


/** Sets the audio parameters (channel count left/right and bits per channel)
 *
 * @param p pointer to the ACB registers
 * @param stream stream number to set the config on
 * @param ch_map active channels map
 * @param bits number of bits per channel
 * @return error code (0=no error; 1=unsupported bits per channel
 */ 
static uint8_t asi_set_aud_cfg(void* p, int unsigned stream, uint8_t ch_map, uint8_t bits)
{
    uint32_t reg_value = 0;

    REPORT(INFO, "asi_set_aud_cfg(%u) => active channels : 0x%01x | bits : %d", stream, aud_chmap2cnt(ch_map), bits);
    
    reg_value = aud_bits_to_container(bits);

    reg_value = ((reg_value << 8) & ASI_CONTAINER_MSK) | ((ch_map << 0) & ASI_CH_MAP_MSK);
    HOI_WR32(p, asi_reg2off(stream, ASI_REG_AUD_INFO), reg_value);

    return 0;
}

/** Reads the audio parameters (channel count left/right and bits per channel)
 *
 * @param p pointer to the ACB registers
 * @param stream stream number to read the config from 
 * @param ch_map active channels map
 * @param bits pointer to the number of bits per channel variable
 */ 
static void asi_get_aud_cfg(void* p, int unsigned stream, uint8_t* ch_map,  uint8_t* bits)
{
    uint32_t reg_value = HOI_RD32(p, asi_reg2off(stream, ASI_REG_AUD_INFO));

    REPORT(INFO, "asi_get_aud_cfg(%u) reg_value : %08x\n", stream, reg_value);

    *ch_map = (uint8_t) ((reg_value & ASI_CH_MAP_MSK) >> 0);
    *bits = aud_container_to_bits((reg_value & ASI_CONTAINER_MSK)>>8);
}

/** set the audio parameters (bits per sample, channel left count, channel right count)
 * 
 * @param p pointer to the ACB register
 * @param aud_params pointer to the audio parameter struct
 */
void asi_set_aud_params(void* p, int unsigned stream, struct hdoip_aud_params* aud_params)
{
    asi_set_aud_cfg(p, stream, aud_params->ch_map, aud_params->sample_width);
}

/** get the audio parameters (bit per sample, channel left count, channel right count)
 * 
 * @param p pointer to the ACB register
 * @param aud_params pointe to the audio parameter struct
 */
void asi_get_aud_params(void* p, int unsigned stream, struct hdoip_aud_params* aud_params)
{
    asi_get_aud_cfg(p, stream, &(aud_params->ch_map), &(aud_params->sample_width));
}

/** set following ethernet parameters: rtp ssrc
 * 
 * @param p pointer to the ACB registers
 * @param eth_params pointer to the ethernet parameter struct
 */ 
void asi_set_eth_params(void* p, int unsigned stream, struct hdoip_eth_params* eth_params)
{
    asi_set_ssrc(p, stream, eth_params->rtp_ssrc);
}

/** get following ethernet parameters: packet length and rtp ssrc
 * 
 * @param p pointer to the ACB registers
 * @param eth_params pointer to the ethernet parameter struct
 */ 
void asi_get_eth_params(void* p, int unsigned stream, struct hdoip_eth_params* eth_params)
{
    eth_params->packet_size = asi_get_frame_size(p, stream) * 4;
    eth_params->rtp_ssrc = asi_get_ssrc(p, stream);
}


