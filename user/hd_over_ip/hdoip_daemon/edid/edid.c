/*
 * edid.c
 *
 *  Created on: 31.01.2011
 *      Author: alda
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "hdoipd.h"
#include "edid.h"
#include "edid_report.h"
#include "cea_861.h"

static uint8_t edid_header[8] = {0,0xff,0xff,0xff,0xff,0xff,0xff,0};

int edid_read_file(t_edid* edid, char *file)
{
    int fd, ret;

    fd = open(file, O_RDONLY, 0600);

    if(fd != -1) {
        ret = read(fd, (void *) edid, sizeof(uint8_t) * 256);
        if(ret == -1) {
            return ret;
        }
    } else {
        return -2;
    }
    close(fd);

    return 0;
}

int edid_write_file(t_edid *edid, char *file)
{
    int ret, fd;
    uint8_t *buf = (uint8_t*) edid;

    fd = open(file, O_CREAT | O_RDWR, 0600);

    if(fd != -1) {
        ret = write(fd, &(buf[0]), sizeof(uint8_t) * 256);

        if(ret == -1) {
            return -1;
        }
    } else {
        return -1;
    }
    close(fd);

#ifdef EDID_WRITE_HEX_FILE
    strcat(file,"_hex");

    int fd_hex = fopen(file, "w");

    if(fd_hex != NULL) {
        for(int i=0 ; i<256 ; i+=8) {
            fprintf(fd_hex, "%02x %02x %02x %02x %02x %02x %02x %02x\n", buf[i],buf[i+1],buf[i+2],buf[i+3],buf[i+4],buf[i+5],buf[i+6],buf[i+7]);
        }
    } else {
        return -1;
    }

    fclose(fd_hex);
#endif // EDID_WRITE_HEX_FILE
    return 0;
}



int edid_verify(t_edid* edid)
{
    uint8_t* p8 = (uint8_t*)edid;
    uint8_t checksum = 0;
    int size = 128;

    if(edid->extension_count > 0) {
        size = 256;
    }


    if (memcmp(p8, edid_header, 8) != 0) {
        return -1;
    }

    for (int i=0;i<size;i++) {
        checksum += p8[i];
    }

    if (checksum) {
        return -1;
    }


    return 0;
}

int edid_checksum_gen(t_edid* edid)
{
    int i;
    uint8_t sum;
    uint8_t *ptr = (uint8_t *) edid;

    sum = 0;
    for(i=0 ; i<128-1 ; i++) {
        sum += ptr[i];
    }

    edid->checksum = (uint8_t) (0x100 - sum);

    sum = 0;
    for(i=128 ; i<256-1 ; i++) {
        sum += ptr[i];
    }

    edid->extension_block.checksum = (uint8_t) (0x100 - sum);
}


void edid_report(t_edid* edid)
{
    report(CONT "edid verify: %s", ((edid_verify(edid) == 0) ? "valid" : "error"));
    report(CONT "edid version is %d.%d", edid->edid_version, edid->edid_revision);

    uint8_t x = edid->hsize;
    uint8_t y = edid->vsize;
    if (!x && !y) {
        report(CONT "edid AR undefined");
    } else if (!x) {
        float ar = 100.0/((float)(y+99));
        report(CONT "edid AR(portrait) = %.2f", ar);
    } else if (!x) {
        float ar = ((float)(y+99))/100.0;
        report(CONT "edid AR(landscape) = %.2f", ar);
    } else {
        report(CONT "edid size = %d cm x %d cm", x, y);
    }

    report(CONT "edid video input definition: 0x%02x", edid->video_input_definition);
    if(edid->video_input_definition & 0x80) {
        report(CONT "edid DIGITAL input interfaces supported:");
        if(edid->video_input_definition & EDID_IND_IF_MASK) {
            switch(edid->video_input_definition & EDID_IND_IF_MASK)
            {
                case EDID_IND_IF_DVI:       report(CONT "DVI ");
                                            break;
                case EDID_IND_IF_HDMI_A:    report(CONT "HDMI-a ");
                                            break;
                case EDID_IND_IF_HDMI_B:    report(CONT "HDMI-b ");
                                            break;
                case EDID_IND_IF_MDDI:      report(CONT "MDDI ");
                                            break;
                case EDID_IND_IF_DISP_PORT: report(CONT "Display Port ");
                                            break;
            }
        }
    }
    report(CONT "edid feature: 0x%02x", edid->feature);

    float gamma = (float)(edid->gamma+100) / 100.0;
    report(CONT "edid gamma = %.2f", gamma);

    report(CONT "edid established timings:");
    edid_report_est(edid);

    report(CONT "edid standard timings:");
    for (int i=0;i<8;i++) {
        edid_report_std_timing(edid->std_timings[i]);
    }

    for (int i=0;i<4;i++) {
        edid_report_dsc(edid->detailed_timing[i].tmp);
    }

    report(CONT "edid extension count = %d", edid->extension_count);

    if(edid->extension_count == 1) {
        report(CONT "extension block : ");
        switch(edid->extension_block.tag) {
            case EDID_EXT_TAG_CEA:  report(CONT "CEA 861 Series Extension\n");
                                    cea_861_report((t_ext_cea_861*) &edid->extension_block.tag);
                                    break;
            case EDID_EXT_TAG_VTB:  report(CONT "Video Timing Block Extension\n");
                                    break;
            case EDID_EXT_TAG_DI:   report(CONT "Display Information Extension\n");
                                    break;
            case EDID_EXT_TAG_LS:   report(CONT "Localized String Extension\n");
                                    break;
            case EDID_EXT_TAG_DPVL: report(CONT "Digital Packet Video Link Extension\n");
                                    break;
            case EDID_EXT_TAG_BLOCK_MAP: report(CONT "Extension Block Map\n");
                                    break;
            case EDID_EXT_TAG_MANUFACT: report(CONT "Extension defined by the display manufacturer\n");
                                    break;
        }
    }
}

void edid_hoi_limit(t_edid* edid)
{

}

void edid_report_vid_timing(t_edid *edid)
{
    int i=0, flag=0;

    edid_report(edid);
    report("\n\n");

    // detailed timing modes
    report("1. priority");
    for(i=0 ; i<4 ; i++) {
        edid_report_detailed_timing(edid->detailed_timing[i].tmp);
    }

    // detailed timing modes in extensions
    report("2. priority");
    if(edid->extension_block.offset != 0) {
        for(i=0 ; i<4 ; i++) {
            edid_report_detailed_timing((&edid->extension_block.tag) + edid->extension_block.offset + (i * 18));
        }
    }

    // CVT timing
    // TODO CVT timing in extensions!
    report("3. priority");
    flag = 0;
    for(i=0 ; i<4 ; i++) {
        if(EDID_DSC_TAG(edid->detailed_timing[i].tmp) == EDID_TAG_CVT) {
            edid_report_cvt_dsc(edid->detailed_timing[i].tmp);
            flag = 1;
        }
    }
    if(flag == 0) {
        report(CONT "no CVT timing available");
    }

    // standard timings
    // TODO standard timing in extensions!
    report("4. priority");
    for (int i=0;i<8;i++) {
        edid_report_std_timing(edid->std_timings[i]);
    }

    // additional timing Mode (Established timings I, II & III, default GTF, GTF secondary Curve, CVT
    report("5. priority");

    // Established timings 23h & 24h -> tag F7h
    report("  established timing I & II");
    edid_report_est(edid);

    report("  established timing III");
    flag = 0;
    for (i=0;i<4; i++) {
        if(EDID_DSC_TAG(&edid->detailed_timing[i].tmp) == EDID_TAG_EST3) {
            flag = 1;
            break;
        }
    }

    if(flag == 1) {
        edid_report_est3((uint8_t *)&edid->detailed_timing[i].tmp);
    } else {
        report(CONT "    not available");
    }

    // display range limits & CVT
    flag = 0;
    for (int i=0;i<4;i++) {
        if(EDID_DSC_TAG(edid->detailed_timing[i].tmp) == EDID_TAG_DRL) {

            flag = 1;
        }
    }
    if(flag == 1) {
        report("  display range limits");
        edid_report_drl(edid->detailed_timing[i].tmp);
    }
}

