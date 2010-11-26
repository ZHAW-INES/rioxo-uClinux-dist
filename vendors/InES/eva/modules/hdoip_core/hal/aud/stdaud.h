/*
 *  stdaud.h
 *
 *  Created on: 22.10.2010
 *      Author: stth
 */

#ifndef STDAUD_H_
#define STDAUD_H_

#include "std.h"

#define AUD_I2S_8BIT                (0x00000000)
#define AUD_I2S_16BIT               (0x00000100)
#define AUD_I2S_24BIT               (0x00000200)
#define AUD_I2S_32BIT               (0x00000300)

struct hdoip_aud_params {
    uint8_t     ch_cnt_left;        /* Channel count left */
    uint8_t     ch_cnt_right;       /* Channel count right */
    uint32_t    fs;                 /* sampling frequency */
    uint8_t     sample_width;       /* in bits */
} __attribute__((packed)); 

uint8_t aud_container_to_bits(uint32_t container);
uint32_t aud_bits_to_container(uint8_t bits);
uint8_t aud_get_sample_length(uint8_t bits, uint8_t channels);
void aud_report_params(struct hdoip_aud_params* aud_params);

#endif /* STDAUD_H_ */
