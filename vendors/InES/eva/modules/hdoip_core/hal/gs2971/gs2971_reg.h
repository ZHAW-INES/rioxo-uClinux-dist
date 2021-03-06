#ifndef GS2971_REG_H_
#define GS2971_REG_H_

// Video Core Registers
//-------------------------------------------------------
#define GS2971_IOPROC_1                         (0x000)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_IOPROC_2                         (0x001)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ERROR_STAT_1                     (0x002)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ERROR_STAT_2                     (0x003)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_EDH_FLAG_IN                      (0x004)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_EDH_FLAG_OUT                     (0x005)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_DATA_FORMAT_DS1                  (0x006)
//-------------------------------------------------------
#define DATA_FORMAT_DS1_YDATA_MASK              (0x000F)
#define DATA_FORMAT_DS1_YDATA_SHIFT             (0)
#define DATA_FORMAT_DS1_CDATA_MASK              (0x00F0)
#define DATA_FORMAT_DS1_CDATA_SHIFT             (4)
#define DATA_FORMAT_DS1_VD_STD_MASK             (0x3F00)
#define DATA_FORMAT_DS1_VD_STD_SHIFT            (8)

//-------------------------------------------------------
#define GS2971_DATA_FORMAT_DS2                  (0x007)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_IO_CONFIG                        (0x008)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_IO_CONFIG2                       (0x009)
//-------------------------------------------------------
#define IO_CONFIG_DEFAULT                       (0x18E8)

//-------------------------------------------------------
#define GS2971_ANC_CONTROL                      (0x00A)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_LINE_A                       (0x00B)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_LINE_B                       (0x00C)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_TYPE1_AP2                    (0x00F)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_TYPE2_AP2                    (0x010)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_TYPE_3_AP1                   (0x011)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_TYPE_4_AP1                   (0x012)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_TYPE_5_AP1                   (0x013)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_TYPE_1_AP2                   (0x014)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_TYPE_2_AP2                   (0x015)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_TYPE_3_AP2                   (0x016)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_TYPE_4_AP2                   (0x017)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ANC_TYPE_5_AP2                   (0x018)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_VIDEO_FORMAT_352_A_1             (0x019)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_VIDEO_FORMAT_352_B_1             (0x01A)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_VIDEO_FORMAT_352_A_2             (0x01B)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_VIDEO_FORMAT_352_B_2             (0x01C)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_VIDEO_FORMAT_352_INS_A           (0x01D)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_VIDEO_FORMAT_352_INS_B           (0x01E)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_RASTER_STRUC_1                   (0x01F)
//-------------------------------------------------------
#define RASTER_STRUC_1_WORDS_PER_AVTLINE_MASK   (0x3FFF)
#define RASTER_STRUC_1_WORDS_PER_AVTLINE_SHIFT  (0)

//-------------------------------------------------------
#define GS2971_RASTER_STRUC_2                   (0x020)
//-------------------------------------------------------
#define RASTER_STRUC_2_WORDS_PER_LINE_MASK      (0x3FFF)
#define RASTER_STRUC_2_WORDS_PER_LINE_SHIFT     (0)

//-------------------------------------------------------
#define GS2971_RASTER_STRUC_3                   (0x021)
//-------------------------------------------------------
#define RASTER_STRUC_3_LINES_PER_FRAME_MASK     (0x07FF)
#define RASTER_STRUC_3_LINES_PER_FRAME_SHIFT    (0)

//-------------------------------------------------------
#define GS2971_RASTER_STRUC_4                   (0x022)
//-------------------------------------------------------
#define RASTER_STRUC_4_ACTLINE_PER_FIELD_MASK   (0x07FF)
#define RASTER_STRUC_4_ACTLINE_PER_FIELD_SHIFT  (0)
#define RASTER_STRUC_4_INT_PROG                 (0x0800)
#define RASTER_STRUC_4_INT_PROG_SHIFT           (11)
#define RASTER_STRUC_4_STD_LOCK                 (0x1000)
#define RASTER_STRUC_4_M                        (0x2000)
#define RASTER_STRUC_4_RATE_SEL_READBACK_MASK   (0xC000)
#define RASTER_STRUC_4_RATE_SEL_READBACK_SHIFT  (14)
#define RATE_SEL_READBACK_HD                    (0)
#define RATE_SEL_READBACK_SD_1                  (1)
#define RATE_SEL_READBACK_SD_2                  (3)
#define RATE_SEL_READBACK_3G                    (2)
//-------------------------------------------------------
#define GS2971_FLYWHEEL_STATUS                  (0x023)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_RATE_SEL                         (0x024)
//-------------------------------------------------------
#define RATE_SEL_AUTO                           (0x0004)
#define RATE_SEL_TOP_MASK                       (0x0003)
#define RATE_SEL_TOP_SHIFT                      (0)

//-------------------------------------------------------
#define GS2971_TIM_861_FORMAT                   (0x025)
//-------------------------------------------------------
#define TIM_861_FORMAT_ERR                      (0x0040)
#define TIM_861_FORMAT_ID_861_MASK              (0x003F)
#define TIM_861_FORMAT_ID_861_SHIFT             (0)

//-------------------------------------------------------
#define GS2971_TIM_861_CFG                      (0x026)
//-------------------------------------------------------
#define TIM_861_CFG_TRS_861                     (0x0001)
#define TIM_861_CFG_HSYNC_INVERT                (0x0002)
#define TIM_861_CFG_VSYNC_INVERT                (0x0004)

//-------------------------------------------------------
#define GS2971_ERROR_MASK_1                     (0x037)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ERROR_MASK_2                     (0x038)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_ACGEN_CTRL                       (0x039)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_CLK_GEN                          (0x06C)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_IO_DRIVE_STRENGTH                (0x06D)
//-------------------------------------------------------
#define IO_DRIVE_STRENGTH_4_MA                  (0x0000)
#define IO_DRIVE_STRENGTH_6_MA                  (0x0015)
#define IO_DRIVE_STRENGTH_10_MA                 (0x002A)
#define IO_DRIVE_STRENGTH_12_MA                 (0x003F)

//-------------------------------------------------------
#define GS2971_EQ_BYPASS                        (0x073)
//-------------------------------------------------------


// Audio Core Registers
//-------------------------------------------------------
#define GS2971_A_CFG_AUD                        (0x400)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_A_DBN_ERR                        (0x401)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_A_REGEN                          (0x402)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_A_AUD_DET                        (0x403)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_A_CSUM_ERR_DET                   (0x404)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_A_CH_MUTE                        (0x405)
//-------------------------------------------------------
#define A_CH_MUTE_ON                            (0x00FF)
#define A_CH_MUTE_OFF                           (0x0000)

//-------------------------------------------------------
#define GS2971_A_CH_VALID                       (0x406)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_A_INT_ENABLE                     (0x407)
//-------------------------------------------------------
#define A_INT_ENABLE_NOT_LOCKED                 (0x4000)
#define A_INT_ENABLE_NO_VIDEO                   (0x2000)
#define A_INT_ENABLE_CSUM_ERROR                 (0x1000)
#define A_INT_ENABLE_ACS_DET3_4B                (0x0800)
#define A_INT_ENABLE_ACS_DET1_2B                (0x0400)
#define A_INT_ENABLE_ACS_DET3_4A                (0x0200)
#define A_INT_ENABLE_ACS_DET1_2A                (0x0100)
#define A_INT_ENABLE_CTRB_DET                   (0x0080)
#define A_INT_ENABLE_CTRA_DET                   (0x0040)
#define A_INT_ENABLE_DBNB_ERROR                 (0x0020)
#define A_INT_ENABLE_DBNA_ERROR                 (0x0010)
#define A_INT_ENABLE_ADPG4_DET                  (0x0008)
#define A_INT_ENABLE_ADPG3_DET                  (0x0004)
#define A_INT_ENABLE_ADPG2_DET                  (0x0002)
#define A_INT_ENABLE_ADPG1_DET                  (0x0001)

//-------------------------------------------------------
#define GS2971_A_CFG_OUTPUT                     (0x408)
//-------------------------------------------------------
#define A_CFG_OUTPUT_BIT_LENGTH_16              (0xAA00)
#define A_CFG_OUTPUT_BIT_LENGTH_20              (0x5500)
#define A_CFG_OUTPUT_BIT_LENGTH_24              (0x0000)
#define A_CFG_OUTPUT_BIT_LENGTH_AUTO            (0xFF00)
#define A_CFG_OUTPUT_MODE_I2S                   (0x00FF)
#define A_CFG_OUTPUT_MODE_SER_R                 (0x00AA)
#define A_CFG_OUTPUT_MODE_SER_L                 (0x0055)
#define A_CFG_OUTPUT_MODE_AES_EBU               (0x0000)

//-------------------------------------------------------
#define GS2971_A_OUTPUT_SEL_1                   (0x409)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_A_OUTPUT_SEL_2                   (0x40A)
//-------------------------------------------------------


#define GS2971_A_AFNA12                   (0x420)
#define GS2971_A_AFNA34                   (0x421)
#define GS2971_A_RATEA                    (0x422)
#define GS2971_A_ACT_A                    (0x423)
#define GS2971_A_PRIM_AUD_DELAY_1         (0x424)
#define GS2971_A_PRIM_AUD_DELAY_2         (0x425)
#define GS2971_A_PRIM_AUD_DELAY_3         (0x426)
#define GS2971_A_PRIM_AUD_DELAY_4         (0x427)
#define GS2971_A_PRIM_AUD_DELAY_5         (0x428)
#define GS2971_A_PRIM_AUD_DELAY_6         (0x429)
#define GS2971_A_PRIM_AUD_DELAY_7         (0x42A)
#define GS2971_A_PRIM_AUD_DELAY_8         (0x42B)
#define GS2971_A_PRIM_AUD_DELAY_9         (0x42C)
#define GS2971_A_PRIM_AUD_DELAY_10        (0x42D)
#define GS2971_A_PRIM_AUD_DELAY_11        (0x42E)
#define GS2971_A_PRIM_AUD_DELAY_12        (0x42F)
#define GS2971_A_AFNB12                   (0x430)
#define GS2971_A_AFNB34                   (0x431)
#define GS2971_A_RATEB                    (0x432)
#define GS2971_A_ACT_B                    (0x433)
#define GS2971_A_SEC_AUD_DELAY_1          (0x434)
#define GS2971_A_SEC_AUD_DELAY_2          (0x435)
#define GS2971_A_SEC_AUD_DELAY_3          (0x436)
#define GS2971_A_SEC_AUD_DELAY_4          (0x437)
#define GS2971_A_SEC_AUD_DELAY_5          (0x438)
#define GS2971_A_SEC_AUD_DELAY_6          (0x439)
#define GS2971_A_SEC_AUD_DELAY_7          (0x43A)
#define GS2971_A_SEC_AUD_DELAY_8          (0x43B)
#define GS2971_A_SEC_AUD_DELAY_9          (0x43C)
#define GS2971_A_SEC_AUD_DELAY_10         (0x43D)
#define GS2971_A_SEC_AUD_DELAY_11         (0x43E)
#define GS2971_A_SEC_AUD_DELAY_12         (0x43F)
#define GS2971_A_ACSR1_2A_BYTE0_1         (0x440)
#define GS2971_A_ACSR1_2A_BYTE2_3         (0x441)
#define GS2971_A_ACSR1_2A_BYTE4_5         (0x442)
#define GS2971_A_ACSR1_2A_BYTE6_7         (0x443)
#define GS2971_A_ACSR1_2A_BYTE8_9         (0x444)
#define GS2971_A_ACSR1_2A_BYTE10_11       (0x445)
#define GS2971_A_ACSR1_2A_BYTE12_13       (0x446)
#define GS2971_A_ACSR1_2A_BYTE14_15       (0x447)
#define GS2971_A_ACSR1_2A_BYTE16_17       (0x448)
#define GS2971_A_ACSR1_2A_BYTE18_19       (0x449)
#define GS2971_A_ACSR1_2A_BYTE20_21       (0x44A)
#define GS2971_A_ACSR1_2A_BYTE22          (0x44B)

#define GS2971_A_ACSR3_4A_BYTE0_1         (0x450)
#define GS2971_A_ACSR3_4A_BYTE2_3         (0x451)
#define GS2971_A_ACSR3_4A_BYTE4_5         (0x452)
#define GS2971_A_ACSR3_4A_BYTE6_7         (0x453)
#define GS2971_A_ACSR3_4A_BYTE8_9         (0x454)
#define GS2971_A_ACSR3_4A_BYTE10_11       (0x455)
#define GS2971_A_ACSR3_4A_BYTE12_13       (0x456)
#define GS2971_A_ACSR3_4A_BYTE14_15       (0x457)
#define GS2971_A_ACSR3_4A_BYTE16_17       (0x458)
#define GS2971_A_ACSR3_4A_BYTE18_19       (0x459)
#define GS2971_A_ACSR3_4A_BYTE20_21       (0x45A)
#define GS2971_A_ACSR3_4A_BYTE22          (0x45B)

#define GS2971_A_ACSR1_2B_BYTE0_1         (0x460)
#define GS2971_A_ACSR1_2B_BYTE2_3         (0x461)
#define GS2971_A_ACSR1_2B_BYTE4_5         (0x462)
#define GS2971_A_ACSR1_2B_BYTE6_7         (0x463)
#define GS2971_A_ACSR1_2B_BYTE8_9         (0x464)
#define GS2971_A_ACSR1_2B_BYTE10_11       (0x465)
#define GS2971_A_ACSR1_2B_BYTE12_13       (0x466)
#define GS2971_A_ACSR1_2B_BYTE14_15       (0x467)
#define GS2971_A_ACSR1_2B_BYTE16_17       (0x468)
#define GS2971_A_ACSR1_2B_BYTE18_19       (0x469)
#define GS2971_A_ACSR1_2B_BYTE20_21       (0x46A)
#define GS2971_A_ACSR1_2B_BYTE22          (0x46B)

#define GS2971_A_ACSR3_4B_BYTE0_1         (0x470)
#define GS2971_A_ACSR3_4B_BYTE2_3         (0x471)
#define GS2971_A_ACSR3_4B_BYTE4_5         (0x472)
#define GS2971_A_ACSR3_4B_BYTE6_7         (0x473)
#define GS2971_A_ACSR3_4B_BYTE8_9         (0x474)
#define GS2971_A_ACSR3_4B_BYTE10_11       (0x475)
#define GS2971_A_ACSR3_4B_BYTE12_13       (0x476)
#define GS2971_A_ACSR3_4B_BYTE14_15       (0x477)
#define GS2971_A_ACSR3_4B_BYTE16_17       (0x478)
#define GS2971_A_ACSR3_4B_BYTE18_19       (0x479)
#define GS2971_A_ACSR3_4B_BYTE20_21       (0x47A)
#define GS2971_A_ACSR3_4B_BYTE22          (0x47B)

#define GS2971_A_ACSR_BYTE_0              (0x480)
#define GS2971_A_ACSR_BYTE_1              (0x481)
#define GS2971_A_ACSR_BYTE_2              (0x482)
#define GS2971_A_ACSR_BYTE_3              (0x483)
#define GS2971_A_ACSR_BYTE_4              (0x484)
#define GS2971_A_ACSR_BYTE_5              (0x485)
#define GS2971_A_ACSR_BYTE_6              (0x486)
#define GS2971_A_ACSR_BYTE_7              (0x487)
#define GS2971_A_ACSR_BYTE_8              (0x488)
#define GS2971_A_ACSR_BYTE_9              (0x489)
#define GS2971_A_ACSR_BYTE_10             (0x48A)
#define GS2971_A_ACSR_BYTE_11             (0x48B)
#define GS2971_A_ACSR_BYTE_12             (0x48C)
#define GS2971_A_ACSR_BYTE_13             (0x48D)
#define GS2971_A_ACSR_BYTE_14             (0x48E)
#define GS2971_A_ACSR_BYTE_15             (0x48F)
#define GS2971_A_ACSR_BYTE_16             (0x490)
#define GS2971_A_ACSR_BYTE_17             (0x491)
#define GS2971_A_ACSR_BYTE_18             (0x492)
#define GS2971_A_ACSR_BYTE_19             (0x493)
#define GS2971_A_ACSR_BYTE_20             (0x494)
#define GS2971_A_ACSR_BYTE_21             (0x495)
#define GS2971_A_ACSR_BYTE_22             (0x496)

// HD and 3G Audio Core Registers
//-------------------------------------------------------
#define GS2971_B_CFG_AUD                        (0x200)
//-------------------------------------------------------
#define B_CFG_AUD_ECC_OFF                       (0x8000)
#define B_CFG_AUD_ALL_DEL                       (0x4000)
#define B_CFG_AUD_MUTE_ALL                      (0x2000)
#define B_CFG_AUD_ACS_USE_SECOND                (0x1000)
#define B_CFG_AUD_BIT_LENGTH_24                 (0x0000)
#define B_CFG_AUD_BIT_LENGTH_20                 (0x0500)
#define B_CFG_AUD_BIT_LENGTH_16                 (0x0A00)
#define B_CFG_AUD_MODE_AES_EBU                  (0x0000)
#define B_CFG_AUD_MODE_SER_L                    (0x0050)
#define B_CFG_AUD_MODE_SER_R                    (0x00A0)
#define B_CFG_AUD_MODE_I2S                      (0x00F0)
#define B_CFG_AUD_DEFAULT_CHANNEL_SELECT        (0x0004)


//-------------------------------------------------------
#define GS2971_B_ACS_DET                        (0x201)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_B_AUD_DET1                       (0x202)
//-------------------------------------------------------
#define B_AUD_DET1_ADPG1_DET                    (0x002)
#define B_AUD_DET1_ADPG2_DET                    (0x004)
#define B_AUD_DET1_ADPG3_DET                    (0x008)
#define B_AUD_DET1_ADPG4_DET                    (0x010)

//-------------------------------------------------------
#define GS2971_B_AUD_DET2                       (0x203)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_B_REGEN                          (0x204)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_B_CH_MUTE                        (0x205)
//-------------------------------------------------------
#define B_CH_MUTE_ON                            (0x00FF)
#define B_CH_MUTE_OFF                           (0x0000)

//-------------------------------------------------------
#define GS2971_B_CH_VALID                       (0x206)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_B_INT_ENABLE                     (0x207)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_B_CFG_AUD_2                      (0x208)
//-------------------------------------------------------
#define B_CFG_AUD_2_DEFAULT                     (0x0000)
#define B_CFG_AUD_2_SEL_PHASE_SRC               (0x0400)
#define B_CFG_AUD_2_LSB_FIRSTB                  (0x0200)
#define B_CFG_AUD_2_LSB_FIRSTA                  (0x0100)
#define B_CFG_AUD_2_FORCE_M                     (0x0080)
#define B_CFG_AUD_2_FORCE_MEQ1001               (0x0040)
#define B_CFG_AUD_2_IGNORE_PHASE                (0x0020)
#define B_CFG_AUD_2_FORCE_ACLK128               (0x0010)
#define B_CFG_AUD_2_EN_NOT_LOCKED               (0x0008)
#define B_CFG_AUD_2_EN_NO_VIDEO                 (0x0004)
#define B_CFG_AUD_2_EN_NO_PHASEB                (0x0002)
#define B_CFG_AUD_2_EN_NO_PHASEA                (0x0001)

//-------------------------------------------------------
#define GS2971_B_CFG_AUD_3                      (0x209)
//-------------------------------------------------------
#define B_CFG_AUD_3_MISSING_PHASE               (0x0004)
#define B_CFG_AUD_3_NO_PHASEB_DATA              (0x0002)
#define B_CFG_AUD_3_NO_PHASEA_DATA              (0x0001)

//-------------------------------------------------------
#define GS2971_B_OUTPUT_SEL_1                   (0x20A)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2971_B_OUTPUT_SEL_2                   (0x20B)
//-------------------------------------------------------

#define GS2971_B_AFNA                     (0x220)
#define GS2971_B_RATEA                    (0x221)
#define GS2971_B_ACTA                     (0x222)
#define GS2971_B_PRIM_AUD_DELAY_1         (0x223)
#define GS2971_B_PRIM_AUD_DELAY_2         (0x224)
#define GS2971_B_PRIM_AUD_DELAY_3         (0x225)
#define GS2971_B_PRIM_AUD_DELAY_4         (0x226)
#define GS2971_B_PRIM_AUD_DELAY_5         (0x227)
#define GS2971_B_PRIM_AUD_DELAY_6         (0x228)

#define GS2971_B_AFNB                     (0x230)
#define GS2971_B_RATEB                    (0x231)
#define GS2971_B_ACTB                     (0x232)
#define GS2971_B_SEC_AUD_DELAY_1          (0x233)
#define GS2971_B_SEC_AUD_DELAY_2          (0x234)
#define GS2971_B_SEC_AUD_DELAY_3          (0x235)
#define GS2971_B_SEC_AUD_DELAY_4          (0x236)
#define GS2971_B_SEC_AUD_DELAY_5          (0x237)
#define GS2971_B_SEC_AUD_DELAY_6          (0x238)

#define GS2971_B_ACSR1_2A_BYTE0_1         (0x240)
#define GS2971_B_ACSR1_2A_BYTE2_3         (0x241)
#define GS2971_B_ACSR1_2A_BYTE4_5         (0x242)
#define GS2971_B_ACSR1_2A_BYTE6_7         (0x243)
#define GS2971_B_ACSR1_2A_BYTE8_9         (0x244)
#define GS2971_B_ACSR1_2A_BYTE10_11       (0x245)
#define GS2971_B_ACSR1_2A_BYTE12_13       (0x246)
#define GS2971_B_ACSR1_2A_BYTE14_15       (0x247)
#define GS2971_B_ACSR1_2A_BYTE16_17       (0x248)
#define GS2971_B_ACSR1_2A_BYTE18_19       (0x249)
#define GS2971_B_ACSR1_2A_BYTE20_21       (0x24A)
#define GS2971_B_ACSR1_2A_BYTE22          (0x24B)

#define GS2971_B_ACSR3_4A_BYTE0_1         (0x250)
#define GS2971_B_ACSR3_4A_BYTE2_3         (0x251)
#define GS2971_B_ACSR3_4A_BYTE4_5         (0x252)
#define GS2971_B_ACSR3_4A_BYTE6_7         (0x253)
#define GS2971_B_ACSR3_4A_BYTE8_9         (0x254)
#define GS2971_B_ACSR3_4A_BYTE10_11       (0x255)
#define GS2971_B_ACSR3_4A_BYTE12_13       (0x256)
#define GS2971_B_ACSR3_4A_BYTE14_15       (0x257)
#define GS2971_B_ACSR3_4A_BYTE16_17       (0x258)
#define GS2971_B_ACSR3_4A_BYTE18_19       (0x259)
#define GS2971_B_ACSR3_4A_BYTE20_21       (0x25A)
#define GS2971_B_ACSR3_4A_BYTE22          (0x25B)

#define GS2971_B_ACSR1_2B_BYTE0_1         (0x260)
#define GS2971_B_ACSR1_2B_BYTE2_3         (0x261)
#define GS2971_B_ACSR1_2B_BYTE4_5         (0x262)
#define GS2971_B_ACSR1_2B_BYTE6_7         (0x263)
#define GS2971_B_ACSR1_2B_BYTE8_9         (0x264)
#define GS2971_B_ACSR1_2B_BYTE10_11       (0x265)
#define GS2971_B_ACSR1_2B_BYTE12_13       (0x266)
#define GS2971_B_ACSR1_2B_BYTE14_15       (0x267)
#define GS2971_B_ACSR1_2B_BYTE16_17       (0x268)
#define GS2971_B_ACSR1_2B_BYTE18_19       (0x269)
#define GS2971_B_ACSR1_2B_BYTE20_21       (0x26A)
#define GS2971_B_ACSR1_2B_BYTE22          (0x26B)

#define GS2971_B_ACSR3_4B_BYTE0_1         (0x270)
#define GS2971_B_ACSR3_4B_BYTE2_3         (0x271)
#define GS2971_B_ACSR3_4B_BYTE4_5         (0x272)
#define GS2971_B_ACSR3_4B_BYTE6_7         (0x273)
#define GS2971_B_ACSR3_4B_BYTE8_9         (0x274)
#define GS2971_B_ACSR3_4B_BYTE10_11       (0x275)
#define GS2971_B_ACSR3_4B_BYTE12_13       (0x276)
#define GS2971_B_ACSR3_4B_BYTE14_15       (0x277)
#define GS2971_B_ACSR3_4B_BYTE16_17       (0x278)
#define GS2971_B_ACSR3_4B_BYTE18_19       (0x279)
#define GS2971_B_ACSR3_4B_BYTE20_21       (0x27A)
#define GS2971_B_ACSR3_4B_BYTE22          (0x27B)

#define GS2971_B_ACSR_BYTE_0              (0x280)
#define GS2971_B_ACSR_BYTE_1              (0x281)
#define GS2971_B_ACSR_BYTE_2              (0x282)
#define GS2971_B_ACSR_BYTE_3              (0x283)
#define GS2971_B_ACSR_BYTE_4              (0x284)
#define GS2971_B_ACSR_BYTE_5              (0x285)
#define GS2971_B_ACSR_BYTE_6              (0x286)
#define GS2971_B_ACSR_BYTE_7              (0x287)
#define GS2971_B_ACSR_BYTE_8              (0x288)
#define GS2971_B_ACSR_BYTE_9              (0x289)
#define GS2971_B_ACSR_BYTE_10             (0x28A)
#define GS2971_B_ACSR_BYTE_11             (0x28B)
#define GS2971_B_ACSR_BYTE_12             (0x28C)
#define GS2971_B_ACSR_BYTE_13             (0x28D)
#define GS2971_B_ACSR_BYTE_14             (0x28E)
#define GS2971_B_ACSR_BYTE_15             (0x28F)
#define GS2971_B_ACSR_BYTE_16             (0x290)
#define GS2971_B_ACSR_BYTE_17             (0x291)
#define GS2971_B_ACSR_BYTE_18             (0x292)
#define GS2971_B_ACSR_BYTE_19             (0x293)
#define GS2971_B_ACSR_BYTE_20             (0x294)
#define GS2971_B_ACSR_BYTE_21             (0x295)
#define GS2971_B_ACSR_BYTE_22             (0x296)

#endif /*GS2971_REG_H_*/
