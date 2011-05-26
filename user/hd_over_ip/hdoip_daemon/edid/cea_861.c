/*
 * cea_861.c
 *
 *  Created on: May 23, 2011
 *      Author: stth
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "cea_861.h"
#include "edid.h"
#include "edid_report.h"
#include "cea_861_svd.h"

void cea_861_report(t_ext_cea_861* ext)
{
    uint8_t *p = (uint8_t *) ext;
    uint8_t *block;
    uint8_t block_size = 0;
    char *str;

    report(CONT "CEA 861 tag      : 0x%02x", ext->tag);
    report(CONT "CEA 861 revision : %d", ext->revision);
    report(CONT "CEA 861 offset   : %d", ext->offset);

    if(ext->revision > 0x02) {
        for(int i=4; i<ext->offset; i+=block_size) {
            block_size = (p[i]&0x1F) + 1;
            block = ((uint8_t *)p+i);

            switch(p[i] & CEA861_TAG_MASK) {
                case CEA861_TAG_AUDIO:  report(CONT "audio data block");
                                        report(CONT "  format code : 0x%02x", CEA861_AUDIO_FORMAT_CODE(block));
                                        report(CONT "  max audio channels : %d",CEA861_AUDIO_CHANNELS(block));

                                        report(CONT "  supported fs: ");
                                        if(CEA861_AUDIO_32KHZ(block)) report(CONT "    32kHz");
                                        if(CEA861_AUDIO_44_1KHZ(block)) report(CONT "    44.1kHz");
                                        if(CEA861_AUDIO_48KHZ(block)) report(CONT "    48kHz");
                                        if(CEA861_AUDIO_88_2KHZ(block)) report(CONT "    88.2kHz");
                                        if(CEA861_AUDIO_96KHZ(block)) report(CONT "    96kHz");
                                        if(CEA861_AUDIO_176_4KHZ(block)) report(CONT "    176.4kHz");
                                        if(CEA861_AUDIO_192KHZ(block)) report(CONT "    192kHz");

                                        if(CEA861_AUDIO_FORMAT_CODE(block) == 1) {
                                            report(CONT "  bit width:");
                                            if(CEA861_AUDIO_16BIT(block)) report(CONT "    16bit");
                                            if(CEA861_AUDIO_20BIT(block)) report(CONT "    20bit");
                                            if(CEA861_AUDIO_24BIT(block)) report(CONT "    24bit");
                                        } else if((CEA861_AUDIO_FORMAT_CODE(block) > 1) &&
                                           (CEA861_AUDIO_FORMAT_CODE(block) < 9)) {
                                            report(CONT "  maximum bit rate : %d kHz", CEA861_AUDIO_MAX_RATE(block));
                                        }
                                        break;
                case CEA861_TAG_VIDEO:  report(CONT "video data block");
                                        for(int j=1 ; j < block_size ; j++) {
                                            report(CONT "  %s %s",cea_861_svd[CEA861_VIDEO_SVD(block[j])], (CEA861_VIDEO_NATIVE(block[j]) == 0 ? "" : "[native]"));
                                        }
                                        break;
                case CEA861_TAG_VENDOR: report(CONT "vendor specific data block (not parsed)");
                                        break;
                case CEA861_TAG_SPEAKER:report(CONT "speaker allocation data block");

                                        if(CEA861_SPEAKER_FLW_FRW(block)) report(CONT "  FLW/FRW");
                                        if(CEA861_SPEAKER_RLC_RRC(block)) report(CONT "  RLC/RRC");
                                        if(CEA861_SPEAKER_FLC_FRC(block)) report(CONT "  FLC/FRC");
                                        if(CEA861_SPEAKER_RC(block)) report(CONT "  RC");
                                        if(CEA861_SPEAKER_RL_RR(block)) report(CONT "  RL/RR");
                                        if(CEA861_SPEAKER_FC(block)) report(CONT "  FC");
                                        if(CEA861_SPEAKER_LFE(block)) report(CONT "  LFE");
                                        if(CEA861_SPEAKER_FL_FR(block)) report(CONT "  FL/FR");
                                        if(CEA861_SPEAKER_FCH(block)) report(CONT "  FCH");
                                        if(CEA861_SPEAKER_TC(block)) report(CONT "  TC");
                                        if(CEA861_SPEAKER_FLH_FRH(block)) report(CONT "  FLH/FRH");
                                        break;
                case CEA861_TAG_DTC:    report(CONT "VESA DTC data block");
                                        break;
                case CEA861_TAG_EXTENDED:report(CONT "use Extended Tag (not parsed)");
                                        break;
            }
        }
    }

    if(ext->offset != 0) {
        report(CONT "detailed timing descriptors");
        for(int i=ext->offset; i<128 ; i+=18) {
            edid_report_detailed_timing(p+i);
        }
    }
}

