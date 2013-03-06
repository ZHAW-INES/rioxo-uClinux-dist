#include "stdaud.h"
#include "std.h"

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
void aud_report_params(struct hdoip_aud_params* aud_params)
{
    REPORT(INFO, "Channel map         : 0x%01X", aud_params->ch_map);
    REPORT(INFO, "Sampling frequency  : %d Hz", aud_params->fs);
    REPORT(INFO, "Bits / sample       : %d bit\n", aud_params->sample_width);

}

