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

void edid_report_drl(uint8_t* p)
{
    report(CONT "  vertical offset: %d",EDID_DRL_VOFFSET(p));
    report(CONT "  horizontal offset: %d",EDID_DRL_HOFFSET(p));
    report(CONT "  minimum vertical rate: %d Hz",EDID_DRL_MIN_VRATE(p));
    report(CONT "  maximum vertical rate: %d Hz",EDID_DRL_MAX_VRATE(p));
    report(CONT "  minimum horizontal rate: %d Hz",EDID_DRL_MIN_HRATE(p));
    report(CONT "  maximum horizontal rate: %d Hz",EDID_DRL_MAX_HRATE(p));
    report(CONT "  maximum pixel clock: %d MHz",EDID_DRL_MAX_PIXCLK(p));
    report(CONT "  video timing support flags : 0x%02x",EDID_DRL_SUPPORT_FLAGS(p));
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

