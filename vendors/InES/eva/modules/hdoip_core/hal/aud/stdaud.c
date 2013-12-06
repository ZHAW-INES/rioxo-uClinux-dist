#include "stdaud.h"
#include "std.h"

/* converts container format to amount of bits
 *
 * @param container audio container format
 * @return amount of bits per sample
 */
uint8_t aud_container_to_bits(uint32_t container)
{
    switch(container) {
        case AUD_I2S_8BIT : return 8;
        case AUD_I2S_16BIT: return 16;
        case AUD_I2S_24BIT: return 24;
        case AUD_I2S_32BIT: return 32;
        default: return 0;  /* unknow container */
    }
}

/* converts amount of bits to audio container format
 *
 * @param bits amount of bits per sample
 * @return audio container format
 */
uint32_t aud_bits_to_container(uint8_t bits)
{
    switch(bits) {
        case 8:  return AUD_I2S_8BIT;
        case 16: return AUD_I2S_16BIT;
        case 24: return AUD_I2S_24BIT;
        case 32: return AUD_I2S_32BIT;
        default: return 0;  /* unknow amount of bits */
    }
}

/** convert the active channels map to number of channels
 * @param ch_map        active channels bitvector
 * @return              number of active channels
 */
uint8_t aud_chmap2cnt(uint16_t ch_map)
{
    int i;
    uint8_t ch = 0;

    for (i = 0; i < 15; i++) {
        if (ch_map & (1<<i)) ch++;
    }
    return ch;
}

/* Align the amount of channels and bits per sample to 32 bit words
 *
 * @param bits bits per sample
 * @param channels total amount of channels
 * @return amount of 32 bit words
 */
uint8_t aud_get_sample_length(uint8_t bits, uint8_t channels)
{
    switch(bits) {
        case 8:     switch(channels) {
                        case 1: 
                        case 2: 
                        case 4: return 1;
                        case 8: return 2;
                        case 3: 
                        case 6: return 3;
                        case 5: return 5;
                        case 7: return 7;
                    }
        case 16:    switch(channels) {
                        case 1:
                        case 2: return 1;
                        case 4: return 2;
                        case 3:
                        case 6: return 3;
                        case 8: return 4;    
                        case 5: return 5;
                        case 7: return 7;
                    }
        case 24:    switch(channels) {  
                        case 1: 
                        case 2: 
                        case 4: return 3;
                        case 8: return 6;
                        case 3:
                        case 6: return 9;
                        case 5: return 15;
                        case 7: return 21;
                    }
        case 32:    return channels;
        default:    return 0;
    }
}

/** Prints all audio parameters to console
 *
 * @param aud_params pointer to the audio parameter struct
 */
void aud_report_params(struct hdoip_aud_params* aud_params, int unsigned stream)
{
    REPORT(INFO, "Stream              : %d", stream);
    REPORT(INFO, "Channel map         : 0x%01X", aud_params->ch_map);
    REPORT(INFO, "Sampling frequency  : %d Hz", aud_params->fs);
    REPORT(INFO, "Bits / sample       : %d bit\n", aud_params->sample_width);

}

