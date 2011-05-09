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
    uint8_t         header[8];          //!< should be "00 ff ff ff ff ff ff 00"
    uint16_t        id_manufacturer;
    uint16_t        id_product_code;
    uint32_t        id_serial_number;
    uint8_t         week;               // offset 16
    uint8_t         year;
    uint8_t         edid_version;
    uint8_t         edid_revision;
    uint8_t         video_input_definition;
    uint8_t         hsize;
    uint8_t         vsize;
    uint8_t         gamma;
    uint8_t         feature;
    uint8_t         color[10];
    uint16_t        timings;            // 35
    uint8_t         timings_reserved;
    uint16_t        std_timings[8];     // 38
    t_edid_timing   detailed_timing[4]; // 54
    uint8_t         extension_count;    // 126
    uint8_t         checksum;
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

int edid_verify(t_edid* edid);
void edid_report(t_edid* edid);
void edid_hoi_limit(t_edid* edid);

#endif /* EDID_H_ */
