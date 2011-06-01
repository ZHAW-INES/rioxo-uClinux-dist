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
#include "debug.h"
#include "edid.h"
#include "edid_report.h"
#include "cea_861.h"

static uint8_t edid_header[8] = {0,0xff,0xff,0xff,0xff,0xff,0xff,0};

char *file = "/tmp/edid";
char *file_hex = "/tmp/edid_hex";


int edid_compare(t_edid* edid1, t_edid* edid2)
{
    int i;
    uint8_t *buf1 = (uint8_t *) edid1;
    uint8_t *buf2 = (uint8_t *) edid2;

    for(i=0 ; i < 256 ; i++) {
        if(buf1[i] != buf2[i]) {
            return 0;
        }
    }
    return 1;
}

int edid_read_file(t_edid* edid)
{
    int fd, ret;

    fd = open(file, O_RDONLY, 0600);

    if(fd > -1) {
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

int edid_write_file(t_edid* edid)
{
    int ret, fd;
    uint8_t *buf = (uint8_t*) edid;

    fd = open(file, O_CREAT | O_RDWR, 0600);

    if(fd > -1) {
        ret = write(fd, &(buf[0]), sizeof(uint8_t) * 256);

        if(ret == -1) {
            return -1;
        }
    } else {
        return -1;
    }
    close(fd);

#ifdef EDID_WRITE_HEX_FILE
    int fd_hex = fopen(file_hex, "w");

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

    if (memcmp(p8, edid_header, 8) != 0) {
        return -1;
    }

    for (int i=0;i<128;i++) {
        checksum += p8[i];
    }

    if (checksum) {
        return -1;
    }


    return 0;
}



void edid_report(t_edid* edid)
{
#ifdef REPORT_EDID
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
    if (edid->timings & EDID_ET_800x600_60) report(CONT "  800 x 600 @ 60Hz");
    if (edid->timings & EDID_ET_800x600_56) report(CONT "  800 x 600 @ 56Hz");
    if (edid->timings & EDID_ET_640x480_75) report(CONT "  640 x 480 @ 75Hz");
    if (edid->timings & EDID_ET_640x480_72) report(CONT "  640 x 480 @ 72Hz");
    if (edid->timings & EDID_ET_640x480_67) report(CONT "  640 x 480 @ 67Hz");
    if (edid->timings & EDID_ET_640x480_60) report(CONT "  640 x 480 @ 60Hz");
    if (edid->timings & EDID_ET_720x400_88) report(CONT "  720 x 400 @ 88Hz");
    if (edid->timings & EDID_ET_720x400_70) report(CONT "  720 x 400 @ 70Hz");
    if (edid->timings & EDID_ET_1280x1024_75) report(CONT "  1280 x 1024 @ 75Hz");
    if (edid->timings & EDID_ET_1024x768_75) report(CONT "  1024 x 768 @ 75Hz");
    if (edid->timings & EDID_ET_1024x768_70) report(CONT "  1024 x 768 @ 70Hz");
    if (edid->timings & EDID_ET_1024x768_60) report(CONT "  1024 x 768 @ 60Hz");
    if (edid->timings & EDID_ET_1024x768_87i) report(CONT "  1024 x 768 @ 87Hz(i)");
    if (edid->timings & EDID_ET_832x624_75) report(CONT "  832 x 624 @ 60Hz");
    if (edid->timings & EDID_ET_800x600_75) report(CONT "  800 x 600 @ 75Hz");
    if (edid->timings & EDID_ET_800x600_72) report(CONT "  800 x 600 @ 72Hz");

    report(CONT "edid standard timings:");
    for (int i=0;i<8;i++) {
        edid_report_std_timing(edid->std_timings[i]);
    }

    edid_report_dsc(edid->detailed_timing[0].tmp);
    edid_report_dsc(edid->detailed_timing[1].tmp);
    edid_report_dsc(edid->detailed_timing[2].tmp);
    edid_report_dsc(edid->detailed_timing[3].tmp);

    report(CONT "edid extension count = %d", edid->extension_count);

    if(edid->extension_count == 1) {
        report(CONT "extension block : ");
        switch(edid->extension_block[0]) {
            case EDID_EXT_TAG_CEA:  report(CONT "CEA 861 Series Extension\n");
                                    cea_861_report((t_ext_cea_861*) edid->extension_block);
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
#endif // REPORT_EDID
}

void edid_hoi_limit(t_edid* edid)
{

}

