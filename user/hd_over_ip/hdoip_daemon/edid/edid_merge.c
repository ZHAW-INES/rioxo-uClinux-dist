/*
 * edid_merge.c
 *
 *  Created on: Jun 17, 2011
 *      Author: stth
 */

#include <stdint.h>
#include <string.h>

#include "hdoipd.h"
#include "edid_merge.h"
#include "cea_861.h"

void edid_merge_display_dsc(uint8_t *edid, uint8_t *dscs1, uint8_t *dscs2)
{
    int offset = 0, i, j;
    int mask_drl=0, mask_est=0, mask_cvt=0;
    uint32_t tmp1, tmp2;
    uint8_t *ptr1, *ptr2, *ptr3;
    uint8_t *ptr_drl1, *ptr_drl2;
    uint8_t *ptr_est1, *ptr_est2;
    uint8_t *ptr_cvt1, *ptr_cvt2;

    for(i=0 ; i<4 ; i++) {
        ptr1 = dscs1 + (i * 18);
        if(!EDID_IS_DSC(ptr1)) {
            for(j=0 ; j<4 ; j++) {
                ptr2 = dscs2 + (j * 18);
                if(!EDID_IS_DSC(ptr2)) {
                    if(memcmp(ptr1, ptr2, 11) == 0) {
                        ptr3 = edid + offset * 18;
                        memcpy(ptr3, ptr1, 18);

                        offset++;
                    }
                } else {
                    switch(EDID_DSC_TAG(ptr2)) {
                        case EDID_TAG_DRL   :mask_drl |= 0x02; ptr_drl2 = ptr2;
                                             break;
                        case EDID_TAG_EST3  :mask_est |= 0x02; ptr_est2 = ptr2;
                                             break;
                        case EDID_TAG_CVT   :mask_cvt |= 0x02; ptr_cvt2 = ptr2;
                                             break;
                    }
                }
            }
        } else {
            switch(EDID_DSC_TAG(ptr1)) {
                case EDID_TAG_DRL   :mask_drl |= 0x01; ptr_drl1 = ptr1;
                                     break;
                case EDID_TAG_EST3  :mask_est |= 0x01; ptr_est1 = ptr1;
                                     break;
                case EDID_TAG_CVT   :mask_cvt |= 0x01; ptr_cvt1 = ptr1;
                                     break;
            }
        }
    }

    // display range limits & CVT
    ptr3 = edid + offset * 18;;
    if(mask_drl == 0x3) { // both has range limits
        memcpy(ptr3, ptr_drl1, 18);

        ptr3[4] &= ptr_drl2[4];

        if(ptr3[5] < ptr_drl2[5])   ptr3[5] = ptr_drl2[5];      //Min vertical rate
        if(ptr3[6] > ptr_drl2[6])   ptr3[6] = ptr_drl2[6];      //Max vertical rate
        if(ptr3[7] < ptr_drl2[7])   ptr3[7] = ptr_drl2[7];      //Min horizontal rate
        if(ptr3[8] > ptr_drl2[8])   ptr3[8] = ptr_drl2[8];      //Max horizontal rate
        if(ptr3[9] > ptr_drl2[9])   ptr3[9] = ptr_drl2[9];      //Max pixel clock
        if(ptr3[10] > ptr_drl2[10]) ptr3[10] = ptr_drl2[10];    //Video timing support flags

        // CVT support definitions
        if(ptr3[10] == EDID_DRL_SUPPORT_FLAGS_CVT) {
            tmp1 = ptr3[12] & 0xFC;
            tmp2 = ptr_drl2[12] & 0xFC;
            if(tmp1 < tmp2) ptr3[12] = (ptr3[12] & 0x03) | (ptr_drl2[12] & 0xFC);

            tmp1 = (((uint32_t)ptr3[12] & 0x03)<<8) | ptr3[13];
            tmp2 = (((uint32_t)ptr_drl2[12] & 0x03)<<8) | ptr_drl2[13];
            if(tmp1 < tmp2) {
                ptr3[13] = ptr_drl2[13];
                ptr3[12] = (ptr3[12] & 0xFC) | (ptr_drl2[12] & 0x3);
            }

            ptr3[14] &= ptr_drl2[14];                       //Supported aspect ratios
            ptr3[15] |= ((ptr3[15] & ptr_drl2[15]) & 0x18); //Preferred aspect ratio & CVT blankin support
            ptr3[16] &= ptr_drl2[16];                       //Type of Display scaling support
            if(ptr3[17] > ptr_drl2[17]) ptr3[17] = ptr_drl2[17];//preferred vertical refresh rate
        }

        offset++;
    }

    // established timing III
    ptr3 = edid + offset * 18;
    if(mask_est == 0x3) { // both has established timings III
        memcpy(ptr3, ptr_est1, 18);
        for(i=6 ; i<12 ; i++) {
            ptr3[i] = ptr_est1[i] & ptr_est2[i];
        }
        offset++;
    }

    // CVT timing
    ptr3 = edid + offset * 18;
    if(mask_cvt == 0x3) { // both has CVT descriptor
        if(memcmp(ptr_cvt1, ptr_cvt2, 18) == 0) {
            if(offset < 4) {
                memcpy(ptr3, ptr_cvt1, 18);
                offset++;
            }
        }
    }
}

void edid_byte_plot(t_edid *edid)
{
    uint8_t *ptr1;
    int i;

    ptr1 = (uint8_t *) edid;
    for(i=0; i<256 ; i+=4) {
        report(" 0x%02x : %02x %02x %02x %02x",i,ptr1[i],ptr1[i+1],ptr1[i+2],ptr1[i+3]);
    }
}

void edid_merge(t_edid *edid1, t_edid *edid2)
{
    int i, j;
    int offset;
    uint8_t *ptr1, *ptr2, *ptr3;
    t_edid edid;

    //***************************************************************************
    // Init new EDID

    // Copy edid1 into output edid
    memcpy((uint8_t *) &edid, (uint8_t *) edid1, 256);

    // Clear detailed timing (set to dummy descriptor)
    memset(edid.detailed_timing[0].tmp, 0, 4*18 * sizeof(uint8_t));
    edid.detailed_timing[0].tmp[3] = EDID_TAG_DUMMY;
    edid.detailed_timing[1].tmp[3] = EDID_TAG_DUMMY;
    edid.detailed_timing[2].tmp[3] = EDID_TAG_DUMMY;
    edid.detailed_timing[3].tmp[3] = EDID_TAG_DUMMY;

    // Clear standard timing
    memset(&edid.std_timings[0], 0x01, 8*2 * sizeof(uint8_t));

    // Clear extension block
    memset(&edid.extension_block, 0x00, 128 * sizeof(uint8_t));

    //***************************************************************************
    // Bytes : 0x14 - 0x18
    // Basic display parameters and features
    edid.video_input_definition &= edid2->video_input_definition;
    edid.feature &= edid2->feature;

    //***************************************************************************
    // Bytes : 0x36 - 0x7D
    // detailed timing modes
    // TODO: case no match found
    ptr1 = (uint8_t *)edid1->detailed_timing[0].tmp;
    ptr2 = (uint8_t *)edid2->detailed_timing[0].tmp;
    ptr3 = (uint8_t *)&edid.detailed_timing[0].tmp;
    edid_merge_display_dsc(ptr3, ptr1, ptr2);

    //***************************************************************************
    // Bytes : 0x26 - 0x35
    // standard timings
    offset = 0;
    for (i=0;i<8;i++) {
        if(edid1->std_timings[i] != 0x0101) { // if not unused data field
            for (j=0;j<8;j++) {
                if(edid1->std_timings[i] == edid2->std_timings[j]) {
                    edid.std_timings[offset] = edid1->std_timings[i];
                    offset++;
                }
            }
        }
    }

    //***************************************************************************
    // Bytes : 0x23 - 0x24
    // established timings I & II
    edid.timings = edid1->timings & edid2->timings;

    //***************************************************************************
    // Extension block

    // if extension block on both files exist
    if((edid1->extension_count == 1) && (edid2->extension_count == 1)) {
        if(edid1->extension_block.tag == edid2->extension_block.tag) { // if same extension block
            ptr1 = &edid1->extension_block.tag;
            ptr2 = &edid2->extension_block.tag;
            ptr3 = &edid.extension_block.tag;
            cea_861_merge((t_ext_cea_861 *) ptr3, (t_ext_cea_861 *) ptr1, (t_ext_cea_861 *) ptr2);
        } else {
            edid.extension_count = 0;
        }
    } else {
        edid.extension_count = 0;
    }

    //***************************************************************************
    // Bytes : 0x7F + offset
    // detailed timing modes (extension)
    // TODO: case no match found
    ptr1 = (uint8_t *)(&edid1->extension_block.tag) + edid1->extension_block.offset;
    ptr2 = (uint8_t *)(&edid2->extension_block.tag) + edid2->extension_block.offset;
    ptr3 = (uint8_t *)(&edid.extension_block.tag) + edid.extension_block.offset;
    edid_merge_display_dsc(ptr3, ptr1, ptr2);

    //***************************************************************************
    // Generate checksum
    edid_checksum_gen(&edid);

    memcpy((uint8_t *) edid1, (uint8_t *) &edid, 256);
}
