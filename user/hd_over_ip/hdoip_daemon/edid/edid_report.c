/*
 * edid_report.c
 *
 *  Created on: May 24, 2011
 *      Author: stth
 */

#include "hdoipd.h"
#include "edid.h"
#include "edid_report.h"

void edid_report_cvt_dsc(uint8_t *p)
{
    report(CONT "  %d @ %dHz ratio = 0x%02x supported rate = 0x%02x",
                                           EDID_CVT_LINES(p),
                                           EDID_CVT_VRATE(p),
                                           EDID_CVT_ASPECT_RATIO(p),
                                           EDID_CVT_SUPPORTED_RATE(p));
}

void edid_report_std_timing(uint16_t p)
{
    uint16_t hres;

    if(p != 0x0101) {
        hres = EDID_ST_HRES_DEC(p);
        switch(p & EDID_ST_AR) {
            case EDID_ST_AR_16x10: hres = hres * 10 / 16;
                                   break;
            case EDID_ST_AR_4x3: hres = hres * 3 / 4;
                                   break;
            case EDID_ST_AR_5x4: hres = hres * 4 / 5;
                                   break;
            default:
                                   break;
        }

        report(CONT "  %d x %d @ %dHz", EDID_ST_HRES_DEC(p),
                                        hres,
                                        EDID_ST_HZ_DEC(p));
    }
}

void edid_report_est(t_edid *edid)
{
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
}

void edid_report_est3(uint8_t *p)
{
    report(CONT "  version : %d", p[5]);
    if(p[6] & EDID_EST3_640x350_85) report(CONT "    640 x 350 @ 85 Hz");
    if(p[6] & EDID_EST3_640x400_85) report(CONT "    640 x 400 @ 85 Hz");
    if(p[6] & EDID_EST3_720x400_85) report(CONT "    720 x 400 @ 85 Hz");
    if(p[6] & EDID_EST3_640x480_85) report(CONT "    640 x 480 @ 85 Hz");
    if(p[6] & EDID_EST3_848x480_85) report(CONT "    848 x 480 @ 85 Hz");
    if(p[6] & EDID_EST3_800x600_85) report(CONT "    800 x 600 @ 85 Hz");
    if(p[6] & EDID_EST3_1024x768_85) report(CONT "    1024 x 768 @ 85 Hz");
    if(p[6] & EDID_EST3_1152x864_75) report(CONT "    1152 x 864 @ 75 Hz");

    if(p[7] & EDID_EST3_1280x768_60_RB) report(CONT "    1280 x 768 @ 60 Hz (reduced blanking)");
    if(p[7] & EDID_EST3_1280x768_60) report(CONT "    1280 x 768 @ 60 Hz");
    if(p[7] & EDID_EST3_1280x768_75) report(CONT "    1280 x 768 @ 75 Hz");
    if(p[7] & EDID_EST3_1280x768_85) report(CONT "    1280 x 768 @ 85 Hz");
    if(p[7] & EDID_EST3_1280x960_60) report(CONT "    1280 x 960 @ 60 Hz");
    if(p[7] & EDID_EST3_1280x960_85) report(CONT "    1280 x 960 @ 85 Hz");
    if(p[7] & EDID_EST3_1280x1024_60) report(CONT "    1280 x 1024 @ 60 Hz");
    if(p[7] & EDID_EST3_1280x1024_85) report(CONT "    1280 x 1024 @ 85 Hz");

    if(p[8] & EDID_EST3_1360x768_60) report(CONT "    1360 x 768 @ 60 Hz");
    if(p[8] & EDID_EST3_1440x900_60_RB) report(CONT "    1440 x 900 @ 60 Hz (reduced blanking)");
    if(p[8] & EDID_EST3_1440x900_60) report(CONT "    1440 x 900 @ 60 Hz");
    if(p[8] & EDID_EST3_1440x900_75) report(CONT "    1440 x 900 @ 75 Hz");
    if(p[8] & EDID_EST3_1440x900_85) report(CONT "    1440 x 900 @ 85 Hz");
    if(p[8] & EDID_EST3_1400x1050_60_RB) report(CONT "    1400 x 1050 @ 60 Hz (reduced blanking)");
    if(p[8] & EDID_EST3_1400x1050_60) report(CONT "    1400 x 1050 @ 60 Hz");
    if(p[8] & EDID_EST3_1400x1050_75) report(CONT "    1400 x 1050 @ 75 Hz");

    if(p[9] & EDID_EST3_1400x1050_85) report(CONT "    1400 x 1050 @ 85 Hz");
    if(p[9] & EDID_EST3_1680x1050_60_RB) report(CONT "    1680 x 1050 @ 85 Hz (reduced blanking)");
    if(p[9] & EDID_EST3_1680x1050_60) report(CONT "    1680 x 1050 @ 60 Hz");
    if(p[9] & EDID_EST3_1680x1050_75) report(CONT "    1680 x 1050 @ 75 Hz");
    if(p[9] & EDID_EST3_1680x1050_85) report(CONT "    1680 x 1050 @ 85 Hz");
    if(p[9] & EDID_EST3_1600x1200_60) report(CONT "    1600 x 1200 @ 60 Hz");
    if(p[9] & EDID_EST3_1600x1200_65) report(CONT "    1600 x 1200 @ 65 Hz");
    if(p[9] & EDID_EST3_1600x1200_70) report(CONT "    1600 x 1200 @ 70 Hz");

    if(p[10] & EDID_EST3_1600x1200_75) report(CONT "    1600 x 1200 @ 75 Hz");
    if(p[10] & EDID_EST3_1600x1200_85) report(CONT "    1600 x 1200 @ 85 Hz");
    if(p[10] & EDID_EST3_1792x1344_60) report(CONT "    1792 x 1344 @ 60 Hz");
    if(p[10] & EDID_EST3_1792x1344_75) report(CONT "    1792 x 1344 @ 75 Hz");
    if(p[10] & EDID_EST3_1856x1392_60) report(CONT "    1856 x 1392 @ 60 Hz");
    if(p[10] & EDID_EST3_1856x1392_75) report(CONT "    1856 x 1392 @ 75 Hz");
    if(p[10] & EDID_EST3_1920x1200_60_RB) report(CONT "    1920 x 1200 @ 60 Hz (reduced blanking)");
    if(p[10] & EDID_EST3_1920x1200_60) report(CONT "    1920 x 1200 @ 60 Hz");

    if(p[11] & EDID_EST3_1920x1200_75) report(CONT "    1920 x 1200 @ 75 Hz");
    if(p[11] & EDID_EST3_1920x1200_85) report(CONT "    1920 x 1200 @ 85 Hz");
    if(p[11] & EDID_EST3_1920x1440_60) report(CONT "    1920 x 1440 @ 60 Hz");
    if(p[11] & EDID_EST3_1920x1440_75) report(CONT "    1920 x 1440 @ 75 Hz");
}

void edid_report_color_point(uint8_t* p)
{
    report(CONT "  [1] white point : 0x%02x",EDID_CP_WHITE_POINT1(p));
    report(CONT "  [1] color point x: 0x%02x",EDID_CP_COLOR_POINT1_X(p));
    report(CONT "  [1] color point y: 0x%02x",EDID_CP_COLOR_POINT1_Y(p));
    report(CONT "  [1] gamma: 0x%02x",EDID_CP_GAMMA1(p));
    report(CONT "  [2] white point : 0x%02x",EDID_CP_WHITE_POINT2(p));
    report(CONT "  [2] color point x: 0x%02x",EDID_CP_COLOR_POINT2_X(p));
    report(CONT "  [2] color point y: 0x%02x",EDID_CP_COLOR_POINT2_Y(p));
    report(CONT "  [2] gamma: 0x%02x",EDID_CP_GAMMA2(p));
}

void edid_report_sgtf(uint8_t* p)
{
    uint16_t tmp;
    // Byte 0 - 9 defines display range limits
    // Byte 10 tag
    // Byte 11 reserved

    // Byte 12 start break frequency
    report(CONT "  start break frequency : %d kHz", p[12]);
    report(CONT "  C x 2                 : %d", p[13]);
    tmp = ((uint16_t)p[14]) | ((uint16_t) p[15]) << 8;
    report(CONT "  M                     : %d", tmp);
    report(CONT "  K                     : %d", p[16]);
    report(CONT "  J x 2                 : %d", p[17]);
}

void edid_report_cvt_support(uint8_t* p)
{
    uint16_t tmp;
    // Byte 0 - 9 defines display range limits
    // Byte 10 tag
    // Byte 11 version

    // Byte 12 Additional pixel clock precision
    report(CONT "  Max. Pix Clk : %d MHz", EDID_DRL_MAX_PIXCLK(p) - ((p[12]>>2) / 4));

    // Byte 13 Maximum active pixels per line
    tmp = (((uint16_t) p[12] & 0x03)<<8 | ((uint16_t)p[13])) * 8;
    report(CONT "  Max. active pixel per line : %d", tmp);

    // Byte 14 supported aspect ratios
    report(CONT "  supported aspect ration");
    if(p[14] & EDID_DRL_CVT_S_ARATIO_4_3) report(CONT "    4 : 3 AR");
    if(p[14] & EDID_DRL_CVT_S_ARATIO_16_9) report(CONT "    16 : 9 AR");
    if(p[14] & EDID_DRL_CVT_S_ARATIO_16_10) report(CONT "    16 : 10 AR");
    if(p[14] & EDID_DRL_CVT_S_ARATIO_5_4) report(CONT "    5 : 4 AR");
    if(p[14] & EDID_DRL_CVT_S_ARATIO_15_9) report(CONT "    15 : 9 AR");

    // Byte 15 preferred aspect ratio
    switch(p[15] & EDID_DRL_CVT_P_ARATIO_MASK) {
        case EDID_DRL_CVT_P_ARATIO_4_3     : report(CONT "  preferred aspect ration : 4 : 3 AR");
                                             break;
        case EDID_DRL_CVT_P_ARATIO_16_9     : report(CONT "  preferred aspect ration : 16 : 9 AR");
                                             break;
        case EDID_DRL_CVT_P_ARATIO_16_10     : report(CONT "  preferred aspect ration : 16 : 10 AR");
                                             break;
        case EDID_DRL_CVT_P_ARATIO_5_4     : report(CONT "  preferred aspect ration : 5 : 4 AR");
                                             break;
        case EDID_DRL_CVT_P_ARATIO_15_9     : report(CONT "  preferred aspect ration : 15 : 9 AR");
                                             break;
    }

    report(CONT "  standard CVT blanking : %s supported", ((p[15] & EDID_DRL_CVT_STD_BLANKING) ? "" : "not"));
    report(CONT "  reduced CVT blanking : %s supported", ((p[15] & EDID_DRL_CVT_RDC_BLANKING) ? "" : "not"));

    report(CONT "  type of display scaling supported");
    if(p[16] & EDID_DRL_CVT_HORZ_SHRINK) {
        report(CONT "    horizontal Shrink");
    }

    if(p[16] & EDID_DRL_CVT_HORZ_STRETCH) {
            report(CONT "    horizontal Stretch");
    }

    if(p[16] & EDID_DRL_CVT_VERT_SHRINK) {
        report(CONT "    vertical Shrink");
    }

    if(p[16] & EDID_DRL_CVT_VERT_STRETCH) {
            report(CONT "    vertical Stretch");
    }

    report(CONT "  preferred vertical refresh rate : %d", p[17]);


}

void edid_report_drl(uint8_t* p)
{
    report(CONT "  vertical offset: %d",EDID_DRL_VOFFSET(p));
    report(CONT "  horizontal offset: %d",EDID_DRL_HOFFSET(p));
    report(CONT "  minimum vertical rate: %d Hz",EDID_DRL_MIN_VRATE(p));
    report(CONT "  maximum vertical rate: %d Hz",EDID_DRL_MAX_VRATE(p));
    report(CONT "  minimum horizontal rate: %d Hz",EDID_DRL_MIN_HRATE(p));
    report(CONT "  maximum horizontal rate: %d Hz",EDID_DRL_MAX_HRATE(p));
    report(CONT "  maximum pixel clock: %d MHz",EDID_DRL_MAX_PIXCLK(p));

    switch(EDID_DRL_SUPPORT_FLAGS(p)) {
        case EDID_DRL_SUPPORT_FLAGS_DGTF :
                    report(CONT "  video timing support flags : default GTF");
                    break;
        case EDID_DRL_SUPPORT_FLAGS_RNG_LIMITS :
                    report(CONT "  video timing support flags : range limits only");
                    break;
        case EDID_DRL_SUPPORT_FLAGS_SGTF :
                    report(CONT "  video timing support flags : second GTF");
                    edid_report_sgtf(p);
                    break;
        case EDID_DRL_SUPPORT_FLAGS_CVT :
                    report(CONT "  video timing support flags : CVT support");
                    edid_report_cvt_support(p);
                    break;
        default:
                    report(CONT "  video timing support flags : reserved");
                    break;
    }
    // TODO: implement secondary timing support & GTF
}

void edid_report_dcm(uint8_t *p)
{
    report(CONT "  version : %d",EDID_DCM_VERSION(p));
    report(CONT "  red A2  : %d",EDID_DCM_RED_A2(p));
    report(CONT "  red A3  : %d",EDID_DCM_RED_A3(p));
    report(CONT "  green A2: %d",EDID_DCM_GREEN_A2(p));
    report(CONT "  green A3: %d",EDID_DCM_GREEN_A3(p));
    report(CONT "  blue A2 : %d",EDID_DCM_BLUE_A2(p));
    report(CONT "  blue A3 : %d",EDID_DCM_BLUE_A3(p));
}

void edid_report_detailed_timing(uint8_t *p)
{
    if((p[0] != 0) && (p[1] != 0)){
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
    if (EDID_IS_DSC(p)) { // p[0] and p[1] equal zero
        // Descriptor
        switch (EDID_DSC_TAG(p)) {
            case EDID_TAG_SERIAL:
                report(CONT "extension serial: %s", edid_report_str(p));
            break;
            case EDID_TAG_ASCII:
                report(CONT "extension text: %s", edid_report_str(p));
            break;
            case EDID_TAG_DRL:
                report(CONT "extension Display Range Limits:");
                edid_report_drl(p);
            break;
            case EDID_TAG_PRODUCT_NAME:
                report(CONT "extension product name: %s", edid_report_str(p));
            break;
            case EDID_TAG_COLOR_POINT:
                report(CONT "extension Color Management Data Definition");
                edid_report_color_point(p);
            break;
            case EDID_TAG_STID:
                report(CONT "extension Standard Timing Identifier Definition");
                for(int i=0 ; i<15 ; i++) {
                    edid_report_std_timing(EDID_STID_TIMING(p, i));
                }

            break;
            case EDID_TAG_DCM:
                report(CONT "extension Color Point Data:");
                edid_report_dcm(p);
            break;
            case EDID_TAG_CVT:
                report(CONT "extension CVT");
                report(CONT "  version : %d",EDID_CVT_VERSION(p));
                for(int i=6; i<18 ; i+=3) {
                    edid_report_cvt_dsc(p[i]);
                }
            break;
            case EDID_TAG_EST3:
                report(CONT "extension Established Timings III");
                edid_report_est3(p);
            break;
            case EDID_TAG_DUMMY:
                report(CONT "extension dummy");
            break;
        }
    } else {
        // Detailed timing
        report(CONT "detailed timing:");
        edid_report_detailed_timing(p);
    }
}

