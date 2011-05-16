/*
 * edid.c
 *
 *  Created on: 31.01.2011
 *      Author: alda
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "edid.h"
#include "debug.h"

static uint8_t edid_header[8] = {0,0xff,0xff,0xff,0xff,0xff,0xff,0};

int edid_verify(t_edid* edid)
{
    uint8_t* p8 = (uint8_t*)edid;
    uint8_t checksum = 0;

    if (memcmp(p8, edid_header, 8) != 0) {
        return -1;
    }

    for (int i=0;i<256;i++) {
        checksum += p8[i];
    }

    if (checksum) {
        return -1;
    }

    return 0;
}

char* edid_report_str(uint8_t* p)
{
    static char tmp[14];
    memcpy(tmp, &p[5], 13);
    for (int i=0;i<13;i++) {
        if (p[5+i] == 0x0a) {
            tmp[i] = 0;
        } else {
            tmp[i] = p[5+i];
        }
    }
    tmp[13] = 0;
    return tmp;
}

void edid_report_dsc(uint8_t* p)
{
    if (EDID_IS_DSC(p)) {
        // Descriptor
        switch (EDID_DSC_TAG(p)) {
            case EDID_TAG_SERIAL:
                report(CONT "serial: %s", edid_report_str(p));
            break;
            case EDID_TAG_ASCII:
                report(CONT "text: %s", edid_report_str(p));
            break;
            case EDID_TAG_DRL:
                report(CONT "Display Range Limits");
            break;
            case EDID_TAG_PRODUCT_NAME:
                report(CONT "product name: %s", edid_report_str(p));
            break;
            case EDID_TAG_COLOR_POINT:
                report(CONT "Color Point Data");
            break;
            case EDID_TAG_STID:
                report(CONT "Standard Timing Identifier Definition");
            break;
            case EDID_TAG_DCM:
                report(CONT "Color Management Data Definition");
            break;
            case EDID_TAG_CVT:
                report(CONT "CVT");
            break;
            case EDID_TAG_EST3:
                report(CONT "Established Timings III");
            break;
            case EDID_TAG_DUMMY:
                report(CONT "dummy");
            break;
        }
    } else {
        // Detailed timing
        report(CONT "  %d x %d @ %d Hz H(%d-%d-%d) V(%d-%d-%d)",
                EDID_DT_WIDTH(p),
                EDID_DT_HEIGHT(p),
                EDID_DT_PFREQ(p),
                EDID_DT_HBLANK(p),
                EDID_DT_HFP(p),
                EDID_DT_HP(p),
                EDID_DT_VBLANK(p),
                EDID_DT_VFP(p),
                EDID_DT_VP(p)
                );
    }
}

void edid_report(t_edid* edid)
{
    report(CONT "edid verify: %s", (edid_verify(edid) ? "valid" : "error"));

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

    for (int i=0;i<8;i++) {
        uint16_t t = edid->std_timings[i];
        if ((t&EDID_ST_AR) == EDID_ST_AR_16x10) {
            report(CONT "  %d x %d @ %dHz", EDID_ST_HRES_DEC(t),
                                       EDID_ST_HRES_DEC(t)*10/16,
                                       EDID_ST_HZ_DEC(t));
        } else if ((t&EDID_ST_AR) == EDID_ST_AR_4x3) {
            report(CONT "  %d x %d @ %dHz", EDID_ST_HRES_DEC(t),
                                       EDID_ST_HRES_DEC(t)*3/4,
                                       EDID_ST_HZ_DEC(t));
        } else if ((t&EDID_ST_AR) == EDID_ST_AR_5x4) {
            report(CONT "  %d x %d @ %dHz", EDID_ST_HRES_DEC(t),
                                       EDID_ST_HRES_DEC(t)*4/5,
                                       EDID_ST_HZ_DEC(t));
        } else {
            report(CONT "  %d x %d @ %dHz", EDID_ST_HRES_DEC(t),
                                       EDID_ST_HRES_DEC(t)*9/16,
                                       EDID_ST_HZ_DEC(t));
        }
    }

    edid_report_dsc(edid->detailed_timing[0].tmp);
    edid_report_dsc(edid->detailed_timing[1].tmp);
    edid_report_dsc(edid->detailed_timing[2].tmp);
    edid_report_dsc(edid->detailed_timing[3].tmp);

    report(CONT "edid extension count = %d", edid->extension_count);
}

void edid_hoi_limit(t_edid* edid)
{

}

