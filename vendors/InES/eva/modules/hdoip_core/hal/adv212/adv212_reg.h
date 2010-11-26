#ifndef ADV212_REG_H_
#define ADV212_REG_H_

// Address space
#define ADV212_SIZE             (0x40)

// Direct Registers
#define ADV212_PIXEL            (0x00)
#define ADV212_CODE             (0x04)
#define ADV212_ATTR             (0x08)
#define ADV212_CMDSTA           (0x10)
#define ADV212_EIRQIE           (0x14)
#define ADV212_EIRQFLG          (0x18)
#define ADV212_SWFLAG           (0x1c)
#define ADV212_BUSMODE          (0x20)
#define ADV212_MMODE            (0x24)
#define ADV212_STAGE            (0x28)
#define ADV212_IADDR            (0x2c)
#define ADV212_IDATA            (0x30)
#define ADV212_BOOT             (0x34)
#define ADV212_PLL_HI           (0x38)
#define ADV212_PLL_LO           (0x3c)

// Indirect Address MAP
#define ADV212_FIRMWARE_BASE    (0x00050000)
#define ADV212_PARAMETER_BASE   (0x00057f00)
#define ADV212_REGISTER_BASE    (0xffff0000)

// Parameter (Offset to ADV212_PARAMETER_BASE)
// Note that the documentation talks about BIG-ENDIAN Offset
// As the NIOS II is little endian -> offset is swapped: 3<->0; 2<->1  
// Note that 8/16 Bit arrays cannot be accessed by base+offset 
#define ADV212_ENC_VFORMAT      (0x03)      // 0x00
#define ADV212_ENC_PREC         (0x02)      // 0x01
#define ADV212_ENC_XFORMLEV     (0x01)      // 0x02
#define ADV212_ENC_UNI          (0x00)      // 0x03
#define ADV212_ENC_CBSIZE       (0x07)      // 0x04
#define ADV212_ENC_WKERNEL      (0x06)      // 0x05
#define ADV212_ENC_STALLPAR     (0x05)      // 0x06
#define ADV212_ENC_ATTRTYPE     (0x04)      // 0x07
#define ADV212_ENC_RCTYPE       (0x0b)      // 0x08
#define ADV212_ENC_RCVAL        (0x08)      // 0x09
#define ADV212_ENC_J2KPROG      (0x0f)      // 0x0c
#define ADV212_ENC_PICFG        (0x0e)      // 0x0d
#define ADV212_ENC_QFACT        (0x0d)      // 0x0e
#define ADV212_ENC_COD_STYLE    (0x0c)      // 0x0f
#define ADV212_ENC_LOAD_SS      (0x35)      // 0x36    
#define ADV212_ENC_LOAD_VW      (0x34)      // 0x37

#define ADV212_DEC_VFORMAT      (0x03)      // 0x00
#define ADV212_DEC_PREC         (0x02)      // 0x01
#define ADV212_DEC_UNI          (0x00)      // 0x03
#define ADV212_DEC_PICFG        (0x0a)      // 0x09
#define ADV212_DEC_DRES         (0x09)      // 0x0a
#define ADV212_DEC_COD_STYLE    (0x08)      // 0x0b
    
// Register
#define ADV212_PMODE1           (0xffff0400)
#define ADV212_CMP_CNT_STATUS   (0xffff0404)
#define ADV212_LINE_CNT_STATUS  (0xffff0408)
#define ADV212_XTOT             (0xffff040c)
#define ADV212_YTOT             (0xffff0410)
#define ADV212_F0_START         (0xffff0414)
#define ADV212_F1_START         (0xffff0418)
#define ADV212_V0_START         (0xffff041c)
#define ADV212_V1_START         (0xffff0420)
#define ADV212_V0_END           (0xffff0424)
#define ADV212_V1_END           (0xffff0428)
#define ADV212_PIXEL_START      (0xffff042c)
#define ADV212_PIXEL_STOP       (0xffff0430)
#define ADV212_MS_CNT_DEL       (0xffff0440)
#define ADV212_PMODE2           (0xffff0448)
#define ADV212_VMODE            (0xffff044c)
#define ADV212_V0_REGION_ST     (0xffff0450)
#define ADV212_V1_REGION_ST     (0xffff0454)
#define ADV212_V0_REGION_END    (0xffff0458)
#define ADV212_V1_REGION_END    (0xffff045c)
#define ADV212_PIXEL_START_REF  (0xffff0460)
#define ADV212_PIXEL_END_REF    (0xffff0464)
#define ADV212_EDMOD0           (0xffff1408)
#define ADV212_EDMOD1           (0xffff140c)
#define ADV212_FFTHRP           (0xffff1410)
#define ADV212_FFTHRC           (0xffff141c)
#define ADV212_FFTHRA           (0xffff1420)
#define ADV212_HWREV            (0xffff14f4)


/** Direct Register Constant
 */

// EIRQIE & EIRQFLG
#define ADV212_EIRQ_PFTH        (0x0001)
#define ADV212_EIRQ_CFTH        (0x0002)
#define ADV212_EIRQ_AFTH        (0x0004)
#define ADV212_EIRQ_PFERR       (0x0010)
#define ADV212_EIRQ_CFERR       (0x0020)
#define ADV212_EIRQ_AFERR       (0x0040)
#define ADV212_EIRQ_SWIRQ0      (0x0400)
#define ADV212_EIRQ_SWIRQ1      (0x0800)
#define ADV212_EIRQ_SWIRQ2      (0x1000)
#define ADV212_EIRQ_ALL         (0x1c77)
// BUSMODE
#define ADV212_HWIDTH_16        (0x0001)
#define ADV212_HWIDTH_32        (0x0002)
#define ADV212_DWIDTH_8         (0x0000)
#define ADV212_DWIDTH_16        (0x0004)
#define ADV212_DWIDTH_32        (0x0008)
#define ADV212_BCFG_NORMAL      (0x0000)
#define ADV212_BCFG_JDATA       (0x0010)
#define ADV212_BCFG_RAW         (0x0020)
#define ADV212_BCFG_JDATA_RAW   (0x0030)
// MMODE
#define ADV212_IWIDTH_16        (0x0001)
#define ADV212_IWIDTH_32        (0x0002)
#define ADV212_IAUTOSTP_8       (0x0000)
#define ADV212_IAUTOSTP_16      (0x0004)
#define ADV212_IAUTOSTP_32      (0x0008)
#define ADV212_IAUTOMOD_INC     (0x0000)
#define ADV212_IAUTOMOD_DEC     (0x0010)
#define ADV212_MAP_FULL         (0x0000)
#define ADV212_MAP_CONTROL      (0x0020)
// BOOT
#define ADV212_NO_BOOT_HOST     (0x000a)
#define ADV212_COPROCESSOR_BOOT (0x000d)
#define ADV212_HARDRST          (0x0040)
#define ADV212_SOFTRST          (0x0080)
// PLL_HI
#define ADV212_PLL_PDN          (0x0001)
#define ADV212_PLL_BYPASS       (0x0002)
#define ADV212_PLL_HCLKD        (0x0008)
#define ADV212_PLL_ACK_OD       (0x0010)
// PLL_LO
#define ADV212_PLL_MUL_MASK     (0x001f)
#define ADV212_PLL_MUL_SHIFT    (0)
#define ADV212_PLL_LFB          (0x0020)
#define ADV212_PLL_IPD          (0x0080)
// SWFLAG (return)
#define ADV212_SWFLAG_ENCODE    (0xff82)
#define ADV212_SWFLAG_DECODE    (0xffa2)


/** Indirect Register
 */

// PMODE1
#define ADV212_PMODE1_PFMT_V_SINGLE         (0x04)
#define ADV212_PMODE1_PFMT_V_INTERLEAVED_CY (0x05)
#define ADV212_PMODE1_PFMT_V_INTERLEAVED_C  (0x07)
#define ADV212_PMODE1_PFMT_H_4X8_SINGLE     (0x14)
#define ADV212_PMODE1_PFMT_H_4X8_YCBYCR     (0x15)
#define ADV212_PMODE1_PFMT_H_4X8_YYCBCR     (0x16)
#define ADV212_PMODE1_PFMT_H_4X8_CBCRCBCR   (0x17)
#define ADV212_PMODE1_PFMT_H_2X16_SINGLE    (0x18)
#define ADV212_PMODE1_PFMT_H_2X16_YCB_YCR   (0x19)
#define ADV212_PMODE1_PFMT_H_2X16_YY_CBCR   (0x1a)
#define ADV212_PMODE1_PFMT_H_2X16_CBCR      (0x1b)
#define ADV212_PMODE1_PREC_8BIT             (0x000)
#define ADV212_PMODE1_PREC_10BIT            (0x100)
#define ADV212_PMODE1_PREC_12BIT            (0x200)
#define ADV212_PMODE1_PREC_14BIT            (0x300)
#define ADV212_PMODE1_PREC_16BIT            (0x400)
// VMODE
#define ADV212_VMODE_MASTER                 (0x0001)
#define ADV212_VMODE_SLAVE                  (0x0000)
#define ADV212_VMODE_ENCODE                 (0x0002)
#define ADV212_VMODE_DECODE                 (0x0000)
#define ADV212_VMODE_HVF                    (0x0004)
#define ADV212_VMODE_EAV_SAV                (0x0000)
#define ADV212_VMODE_PRGRSV_SCN             (0x0080)
// EDMOD0
#define ADV212_EDMOD0_DMEN0                 (0x0001)
#define ADV212_EDMOD0_DMSEL0_PIXEL          (0x0000)
#define ADV212_EDMOD0_DMSEL0_COMPRESSED     (0x0002)
#define ADV212_EDMOD0_DMSEL0_ATTRIBUTE      (0x0004)
#define ADV212_EDMOD0_DMMOD0_CS             (0x0000)
#define ADV212_EDMOD0_DMMOD0_SINGLE         (0x0008)
#define ADV212_EDMOD0_DMMOD0_BURST          (0x0010)
#define ADV212_EDMOD0_DMMOD0_JDATA          (0x0018)
#define ADV212_EDMOD0_DMMOD0_SINGLE_FLYBY   (0x0028)
#define ADV212_EDMOD0_DMMOD0_BURST_FLYBY    (0x0030)
#define ADV212_EDMOD0_DMBL0_8               (0x0000)
#define ADV212_EDMOD0_DMBL0_16              (0x0040)
#define ADV212_EDMOD0_DMBL0_32              (0x0080)
#define ADV212_EDMOD0_DMBL0_64              (0x00c0)
#define ADV212_EDMOD0_DMBL0_128             (0x0100)
#define ADV212_EDMOD0_DMBL0_512             (0x0180)
#define ADV212_EDMOD0_DMBL0_1024            (0x01c0)
#define ADV212_EDMOD0_DR0POL_P              (0x0200)
#define ADV212_EDMOD0_DR0POL_N              (0x0000)
#define ADV212_EDMOD0_DA0POL_P              (0x0400)
#define ADV212_EDMOD0_DA0POL_N              (0x0000)
// EDMOD1
#define ADV212_EDMOD1_DMEN1                 (0x0001)
#define ADV212_EDMOD1_DMSEL1_PIXEL          (0x0000)
#define ADV212_EDMOD1_DMSEL1_COMPRESSED     (0x0002)
#define ADV212_EDMOD1_DMSEL1_ATTRIBUTE      (0x0004)
#define ADV212_EDMOD1_DMMOD1_CS             (0x0000)
#define ADV212_EDMOD1_DMMOD1_SINGLE         (0x0008)
#define ADV212_EDMOD1_DMMOD1_BURST          (0x0010)
#define ADV212_EDMOD1_DMMOD1_SINGLE_FLYBY   (0x0028)
#define ADV212_EDMOD1_DMMOD1_BURST_FLYBY    (0x0030)
#define ADV212_EDMOD1_DMBL1_8               (0x0000)
#define ADV212_EDMOD1_DMBL1_16              (0x0040)
#define ADV212_EDMOD1_DMBL1_32              (0x0080)
#define ADV212_EDMOD1_DMBL1_64              (0x00c0)
#define ADV212_EDMOD1_DMBL1_128             (0x0100)
#define ADV212_EDMOD1_DMBL1_512             (0x0180)
#define ADV212_EDMOD1_DMBL1_1024            (0x01c0)
// PMODE2
#define ADV212_PMODE2_VCLK_P                (0x01)
#define ADV212_PMODE2_VSYNC_P               (0x02)
#define ADV212_PMODE2_HSYNC_P               (0x04)
#define ADV212_PMODE2_FIELD_P               (0x08)
#define ADV212_PMODE2_UNIPOLAR_Y            (0x10)
#define ADV212_PMODE2_UNIPOLAR_C            (0x20)
#define ADV212_PMODE2_VCLK_N                (0x00)
#define ADV212_PMODE2_VSYNC_N               (0x00)
#define ADV212_PMODE2_HSYNC_N               (0x00)
#define ADV212_PMODE2_FIELD_N               (0x00)
#define ADV212_PMODE2_BIPOLAR_Y             (0x00)
#define ADV212_PMODE2_BIPOLAR_C             (0x00)

/** Encoding Constant
 */

// VFORMAT
#define ADV212_VFORMAT_NTSC422  (0)
#define ADV212_VFORMAT_PAL422   (1)
#define ADV212_VFORMAT_1080i60l (2)
#define ADV212_VFORMAT_1080i60c (3)
#define ADV212_VFORMAT_CUSTOM   (4)
#define ADV212_VFORMAT_720p60l  (5)
#define ADV212_VFORMAT_720p60c  (6)
#define ADV212_VFORMAT_NTSCDINT (8)
#define ADV212_VFORMAT_PALDINT  (9)
// PREC 
#define ADV212_PREC_8BIT        (0)
#define ADV212_PREC_10BIT       (1)
#define ADV212_PREC_12BIT       (2)
// XFORMLEV
#define ADV212_XFORMLEV_1       (1)
#define ADV212_XFORMLEV_2       (2)
#define ADV212_XFORMLEV_3       (3)
#define ADV212_XFORMLEV_4       (4)
#define ADV212_XFORMLEV_5       (5)
#define ADV212_XFORMLEV_6       (6)
// UNI (Bit)
#define ADV212_UNI_BIPOLAR_C    (0)
#define ADV212_UNI_BIPOLAR_Y    (0)
#define ADV212_UNI_UNIPOLAR_C   (1)
#define ADV212_UNI_UNIPOLAR_Y   (2)
// CBSIZE
#define ADV212_CBSIZE_32x32     (0)
#define ADV212_CBSIZE_64x32     (1)
#define ADV212_CBSIZE_64x64     (2)
#define ADV212_CBSIZE_128x32    (3)
// WKERNEL
#define ADV212_IRREVERSIBLE_9x7 (0)
#define ADV212_REVERSIBLE_5x3   (1)
#define ADV212_IRREVERSIBLE_5x3 (2)
// STALLPAR
#define ADV212_STALLPAR_NONE    (0)
// ATTRTYPE
#define ADV212_ATTRTYPE_NO      (0)
#define ADV212_ATTRTYPE_ADV     (1)
#define ADV212_ATTRTYPE_LOG2    (2)
#define ADV212_ATTRTYPE_LOG2ADJ (3)
#define ADV212_ATTRTYPE_AF      (4)
#define ADV212_ATTRTYPE_AFLEN   (5)
// RCTYPE
#define ADV212_RCTYPE_NOTRUNC   (0)
#define ADV212_RCTYPE_IMGSIZE   (1)
#define ADV212_RCTYPE_QUALITY   (2)
#define ADV212_RCTYPE_MLSIZE    (3)
#define ADV212_RCTYPE_MLQUALITY (4)
// J2KPROG
#define ADV212_J2KPROG_LRCP     (0)
#define ADV212_J2KPROG_RLCP     (1)
#define ADV212_J2KPROG_RPCL     (2)
#define ADV212_J2KPROG_PCRL     (3)
#define ADV212_J2KPROG_CPRL     (4)
// PICFG (Bit)
#define ADV212_PICFG_VCLK_P     (0x01)
#define ADV212_PICFG_VSYNC_P    (0x02)
#define ADV212_PICFG_HSYNC_P    (0x04)
#define ADV212_PICFG_FIELD_P    (0x08)
#define ADV212_PICFG_VCLK_N     (0x00)
#define ADV212_PICFG_VSYNC_N    (0x00)
#define ADV212_PICFG_HSYNC_N    (0x00)
#define ADV212_PICFG_FIELD_N    (0x00)
#define ADV212_PICFG_HVF        (0x40)
#define ADV212_PICFG_EAV_SAV    (0x00)
// QFACT
#define ADV212_QFACT_1          (0)
// COD_STYLE (Bit)
#define ADV212_COD_PPT          (0x80)
#define ADV212_COD_PLT          (0x40)
#define ADV212_COD_SOP          (0x20)
#define ADV212_COD_EPH          (0x10)
#define ADV212_COD_FORMAT_ADV   (0x00)
#define ADV212_COD_FORMAT_J2C   (0x01)
#define ADV212_COD_FORMAT_JP2   (0x02)
#define ADV212_COD_FORMAT_JP2g  (0x03)
// DRES
#define ADV212_DRES_KEEP        (0x00)
#define ADV212_DRES_ALL         (0x80)
#define ADV212_DRES_DIV_4       (0x81)
#define ADV212_DRES_DIV_16      (0x82)
#define ADV212_DRES_DIV_64      (0x83)
#define ADV212_DRES_DIV_256     (0x84)
#define ADV212_DRES_DIV_1024    (0x85)
#define ADV212_DRES_DIV_4096    (0x86)

#endif /*ADV212_REG_H_*/
