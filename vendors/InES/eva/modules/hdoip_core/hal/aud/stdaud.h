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
#define AUD_STREAM_CNT                      2

struct hdoip_aud_params {
    uint8_t     ch_map;             /* Channel count left */
    uint32_t    fs;                 /* sampling frequency */
    uint8_t     sample_width;       /* in bits */
} __attribute__((packed));

uint8_t aud_get_sample_length(uint8_t bits, uint8_t channels);
void aud_report_params(struct hdoip_aud_params* aud_params, int unsigned stream);

uint32_t aud_bits_to_container(uint8_t bits);
uint8_t aud_container_to_bits(uint32_t container);
uint8_t aud_chmap2cnt(uint16_t ch_map);



#endif /* STDAUD_H_ */
