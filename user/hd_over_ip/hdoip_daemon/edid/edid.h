/*
 * edid.h
 *
 *  Created on: 31.01.2011
 *      Author: alda
 */

#ifndef EDID_H_
#define EDID_H_

#include "stdint.h"


typedef struct {
    uint8_t         tmp[18];
} __attribute__((packed)) t_edid_timing;

typedef struct {
    uint8_t     tag;                // 0x00
    uint8_t     revision;           // 0x01
    uint8_t     offset;             // 0x02
    uint8_t     data[124];          // 0x03 - 0x7E
    uint8_t     checksum;           // 0x7F
} __attribute__((packed)) t_edid_ext_block;

typedef struct {
    uint8_t             header[8];              // 0x00 - 0x07 !< should be "00 ff ff ff ff ff ff 00"
    uint16_t            id_manufacturer;        // 0x08 - 0x09
    uint16_t            id_product_code;        // 0x0A - 0x0B
    uint32_t            id_serial_number;       // 0x0C - 0x0F
    uint8_t             week;                   // 0x10
    uint8_t             year;                   // 0x11
    uint8_t             edid_version;           // 0x12
    uint8_t             edid_revision;          // 0x13
    uint8_t             video_input_definition; // 0x14
    uint8_t             hsize;                  // 0x15
    uint8_t             vsize;                  // 0x16
    uint8_t             gamma;                  // 0x17
    uint8_t             feature;                // 0x18
    uint8_t             color[10];              // 0x19 - 0x22
    uint16_t            timings;                // 0x23 - 0x24
    uint8_t             timings_reserved;       // 0x25
    uint16_t            std_timings[8];         // 0x26 - 0x35
    t_edid_timing       detailed_timing[4];     // 0x36 - 0x7D
    uint8_t             extension_count;        // 0x7E
    uint8_t             checksum;               // 0x7F
    t_edid_ext_block    extension_block;        // 0x80 - 0xFF
} __attribute__((packed)) t_edid;

// established timings - bitmap
#define EDID_ET_800x600_60          (0x0001)
#define EDID_ET_800x600_56          (0x0002)
#define EDID_ET_640x480_75          (0x0004)
#define EDID_ET_640x480_72          (0x0008)
#define EDID_ET_640x480_67          (0x0010)
#define EDID_ET_640x480_60          (0x0020)
#define EDID_ET_720x400_88          (0x0040)
#define EDID_ET_720x400_70          (0x0080)
#define EDID_ET_1280x1024_75        (0x0100)
#define EDID_ET_1024x768_75         (0x0200)
#define EDID_ET_1024x768_70         (0x0400)
#define EDID_ET_1024x768_60         (0x0800)
#define EDID_ET_1024x768_87i        (0x1000)
#define EDID_ET_832x624_75          (0x2000)
#define EDID_ET_800x600_75          (0x4000)
#define EDID_ET_800x600_72          (0x8000)

// standard timings - bitmap
#define EDID_ST_HRES_DEC(x)         (((x&0xff)+31)*8)
#define EDID_ST_HRES_ENC(x)         (((x>>3)-31)&0xff)
#define EDID_ST_HZ_DEC(x)           (((x>>8)&0x3f)+60)
#define EDID_ST_HZ_ENC(x)           (((x-60)&0x3f)<<8)
#define EDID_ST_AR                  0xc000
#define EDID_ST_AR_16x10            0x0000
#define EDID_ST_AR_4x3              0x4000
#define EDID_ST_AR_5x4              0x8000
#define EDID_ST_AR_16x9             0xc000

// standard timings (#9 to #14)
#define EDID_STID_TIMING(p, nr)       (((uint16_t)(p[(nr)-3]) << 8) | (uint16_t)(p[(nr)-4]))

// Descriptor
#define EDID_TAG_SERIAL             0xff
#define EDID_TAG_ASCII              0xfe
#define EDID_TAG_DRL                0xfd
#define EDID_TAG_PRODUCT_NAME       0xfc
#define EDID_TAG_COLOR_POINT        0xfb
#define EDID_TAG_STID               0xfa
#define EDID_TAG_DCM                0xf9
#define EDID_TAG_CVT                0xf8
#define EDID_TAG_EST3               0xf7
#define EDID_TAG_DUMMY              0x10

#define EDID_IS_DSC(p)              ((!(p)[0])&&(!(p)[1]))
#define EDID_DSC_TAG(p)             ((p)[3])

// Detailed Timing
#define EDID_DT_PFREQ(p)            ((uint32_t)(((((uint32_t)p[1])<<8) + ((uint32_t)p[0]))*10000))
#define EDID_DT_WIDTH(p)            (((uint32_t)p[2]) | (((uint32_t)p[4]&0xf0)<<4))
#define EDID_DT_HBLANK(p)           (((uint32_t)p[3]) | (((uint32_t)p[4]&0x0f)<<8))
#define EDID_DT_HEIGHT(p)           (((uint32_t)p[5]) | (((uint32_t)p[7]&0xf0)<<4))
#define EDID_DT_VBLANK(p)           (((uint32_t)p[6]) | (((uint32_t)p[7]&0x0f)<<8))
#define EDID_DT_HFP(p)              (((uint32_t)p[8]) | (((uint32_t)p[11]&0xc0)<<2))
#define EDID_DT_HP(p)               (((uint32_t)p[9]) | (((uint32_t)p[11]&0x30)<<4))
#define EDID_DT_VFP(p)              (((((uint32_t)p[10])&0xf0)>>4) | (((uint32_t)p[11]&0x0c)<<2))
#define EDID_DT_VP(p)               (((((uint32_t)p[10])&0x0f)) | (((uint32_t)p[11]&0x03)<<4))

// Color point descriptor
#define EDID_CP_WHITE_POINT1(p)     (p[5])
#define EDID_CP_COLOR_POINT1_X(p)   (p[7] | ((p[6] & 0x0C)>>2))
#define EDID_CP_COLOR_POINT1_Y(p)   (p[8] | ((p[6] & 0x03)))
#define EDID_CP_GAMMA1(p)           (p[9])
#define EDID_CP_WHITE_POINT2(p)     (p[10])
#define EDID_CP_COLOR_POINT2_X(p)   (p[12] | ((p[11] & 0x0C)>>2))
#define EDID_CP_COLOR_POINT2_Y(p)   (p[13] | ((p[11] & 0x03)))
#define EDID_CP_GAMMA2(p)           (p[14])

// Display range limits
#define EDID_DRL_VOFFSET(p)         ((p[4] & 0x03))
#define EDID_DRL_HOFFSET(p)         (((p[4] & 0x0C)>>2))
#define EDID_DRL_MIN_VRATE(p)       ((p[5]))
#define EDID_DRL_MAX_VRATE(p)       ((p[6]))
#define EDID_DRL_MIN_HRATE(p)       ((p[7]))
#define EDID_DRL_MAX_HRATE(p)       ((p[8]))
#define EDID_DRL_MAX_PIXCLK(p)      (((p[9]) * 10))
#define EDID_DRL_SUPPORT_FLAGS(p)   ((p[10]))

// DRL_SUPPORT_FLAGS
#define EDID_DRL_SUPPORT_FLAGS_DGTF         (0x00)
#define EDID_DRL_SUPPORT_FLAGS_RNG_LIMITS   (0x01)
#define EDID_DRL_SUPPORT_FLAGS_SGTF         (0x02)
#define EDID_DRL_SUPPORT_FLAGS_CVT          (0x04)

// Color management data definition
#define EDID_DCM_VERSION(p)         (p[5])
#define EDID_DCM_RED_A2(p)          (((uint16_t)p[8]) | ((uint16_t)p[9]<<8))
#define EDID_DCM_RED_A3(p)          (((uint16_t)p[6]) | ((uint16_t)p[7]<<8))
#define EDID_DCM_GREEN_A2(p)        (((uint16_t)p[12]) | ((uint16_t)p[13]<<8))
#define EDID_DCM_GREEN_A3(p)        (((uint16_t)p[10]) | ((uint16_t)p[11]<<8))
#define EDID_DCM_BLUE_A2(p)         (((uint16_t)p[16]) | ((uint16_t)p[17]<<8))
#define EDID_DCM_BLUE_A3(p)         (((uint16_t)p[14]) | ((uint16_t)p[15]<<8))

// CVT 3 Byte code descriptor definition
#define EDID_CVT_VERSION(p)         (p[5])
#define EDID_CVT_LINES(p)           (((uint16_t) p[0]) | ((uint16_t) p[1]>>4))
#define EDID_CVT_ASPECT_RATIO(p)    ((p[1]&0x0C)>>2)
#define EDID_CVT_VRATE(p)           ((p[2]&0x60)>>5)
#define EDID_CVT_SUPPORTED_RATE(p)  (p[2]&0x1F)

// Range limits & CVT Support
#define EDID_DRL_CVT_S_ARATIO_MASK  (0xF8)
#define EDID_DRL_CVT_S_ARATIO_4_3   (0x80)
#define EDID_DRL_CVT_S_ARATIO_16_9  (0x40)
#define EDID_DRL_CVT_S_ARATIO_16_10 (0x20)
#define EDID_DRL_CVT_S_ARATIO_5_4   (0x10)
#define EDID_DRL_CVT_S_ARATIO_15_9  (0x08)

#define EDID_DRL_CVT_P_ARATIO_MASK  (0xE0)
#define EDID_DRL_CVT_P_ARATIO_4_3   (0x00)
#define EDID_DRL_CVT_P_ARATIO_16_9  (0x20)
#define EDID_DRL_CVT_P_ARATIO_16_10 (0x40)
#define EDID_DRL_CVT_P_ARATIO_5_4   (0x60)
#define EDID_DRL_CVT_P_ARATIO_15_9  (0x80)

#define EDID_DRL_CVT_STD_BLANKING   (0x08)
#define EDID_DRL_CVT_RDC_BLANKING   (0x10)

#define EDID_DRL_CVT_HORZ_SHRINK    (0x80)
#define EDID_DRL_CVT_HORZ_STRETCH   (0x40)
#define EDID_DRL_CVT_VERT_SHRINK    (0x20)
#define EDID_DRL_CVT_VERT_STRETCH   (0x10)

// Established timings III descriptor defintion
#define EDID_EST3_640x350_85        (0x80)
#define EDID_EST3_640x400_85        (0x40)
#define EDID_EST3_720x400_85        (0x20)
#define EDID_EST3_640x480_85        (0x10)
#define EDID_EST3_848x480_85        (0x08)
#define EDID_EST3_800x600_85        (0x04)
#define EDID_EST3_1024x768_85       (0x02)
#define EDID_EST3_1152x864_75       (0x01)

#define EDID_EST3_1280x768_60_RB    (0x80)
#define EDID_EST3_1280x768_60       (0x40)
#define EDID_EST3_1280x768_75       (0x20)
#define EDID_EST3_1280x768_85       (0x10)
#define EDID_EST3_1280x960_60       (0x08)
#define EDID_EST3_1280x960_85       (0x04)
#define EDID_EST3_1280x1024_60      (0x02)
#define EDID_EST3_1280x1024_85      (0x01)

#define EDID_EST3_1360x768_60       (0x80)
#define EDID_EST3_1440x900_60_RB    (0x40)
#define EDID_EST3_1440x900_60       (0x20)
#define EDID_EST3_1440x900_75       (0x10)
#define EDID_EST3_1440x900_85       (0x08)
#define EDID_EST3_1400x1050_60_RB   (0x04)
#define EDID_EST3_1400x1050_60      (0x02)
#define EDID_EST3_1400x1050_75      (0x01)

#define EDID_EST3_1400x1050_85      (0x80)
#define EDID_EST3_1680x1050_60_RB   (0x40)
#define EDID_EST3_1680x1050_60      (0x20)
#define EDID_EST3_1680x1050_75      (0x10)
#define EDID_EST3_1680x1050_85      (0x08)
#define EDID_EST3_1600x1200_60      (0x04)
#define EDID_EST3_1600x1200_65      (0x02)
#define EDID_EST3_1600x1200_70      (0x01)

#define EDID_EST3_1600x1200_75      (0x80)
#define EDID_EST3_1600x1200_85      (0x40)
#define EDID_EST3_1792x1344_60      (0x20)
#define EDID_EST3_1792x1344_75      (0x10)
#define EDID_EST3_1856x1392_60      (0x08)
#define EDID_EST3_1856x1392_75      (0x04)
#define EDID_EST3_1920x1200_60_RB   (0x02)
#define EDID_EST3_1920x1200_60      (0x01)

#define EDID_EST3_1920x1200_75      (0x80)
#define EDID_EST3_1920x1200_85      (0x40)
#define EDID_EST3_1920x1440_60      (0x20)
#define EDID_EST3_1920x1440_75      (0x10)

//Video Input Definition
#define EDID_IND_COLOR_MASK         0x70
#define EDID_IND_COLOR_UNDEF        0x00
#define EDID_IND_COLOR_6BIT         0x10
#define EDID_IND_COLOR_8BIT         0x20
#define EDID_IND_COLOR_10BIT        0x30
#define EDID_IND_COLOR_12BIT        0x40
#define EDID_IND_COLOR_14BIT        0x50
#define EDID_IND_COLOR_16BIT        0x60

#define EDID_IND_IF_MASK            0x0F
#define EDID_IND_IF_DVI             0x01
#define EDID_IND_IF_HDMI_A          0x02
#define EDID_IND_IF_HDMI_B          0x03
#define EDID_IND_IF_MDDI            0x04
#define EDID_IND_IF_DISP_PORT       0x05

//Extension Tag numbers
#define EDID_EXT_TAG_CEA            0x02
#define EDID_EXT_TAG_VTB            0x10
#define EDID_EXT_TAG_DI             0x40
#define EDID_EXT_TAG_LS             0x50
#define EDID_EXT_TAG_DPVL           0x60
#define EDID_EXT_TAG_BLOCK_MAP      0xF0
#define EDID_EXT_TAG_MANUFACT       0xFF


int edid_verify(t_edid* edid);
int edid_checksum_gen(t_edid* edid);
void edid_report(t_edid* edid);
void edid_hoi_limit(t_edid* edid);
int edid_write_file(t_edid *edid, char *file);
int edid_read_file(t_edid* edid, char *file);
int edid_compare(t_edid* edid1, t_edid* edid2);
void edid_report_vid_timing(t_edid *edid);

#endif /* EDID_H_ */
