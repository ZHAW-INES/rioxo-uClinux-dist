/*
 * cea_861.h
 *
 *  Created on: May 23, 2011
 *      Author: stth
 */

#ifndef CEA_861_H_
#define CEA_861_H_

#include "stdint.h"


// data block tag
#define CEA861_TAG_MASK             (0xE0)
#define CEA861_TAG_AUDIO            (0x20)
#define CEA861_TAG_VIDEO            (0x40)
#define CEA861_TAG_VENDOR           (0x60)
#define CEA861_TAG_SPEAKER          (0x80)
#define CEA861_TAG_DTC              (0xA0)
#define CEA861_TAG_EXTENDED         (0xE0)

// audio data block
#define CEA861_AUDIO_CHANNELS(p)    (((p[1]) & 0x07) + 1)
#define CEA861_AUDIO_FORMAT_CODE(p) (((p[1]) & 0x78)>>3)
#define CEA861_AUDIO_192KHZ(p)      ((p[2] & 0x40)>>6)
#define CEA861_AUDIO_176_4KHZ(p)    ((p[2] & 0x20)>>5)
#define CEA861_AUDIO_96KHZ(p)       ((p[2] & 0x10)>>4)
#define CEA861_AUDIO_88_2KHZ(p)     ((p[2] & 0x08)>>3)
#define CEA861_AUDIO_48KHZ(p)       ((p[2] & 0x04)>>2)
#define CEA861_AUDIO_44_1KHZ(p)     ((p[2] & 0x02)>>1)
#define CEA861_AUDIO_32KHZ(p)       ((p[2] & 0x01))
#define CEA861_AUDIO_24BIT(p)       ((p[3] & 0x04)>>2)
#define CEA861_AUDIO_20BIT(p)       ((p[3] & 0x02)>>1)
#define CEA861_AUDIO_16BIT(p)       ((p[3] & 0x01))
#define CEA861_AUDIO_MAX_RATE(p)     (p[3] * 8)

// speaker allocation data block
#define CEA861_SPEAKER_FLW_FRW(p)   ((p[1] & 0x80)>>7)
#define CEA861_SPEAKER_RLC_RRC(p)   ((p[1] & 0x40)>>6)
#define CEA861_SPEAKER_FLC_FRC(p)   ((p[1] & 0x20)>>5)
#define CEA861_SPEAKER_RC(p)        ((p[1] & 0x10)>>4)
#define CEA861_SPEAKER_RL_RR(p)     ((p[1] & 0x08)>>3)
#define CEA861_SPEAKER_FC(p)        ((p[1] & 0x04)>>2)
#define CEA861_SPEAKER_LFE(p)       ((p[1] & 0x02)>>1)
#define CEA861_SPEAKER_FL_FR(p)     ((p[1] & 0x01))
#define CEA861_SPEAKER_FCH(p)       ((p[2] & 0x04)>>2)
#define CEA861_SPEAKER_TC(p)        ((p[2] & 0x02)>>1)
#define CEA861_SPEAKER_FLH_FRH(p)   ((p[2] & 0x01))



#define CEA861_VIDEO_SVD(p)         (p&0x7F)
#define CEA861_VIDEO_NATIVE(p)      ((p&0x80)>>7)

typedef struct {
    uint8_t     tag;                // 0x00
    uint8_t     revision;           // 0x01
    uint8_t     offset;             // 0x02
    uint8_t     options;            // 0x03
    uint8_t     descriptors[123];   // 0x04 - 0x7E
    uint8_t     checksum;           // 0x7F
} __attribute__((packed)) t_ext_cea_861;

void cea_861_report(t_ext_cea_861* ext);
void cea_861_merge(t_ext_cea_861 *cea, t_ext_cea_861 *cea1, t_ext_cea_861 *cea2);

#endif /* CEA_861_H_ */
