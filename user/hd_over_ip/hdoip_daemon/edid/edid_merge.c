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
#include "version.h"

void edid_merge_display_dsc(uint8_t *edid, uint8_t *dscs1, uint8_t *dscs2, bool force_write_timing)
{
    int offset = 0, i, j;
    int mask_drl=0, mask_est=0, mask_cvt=0, mask_nam=0;
    uint32_t tmp1, tmp2;
    uint8_t *ptr1 = dscs1;
    uint8_t *ptr2 = dscs1;
    uint8_t *ptr3 = dscs1;
    uint8_t *ptr_drl1 = dscs1;
    uint8_t *ptr_drl2 = dscs1;
    uint8_t *ptr_est1 = dscs1;
    uint8_t *ptr_est2 = dscs1;
    uint8_t *ptr_cvt1 = dscs1;
    uint8_t *ptr_cvt2 = dscs1;

    uint32_t size1_h, size2_h;
    uint32_t size1_v, size2_v;

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
                        case EDID_TAG_DRL           :mask_drl |= 0x02; ptr_drl2 = ptr2;
                                                    break;
                        case EDID_TAG_EST3          :mask_est |= 0x02; ptr_est2 = ptr2;
                                                    break;
                        case EDID_TAG_CVT           :mask_cvt |= 0x02; ptr_cvt2 = ptr2;
                                                    break;
                        case EDID_TAG_PRODUCT_NAME  :mask_nam |= 0x02;
                                                    break;
                    }
                }
            }
        } else {
            switch(EDID_DSC_TAG(ptr1)) {
                case EDID_TAG_DRL           :mask_drl |= 0x01; ptr_drl1 = ptr1;
                                            break;
                case EDID_TAG_EST3          :mask_est |= 0x01; ptr_est1 = ptr1;
                                            break;
                case EDID_TAG_CVT           :mask_cvt |= 0x01; ptr_cvt1 = ptr1;
                                            break;
                case EDID_TAG_PRODUCT_NAME  :mask_nam |= 0x01;
                                            break;
            }
        }
    }

    // if no detailed timing blocks match, write at least one timing block (that with the smaller resolution)
    if ((offset == 0) && force_write_timing) {

        for(i=0 ; i<4 ; i++) {
            ptr1 = dscs1 + (i * 18);
            if(!EDID_IS_DSC(ptr1)) {
                for(j=0 ; j<4 ; j++) {
                    ptr2 = dscs2 + (j * 18);
                    if(!EDID_IS_DSC(ptr2)) {

                        size1_h = ((ptr1[4] & 0xF0) << 4) | ptr1[2];
                        size1_v = ((ptr1[7] & 0xF0) << 4) | ptr1[5];

                        size2_h = ((ptr2[4] & 0xF0) << 4) | ptr2[2];
                        size2_v = ((ptr2[7] & 0xF0) << 4) | ptr2[5];
    
                        ptr3 = edid + offset * 18;
                        if ((size1_h*size1_v) < (size2_h*size2_v)) {
                            memcpy(ptr3, ptr1, 18);
                        } else {
                            memcpy(ptr3, ptr2, 18);
                        }
                        offset++;

                        goto descriptor_found;
                    }
                }
            }
        }
    }

    descriptor_found:

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

    // Display product name
    ptr3 = edid + offset * 18;
    if(mask_nam == 0x3) { // both has Display product name

#ifdef VERSION_LABEL_RIOXO
        ptr3[0]  = 0x00;
        ptr3[1]  = 0x00;
        ptr3[2]  = 0x00;
        ptr3[3]  = 0xFC;
        ptr3[4]  = 0x00;
        ptr3[5]  = 'R';
        ptr3[6]  = 'I';
        ptr3[7]  = 'O';
        ptr3[8]  = 'X';
        ptr3[9]  = 'O';
        ptr3[10] = 0x0A;
        ptr3[11] = 0x20;
        ptr3[12] = 0x20;
        ptr3[13] = 0x20;
        ptr3[14] = 0x20;
        ptr3[15] = 0x20;
        ptr3[16] = 0x20;
        ptr3[17] = 0x20;
#elif defined VERSION_LABEL_EMCORE
        ptr3[0]  = 0x00;
        ptr3[1]  = 0x00;
        ptr3[2]  = 0x00;
        ptr3[3]  = 0xFC;
        ptr3[4]  = 0x00;
        ptr3[5]  = 'E';
        ptr3[6]  = 'M';
        ptr3[7]  = 'C';
        ptr3[8]  = 'O';
        ptr3[9]  = 'R';
        ptr3[10] = 'E';
        ptr3[11] = 0x0A;
        ptr3[12] = 0x20;
        ptr3[13] = 0x20;
        ptr3[14] = 0x20;
        ptr3[15] = 0x20;
        ptr3[16] = 0x20;
        ptr3[17] = 0x20;
#elif defined VERSION_LABEL_BLACKBOX
        ptr3[0]  = 0x00;
        ptr3[1]  = 0x00;
        ptr3[2]  = 0x00;
        ptr3[3]  = 0xFC;
        ptr3[4]  = 0x00;
        ptr3[5]  = 'B';
        ptr3[6]  = 'L';
        ptr3[7]  = 'A';
        ptr3[8]  = 'C';
        ptr3[9]  = 'K';
        ptr3[10] = ' ';
        ptr3[11] = 'B';
        ptr3[12] = 'O';
        ptr3[13] = 'X';
        ptr3[14] = 0x0A;
        ptr3[15] = 0x20;
        ptr3[16] = 0x20;
        ptr3[17] = 0x20;
#else
    #error NO LABEL IS SELECTED IN FILE: "version.h"
#endif

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
    char *tmp;
    char year[5];
    char week[3];
    char sn[9];

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
    // Bytes : 0x08 - 0x11
    // Manufacturer, Serial number and date of manufacture

    // extract manufacturing date out of serial number 
    tmp = reg_get("serial-number");
    strncpy(year, &(tmp[0]), 4);
    year[4] = '\0';
    strncpy(week, &(tmp[4]), 2);
    week[2] = '\0';
    strncpy(sn,   &(tmp[6]), 8);
    sn[8] = '\0';

#ifdef VERSION_LABEL_RIOXO
    edid.id_manufacturer    = 0x0F4B; // RXO
#elif defined VERSION_LABEL_EMCORE
    edid.id_manufacturer    = 0xA315; // EMC
#elif defined VERSION_LABEL_BLACKBOX
    edid.id_manufacturer    = 0x5808; // BBX
#else
    #error NO LABEL IS SELECTED IN FILE: "version.h"
#endif

    edid.id_product_code    = 10;
    edid.id_serial_number   = atoi(sn);
    edid.year               = (uint8_t)(atoi(year) - 1990);
    edid.week               = (uint8_t)atoi(week);

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
    edid_merge_display_dsc(ptr3, ptr1, ptr2, true);

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

    // if no extension block is written, fill up extension block space with 0xFF
    if (edid.extension_count == 0) {
        ptr3 = &edid.extension_block.tag;
        for (i=0 ; i<128 ; i++) {
            *(ptr3+i) = 0xFF;
        }
    }

    //***************************************************************************
    // Bytes : 0x7F + offset
    // detailed timing modes (extension)
    // TODO: case no match found
    ptr1 = (uint8_t *)(&edid1->extension_block.tag) + edid1->extension_block.offset;
    ptr2 = (uint8_t *)(&edid2->extension_block.tag) + edid2->extension_block.offset;
    ptr3 = (uint8_t *)(&edid.extension_block.tag) + edid.extension_block.offset;
    edid_merge_display_dsc(ptr3, ptr1, ptr2, false);

    //***************************************************************************
    // Generate checksum
    edid_checksum_gen(&edid);

    memcpy((uint8_t *) edid1, (uint8_t *) &edid, 256);
}
