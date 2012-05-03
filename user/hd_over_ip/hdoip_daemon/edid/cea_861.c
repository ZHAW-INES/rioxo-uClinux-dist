/*
 * cea_861.c
 *
 *  Created on: May 23, 2011
 *      Author: stth
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hdoipd.h"
#include "cea_861.h"
#include "edid.h"
#include "edid_report.h"
#include "cea_861_svd.h"

void cea_861_report(t_ext_cea_861* ext)
{
    uint8_t *p = (uint8_t *) ext;
    uint8_t *block;
    uint8_t block_size = 0;

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
                case CEA861_TAG_VENDOR: report(CONT "vendor specific data block");
                                        if ((CEA861_IEEE_REG0(block) == 0x03) && (CEA861_IEEE_REG1(block) == 0x0C) && (CEA861_IEEE_REG2(block) == 0x00)) { // if vendor is "HDMI Licensing, LLC"
                                            report(CONT "  supported deep colors: ");
                                            if (CEA861_DC_48BIT(block)) report(CONT "    48bit");
                                            if (CEA861_DC_36BIT(block)) report(CONT "    36bit");
                                            if (CEA861_DC_30BIT(block)) report(CONT "    30bit");
                                        }
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
                case CEA861_TAG_EXTENDED:report(CONT "use Extended Tag");
                                        break;
            }
        }
    }

    if(ext->offset != 0) {
        report(CONT "detailed timing descriptors");
        for(int i=0; i<4 ; i++) {
            edid_report_detailed_timing(p+ext->offset+i*18);
        }
    }
}

void cea_861_merge(t_ext_cea_861 *cea, t_ext_cea_861 *cea1, t_ext_cea_861 *cea2)
{
    int i, j, size1, size2, cnt, k, l;
    uint8_t *ptr1, *ptr2, *ptr3;
    uint8_t SAD1[16][3] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}}; // SAD = Short Audio Descriptor
    uint8_t SAD2[16][3] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};

    ptr1 = (uint8_t *) cea1;
    ptr2 = (uint8_t *) cea2;
    ptr3 = (uint8_t *) cea;

    memset(cea, 0x00, 128);

    cea->tag = cea1->tag;
    cea->revision = cea1->revision;
    cea->options = cea1->options & cea2->options;   // TODO total number of native DTDs?
    cea->offset = 4;

    for(i=4; i<cea1->offset; i+=size1) {
        size1 = (ptr1[i] & 0x1F) + 1;
        for(j=4; j<cea2->offset; j+=size2) {
            size2 = (ptr2[j] & 0x1F) + 1;
            if((ptr1[i] & CEA861_TAG_MASK) == (ptr2[j] & CEA861_TAG_MASK)) {
                ptr3 = ((uint8_t *) cea) + cea->offset;
                switch(ptr1[i] & CEA861_TAG_MASK) {
                    case CEA861_TAG_AUDIO:  // write only SAD with same audio format code
                                            l = (size1-1)/3;
                                            for (k=0 ; k<l ; k++) {
                                                SAD1[((ptr1[i+1+(k*3)])&0x78)>>3][0] = ptr1[i+1+(k*3)];
                                                SAD1[((ptr1[i+1+(k*3)])&0x78)>>3][1] = ptr1[i+2+(k*3)];
                                                SAD1[((ptr1[i+1+(k*3)])&0x78)>>3][2] = ptr1[i+3+(k*3)];
                                            }

                                            l = (size2-1)/3;
                                            for (k=0 ; k<l ; k++) {
                                                SAD2[((ptr2[j+1+(k*3)])&0x78)>>3][0] = ptr2[j+1+(k*3)];
                                                SAD2[((ptr2[j+1+(k*3)])&0x78)>>3][1] = ptr2[j+2+(k*3)];
                                                SAD2[((ptr2[j+1+(k*3)])&0x78)>>3][2] = ptr2[j+3+(k*3)];
                                            }

                                            l = 0;
                                            for (k=0 ; k<16 ; k++) {
                                                if ((SAD1[k][0] != 0) && (SAD2[k][0] != 0)) { //search same audio format code
                                                    if (((SAD1[k][0] & 0x78)>>3) == 1) { // We support only LPCM
                                                        l++;
                                                        if ((SAD1[k][0] & 0x07) > (SAD2[k][0] & 0x07)) {
                                                            ptr3[1+((l-1)*3)] = SAD2[k][0];
                                                        } else {
                                                            ptr3[1+((l-1)*3)] = SAD1[k][0];
                                                        }
                                                        ptr3[2+((l-1)*3)] = SAD1[k][1] & SAD2[k][1];
                                                        ptr3[3+((l-1)*3)] = SAD1[k][2] & SAD2[k][2];
                                                    }
                                                }
                                            }

                                            ptr3[0] = ((ptr1[i] & (~0x1F)) | ((uint8_t)(l*3))); //write size due to count of SAD
                                            cea->offset += (l*3)+1;
                                            break;
                    case CEA861_TAG_SPEAKER:ptr3[0] = ptr1[i];
                                            ptr3[1] = ptr1[i+1] & ptr2[j+1];
                                            ptr3[2] = ptr1[i+2] & ptr2[j+2];
                                            ptr3[3] = ptr1[i+3];
                                            cea->offset += 4;
                                            break;
                    case CEA861_TAG_VIDEO:  cnt = 1;
                                            for(k=1 ; k<size1 ; k++) {
                                                for(l=1 ; l<size2 ; l++) {
                                                    if(CEA861_VIDEO_SVD(ptr1[i+k]) == CEA861_VIDEO_SVD(ptr2[j+l])) {
                                                        ptr3[cnt] = CEA861_VIDEO_SVD(ptr1[i+k]) | ((ptr1[i+k] & ptr2[j+l]) & 0x80);
                                                        cnt++;
                                                        break;
                                                    }
                                                }
                                            }
                                            if(cnt != 1) {
                                                ptr3[0] = (ptr1[i] & 0xE0) | ((cnt-1) & 0x1F);
                                                cea->offset += cnt;
                                            }
                                            break;
                    case CEA861_TAG_VENDOR: ptr3[0] = ptr1[i];
                                            for(k=1 ; k<size1 ; k++) {
                                                if ((k == 6) && ((ptr1[i+1] == 0x03) && (ptr1[i+2] == 0x0C) && (ptr1[i+3] == 0x00))) { // if vendor is "HDMI Licensing, LLC"
                                                    ptr3[k] = ptr1[i+k] & 0xAF; // do NOT support 30bit and 48bit deep color
                                                } else {
                                                    ptr3[k] = ptr1[i+k];
                                                }
                                            }
                                            cea->offset += size1;
                                            
                                            break;
                    case CEA861_TAG_EXTENDED: //copy block 1:1 from ptr1
                                            ptr3[0] = ptr1[i];
                                            for(k=1 ; k<size1 ; k++) {
                                                ptr3[k] = ptr1[i+k];
                                            }
                                            cea->offset += size1;
                                            break;

                    case CEA861_TAG_DTC:    ptr3[0] = ptr1[i]; //copy block 1:1 from ptr1
                                            for(k=1 ; k<size1 ; k++) {
                                                ptr3[k] = ptr1[i+k];
                                            }
                                            cea->offset += size1;
                                            break;
                    default:                break;
                }
            }
        }
    }
}
