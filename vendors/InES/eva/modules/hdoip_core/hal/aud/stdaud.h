/*
 *  stdaud.h
 *
 *  Created on: 22.10.2010
 *      Author: stth
 */

#ifndef STDAUD_H_
#define STDAUD_H_

#include <linux/types.h>

#define AUD_I2S_8BIT                (0x00000000)
#define AUD_I2S_16BIT               (0x00000001)
#define AUD_I2S_24BIT               (0x00000002)
#define AUD_I2S_32BIT               (0x00000003)

/* audio streams */
#define AUD_STREAM_NR_EMBEDDED              0
#define AUD_STREAM_NR_IF_BOARD              1
#define AUD_STREAM_CNT                      1

struct hdoip_aud_params {
    uint8_t     ch_map;             /* Channel count left */
    uint32_t    fs;                 /* sampling frequency */
    uint8_t     sample_width;       /* in bits */
} __attribute__((packed));

uint8_t aud_get_sample_length(uint8_t bits, uint8_t channels);
void aud_report_params(struct hdoip_aud_params* aud_params);

/* converts container format to amount of bits
 *
 * @param container audio container format
 * @return amount of bits per sample
 */
static inline uint8_t aud_container_to_bits(uint32_t container)
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
static inline uint32_t aud_bits_to_container(uint8_t bits)
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
static inline uint8_t aud_chmap2cnt(uint16_t ch_map)
{
    int i;
    uint8_t ch = 0;

    for (i = 0; i < 15; i++) {
        if (ch_map & (1<<i)) ch++;
    }
    return ch;
}

#endif /* STDAUD_H_ */
