//! \file vio_reg.h
#ifndef VIO_REG_H_
#define VIO_REG_H_

/** register byte offset
 * 
 */
// Base Mux
#define VIO_OFF_CONFIG              (0x0000)
#define VIO_OFF_STATUS              (0x0004)
#define VIO_OFF_MUX                 (0x0008)
#define VIO_OFF_HSPLIT              (0x000c)
#define VIO_OFF_FIN                 (0x0010)
#define VIO_OFF_FOUT                (0x0014)
#define VIO_OFF_PHASESTEP           (0x0018)
#define VIO_OFF_ERROR               (0x001c)
// Timing Generator
#define VIO_OFF_TG_SIZE             (0x0020)
#define VIO_OFF_TG_HSYNC            (0x0024)
#define VIO_OFF_TG_VSYNC            (0x0028)
#define VIO_OFF_TG_HAVID            (0x002c)
#define VIO_OFF_TG_VAVID            (0x0030)
#define VIO_OFF_TG_DURATION         (0x0034)
#define VIO_OFF_TG_THRESHOLD        (0x0038)
#define VIO_OFF_TG_SCALE            (0x003c)
#define VIO_OFF_TG_CAPTURE          (0x0040)
// Statistic
#define VIO_OFF_VID_IN_CNT          (0x0044)
#define VIO_OFF_VID_OUT_CNT         (0x0048)
#define VIO_OFF_ST_IN_CNT           (0x004c)
#define VIO_OFF_ST_OUT_CNT          (0x0050)
#define VIO_OFF_FVSYNC_CNT          (0x0058)
// Frame length
#define VIO_OFF_FRAME_LENGTH        (0x0054)
// PLL
#define VIO_OFF_PLL_CONFIG          (0x0100)
#define VIO_OFF_PLL_STATUS          (0x0104)
#define VIO_OFF_PLL_TYPEPARAM       (0x0108)
#define VIO_OFF_PLL_DATA            (0x010c)
#define VIO_OFF_PLL_SHIFT           (0x0110)
#define VIO_OFF_PLL_PCS             (0x0114)
// input pixel transformation
#define VIO_OFF_INPUT_PT            (0x0140)
// overlay text
#define VIO_OFF_OSD_SCREEN          (0x8000)
#define VIO_OFF_OSD_CHAR            (0xc000)
#define VIO_OFF_OSD_CONFIG          (0xe020)
#define VIO_OFF_OSD_SCREEN_WIDTH    (0xe024)
#define VIO_OFF_OSD_SCREEN_HEIGHT   (0xe026)
#define VIO_OFF_OSD_CHAR_WIDTH      (0xe028)
#define VIO_OFF_OSD_CHAR_HEIGHT     (0xe02a)
// output pixel transformation
#define VIO_OFF_OUTPUT_PT           (0xe080)
// Timing measurement
#define VIO_OFF_TM_V                (0x0160)
#define VIO_OFF_TM_H                (0x0162)
#define VIO_OFF_TM_VP               (0x0164)
#define VIO_OFF_TM_HP               (0x0166)
#define VIO_OFF_TM_VB               (0x0168)
#define VIO_OFF_TM_HB               (0x016a)
#define VIO_OFF_TM_VA               (0x016c)
#define VIO_OFF_TM_HA               (0x016e)


/** pixel transformation byte offset
 * 
 */
#define VIO_OFF_PP_M11              (0x0000)
#define VIO_OFF_PP_M12              (0x0002)
#define VIO_OFF_PP_M13              (0x0004)
#define VIO_OFF_PP_M21              (0x0006)
#define VIO_OFF_PP_M22              (0x0008)
#define VIO_OFF_PP_M23              (0x000a)
#define VIO_OFF_PP_M31              (0x000c)
#define VIO_OFF_PP_M32              (0x000e)
#define VIO_OFF_PP_M33              (0x0010)
#define VIO_OFF_PP_V1               (0x0012)
#define VIO_OFF_PP_V2               (0x0014)
#define VIO_OFF_PP_V3               (0x0016)
#define VIO_OFF_PP_CFG              (0x0018)
 

/** register value
 * 
 */
// VIO_OFF_CONFIG
#define VIO_CFG_RUN                 (1<<0)
#define VIO_CFG_VIN                 (1<<1)
#define VIO_CFG_VOUT                (1<<2)
#define VIO_CFG_OVERLAY             (1<<3)
#define VIO_CFG_PLL_CTRL            (1<<4)
#define VIO_CFG_PLAIN_ENC           (1<<5)
#define VIO_CFG_PLAIN_DEC           (1<<6)
#define VIO_CFG_PLL_RUN             (1<<7)
#define VIO_CFG_HPD                 (1<<8)
#define VIO_CFG_PLL_SW              (1<<9)
#define VIO_CFG_SCOMM5              (1<<10)
#define VIO_CFG_SS_BP               (1<<11)
#define VIO_CFG_MODE                (3<<12)
#define VIO_CFG_MODE_SHIFT          (12)
#define VIO_CFG_MODE_1C             (0<<12)
#define VIO_CFG_MODE_2C             (1<<12)
#define VIO_CFG_MODE_3C             (2<<12)
#define VIO_CFG_MODE_4C             (3<<12)
#define VIO_CFG_ADV_ENC             (1<<14)
#define VIO_CFG_ADV_DEC             (1<<15)
#define VIO_CFG_INTERLACE           (1<<16)
#define VIO_CFG_WAITVS              (1<<17)
#define VIO_CFG_SCOMM5_HW           (1<<18)
#define VIO_CFG_VIN_TIMING          (1<<20)
#define VIO_CFG_TG_VSEL             (1<<21)
// VIO_OFF_STATUS
#define VIO_STA_IN_CLOCK_EXT        (1<<0)
#define VIO_STA_IN_CLKBAD_BASE      (1<<1)
#define VIO_STA_IN_CLKBAD_VIDEO_RX  (1<<2)
#define VIO_STA_INSYNC              (1<<3)
#define VIO_STA_JDATA0_SYNC_LOST    (1<<4)
#define VIO_STA_JDATA1_SYNC_LOST    (1<<5)
#define VIO_STA_JDATA2_SYNC_LOST    (1<<6)
#define VIO_STA_JDATA3_SYNC_LOST    (1<<7)
#define VIO_STA_PLL_LOCKED          (1<<8)
#define VIO_STA_PLL_LOST            (1<<9)
// VIO_OFF_MUX
#define VIO_MUX_VOUT_MASK           (0x03)
#define VIO_MUX_VOUT_LOOP           (0x00)
#define VIO_MUX_VOUT_PLAIN          (0x01)
#define VIO_MUX_VOUT_ADV212         (0x02)
#define VIO_MUX_VOUT_DEBUG          (0x03)
#define VIO_MUX_STIN_MASK           (0x0c)
#define VIO_MUX_STIN_LOOP           (0x00)
#define VIO_MUX_STIN_PLAIN          (0x04)
#define VIO_MUX_STIN_ADV212         (0x08)
#define VIO_MUX_PLLC_MASK           (0x30)
#define VIO_MUX_PLLC_FREE           (0x00)
#define VIO_MUX_PLLC_SW             (0x10)
#define VIO_MUX_PLLC_TG             (0x20)
#define VIO_MUX_PROC_MASK           (0x40)
#define VIO_MUX_PROC_ETH			(0x00)
#define VIO_MUX_PROC_VRP			(0x40)

// VIO_OFF_PLL_CONFIG
#define VIO_PLL_CFG_READ            (0x01)
#define VIO_PLL_CFG_WRITE           (0x02)
#define VIO_PLL_CFG_RECONFIG        (0x04)
// VIO_OFF_PLL_STATUS
#define VIO_PLL_STA_BUSY            (0x01)
#define VIO_PLL_STA_DRIFT_MASK      (0x7e)
#define VIO_PLL_STA_DRIFT_SHIFT     (1)
#define VIO_PLL_STA_DRIFT_SIGN      (0x40)
#define VIO_PLL_STA_ACTIVE          (0x80)
// VIO_OFF_PLL_TYPEPARAM
#define VIO_PLL_TYPE_N              (0x00)
#define VIO_PLL_TYPE_M              (0x08)
#define VIO_PLL_TYPE_CPLF           (0x10)
#define VIO_PLL_TYPE_VCO            (0x18)
#define VIO_PLL_TYPE_C0             (0x20)
#define VIO_PLL_TYPE_C1             (0x28)
#define VIO_PLL_TYPE_C2             (0x30)
#define VIO_PLL_TYPE_C3             (0x38)
#define VIO_PLL_HIGHCOUNT           (0x00)
#define VIO_PLL_LOWCOUNT            (0x01)
#define VIO_PLL_BYPASS              (0x04)
#define VIO_PLL_MODE_ODD            (0x05)
#define VIO_PLL_NOMINAL             (0x07)
#define VIO_PLL_POSTSCALE           (0x00)
#define VIO_PLL_CP_CURRENT          (0x00)
#define VIO_PLL_CP_UNUSED           (0x05)
#define VIO_PLL_LF_UNUSED           (0x04)
#define VIO_PLL_LF_RESISTOR         (0x01)
#define VIO_PLL_LF_CAPACITANCE      (0x05)
// VIO_PLL_SHIFT
#define VIO_PLL_SHIFT_MASK          (0x0ff)
#define VIO_PLL_SHIFT_UP            (0x000)
#define VIO_PLL_SHIFT_DOWN          (0x100)
#define VIO_PLL_SHIFT_EN            (0x200)
// VIO_OFF_PLL_PCS
#define VIO_PLL_PCS_0               (2)
#define VIO_PLL_PCS_1               (3)
#define VIO_PLL_PCS_2               (4)
#define VIO_PLL_PCS_3               (5)
// VIO_OFF_PP_CFG
#define VIO_PP_POL_MASK             (0x03)
#define VIO_PP_HPOL_P               (0x00)
#define VIO_PP_HPOL_N               (0x01)
#define VIO_PP_VPOL_P               (0x00)
#define VIO_PP_VPOL_N               (0x02)
#define VIO_PP_DOWN					(1<<2)
#define VIO_PP_UP					(1<<3)
#define VIO_PP_DFIRST				(1<<4)
#define VIO_PP_UFIRST				(1<<5)

// VIO_OFF_OSD_CONFIG
#define VIO_OSD_CFG_ACTIVE          (1)

#endif /*VIO_REG_H_*/
