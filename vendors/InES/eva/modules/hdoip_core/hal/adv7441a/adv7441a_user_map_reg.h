
#ifndef ADV7441A_USER_MAP_REG_H
#define ADV7441A_USER_MAP_REG_H

#define	ADV7441A_REG_INPUT_CONTROL                               0x00

#define	ADV7441A_REG_VIDEO_SELECTION                             0x01
#define	ADV7441A_BIT_ENHSPLL                                     0x40
#define	ADV7441A_BIT_BETACAM                                     0x20
#define	ADV7441A_BIT_ENVSPROC                                    0x08

#define	ADV7441A_REG_OUTPUT_CONTROL                              0x03
#define	ADV7441A_BIT_VBI_EN                                      0x80
#define	ADV7441A_BIT_TOD                                         0x40
#define	ADV7441A_BIT_SD_DUP_AV                                   0x01

#define	ADV7441A_REG_EXTENDED_OUTPUT_CONTROL                     0x04
#define	ADV7441A_BIT_BT656_4                                     0x80
#define	ADV7441A_BIT_INT2_EN                                     0x10
#define	ADV7441A_BIT_TIM_OE                                      0x08
#define	ADV7441A_BIT_BL_C_VBI                                    0x04
#define	ADV7441A_BIT_EN_SFL_PIN                                  0x02
#define	ADV7441A_BIT_RANGE                                       0x01

#define	ADV7441A_REG_PRIMARY_MODE                                0x05

#define	ADV7441A_REG_VIDEO_STANDARD                              0x06

#define	ADV7441A_REG_AUTODETECT_ENABLE                           0x07
#define	ADV7441A_BIT_AD_SEC525_EN                                0x80
#define	ADV7441A_BIT_AD_SECAM_EN                                 0x40
#define	ADV7441A_BIT_AD_N443_EN                                  0x20
#define	ADV7441A_BIT_AD_P60_EN                                   0x10
#define	ADV7441A_BIT_AD_PALN_EN                                  0x08
#define	ADV7441A_BIT_AD_PALM_EN                                  0x04
#define	ADV7441A_BIT_AD_NTSC_EN                                  0x02
#define	ADV7441A_BIT_AD_PAL_EN                                   0x01

#define	ADV7441A_REG_CONTRAST                                    0x08

#define	ADV7441A_REG_BRIGHTNESS                                  0x0A

#define	ADV7441A_REG_HUE                                         0x0B

#define	ADV7441A_REG_DEFAULT_VALUE_Y                             0x0C
#define	ADV7441A_BIT_DEF_VAL_AUTO_EN                             0x02
#define	ADV7441A_BIT_DEF_VAL_EN                                  0x01

#define	ADV7441A_REG_DEFAULT_VALUE_C                             0x0D

#define	ADV7441A_REG_USS2_SLAVE_ADDRESS                          0x0E

#define	ADV7441A_REG_POWER_MANAGEMENT                            0x0F
#define	ADV7441A_BIT_RES                                         0x80
#define	ADV7441A_BIT_PWRDN                                       0x20
#define	ADV7441A_BIT_PWRSAV                                      0x10
#define	ADV7441A_BIT_CP_PWRDN                                    0x08
#define	ADV7441A_BIT_PDBP                                        0x04
#define	ADV7441A_BIT_FB_PWRDN                                    0x02

#define	ADV7441A_REG_STATUS_1                                    0x10
#define	ADV7441A_BIT_COL_KILL                                    0x80
#define	ADV7441A_BIT_FOLLOW_PW                                   0x08
#define	ADV7441A_BIT_FSC_LOCK                                    0x04
#define	ADV7441A_BIT_LOST_LOCK                                   0x02
#define	ADV7441A_BIT_IN_LOCK                                     0x01

#define	ADV7441A_REG_IDENT                                       0x11

#define	ADV7441A_REG_STATUS_2                                    0x12
#define	ADV7441A_BIT_TLLC_PLL_LOCK                               0x80
#define	ADV7441A_BIT_CP_FREE_RUN                                 0x40
#define	ADV7441A_BIT_FSC_NSTD                                    0x20
#define	ADV7441A_BIT_LL_NSTD                                     0x10
#define	ADV7441A_BIT_MV_AGC_DET                                  0x08
#define	ADV7441A_BIT_MV_PS_DET                                   0x04
#define	ADV7441A_BIT_MVCS_T3                                     0x02
#define	ADV7441A_BIT_MVCS_DET                                    0x01

#define	ADV7441A_REG_STATUS_3                                    0x13
#define	ADV7441A_BIT_PAL_SW_LOCK                                 0x80
#define	ADV7441A_BIT_INTERLACE                                   0x40
#define	ADV7441A_BIT_STD_FLD_LEN                                 0x20
#define	ADV7441A_BIT_FREE_RUN_ACT                                0x10
#define	ADV7441A_BIT_CVBS                                        0x08
#define	ADV7441A_BIT_SD_OP_50HZ                                  0x04
#define	ADV7441A_BIT_GEMD                                        0x02
#define	ADV7441A_BIT_INST_HLOCK                                  0x01

#define	ADV7441A_REG_ANALOGUE_CONTROL_INTERNAL                   0x13
#define	ADV7441A_BIT_AA_FILT_HIGH_BW_EN                          0x02

#define	ADV7441A_REG_ANALOGUE_CLAMP_CONTROL                      0x14
#define	ADV7441A_BIT_CCLEN                                       0x10

#define	ADV7441A_REG_DIGITAL_CLAMP_CONTROL_1                     0x15

#define	ADV7441A_REG_SHAPING_FILTER_CONTROL                      0x17

#define	ADV7441A_REG_SHAPING_FILTER_CONTROL_2                    0x18
#define	ADV7441A_BIT_WYSFMOVR                                    0x80

#define	ADV7441A_REG_COMB_FILTER_CONTROL                         0x19

#define	ADV7441A_REG_VERTICAL_SCALE_VALUE_1                      0x1D
#define	ADV7441A_BIT_TRI_LLC                                     0x80
#define	ADV7441A_BIT_EN28XTAL                                    0x40

#define	ADV7441A_REG_PIXEL_DELAY_CONTROL                         0x27
#define	ADV7441A_BIT_SWPC                                        0x80
#define	ADV7441A_BIT_AUTO_PDC_EN                                 0x40

#define	ADV7441A_REG_MISC_GAIN_CONTROL                           0x2B
#define	ADV7441A_BIT_CKE                                         0x40
#define	ADV7441A_BIT_PW_UPD                                      0x01

#define	ADV7441A_REG_AGC_MODE_CONTROL                            0x2C

#define	ADV7441A_REG_CHROMA_GAIN_CONTROL_1                       0x2D

#define	ADV7441A_REG_CHROMA_GAIN_CONTROL_2                       0x2E

#define	ADV7441A_REG_LUMA_GAIN_CONTROL_1                         0x2F

#define	ADV7441A_REG_LUMA_GAIN_CONTROL_2                         0x30

#define	ADV7441A_REG_VSYNC_FIELD_CONTROL_1                       0x31
#define	ADV7441A_BIT_NEWAVMODE                                   0x10
#define	ADV7441A_BIT_HVSTIM                                      0x08

#define	ADV7441A_REG_VSYNC_FIELD_CONTROL_2                       0x32
#define	ADV7441A_BIT_VSBHO                                       0x80
#define	ADV7441A_BIT_VSBHE                                       0x40

#define	ADV7441A_REG_VSYNC_FIELD_CONTROL_3                       0x33
#define	ADV7441A_BIT_VSEHO                                       0x80
#define	ADV7441A_BIT_VSEHE                                       0x40

#define	ADV7441A_REG_HSYNC_POSITION_CONTROL_1                    0x34

#define	ADV7441A_REG_HSYNC_POSITION_CONTROL_2                    0x35

#define	ADV7441A_REG_HSYNC_POSITION_CONTROL_3                    0x36

#define	ADV7441A_REG_POLARITY                                    0x37
#define	ADV7441A_BIT_PHS                                         0x80
#define	ADV7441A_BIT_PVS                                         0x20
#define	ADV7441A_BIT_PF                                          0x08
#define	ADV7441A_BIT_PCLK                                        0x01

#define	ADV7441A_REG_NTSC_COMB_CONTROL                           0x38

#define	ADV7441A_REG_PAL_COMB_CONTROL                            0x39

#define	ADV7441A_REG_ADC_CONTROL                                 0x3A
#define	ADV7441A_BIT_PDN_ADC0                                    0x08
#define	ADV7441A_BIT_PDN_ADC1                                    0x04
#define	ADV7441A_BIT_PDN_ADC2                                    0x02
#define	ADV7441A_BIT_PDN_ADC3                                    0x01

#define	ADV7441A_REG_TLLC_CONTROL_ANALOGUE                       0x3C

#define	ADV7441A_REG_MANUAL_WINDOW_CONTROL                       0x3D

#define	ADV7441A_REG_RESAMPLE_CONTROL                            0x41
#define	ADV7441A_BIT_SFL_INV                                     0x40

#define	ADV7441A_REG_AUTOPLL_PARM_CTRL                           0x47
#define	ADV7441A_BIT_CALC_VCO_RANGE_EN                           0x08
#define	ADV7441A_BIT_CALC_PLL_QPUMP_EN                           0x02

#define	ADV7441A_REG_GEMSTAR_CTRL_1                              0x48

#define	ADV7441A_REG_GEMSTAR_CTRL_2                              0x49

#define	ADV7441A_REG_GEMSTAR_CTRL_3                              0x4A

#define	ADV7441A_REG_GEMSTAR_CTRL_4                              0x4B

#define	ADV7441A_REG_GEMSTAR_CTRL_5                              0x4C
#define	ADV7441A_BIT_GDECAD                                      0x01

#define	ADV7441A_REG_CTI_DNR_CTRL_1                              0x4D
#define	ADV7441A_BIT_DNR_EN                                      0x20
#define	ADV7441A_BIT_CTI_AB_EN                                   0x02
#define	ADV7441A_BIT_CTI_EN                                      0x01

#define	ADV7441A_REG_CTI_DNR_CTRL_2                              0x4E

#define	ADV7441A_REG_CTI_DNR_CTRL_4                              0x50

#define	ADV7441A_REG_LOCK_COUNT                                  0x51
#define	ADV7441A_BIT_FSCLE                                       0x80
#define	ADV7441A_BIT_SRLS                                        0x40

#define	ADV7441A_REG_CSC_1                                       0x52

#define	ADV7441A_REG_CSC_2                                       0x53

#define	ADV7441A_REG_CSC_3                                       0x54

#define	ADV7441A_REG_CSC_4                                       0x55

#define	ADV7441A_REG_CSC_5                                       0x56

#define	ADV7441A_REG_CSC_6                                       0x57

#define	ADV7441A_REG_CSC_7                                       0x58

#define	ADV7441A_REG_CSC_8                                       0x59

#define	ADV7441A_REG_CSC_9                                       0x5A

#define	ADV7441A_REG_CSC_10                                      0x5B

#define	ADV7441A_REG_CSC_11                                      0x5C

#define	ADV7441A_REG_CSC_12                                      0x5D

#define	ADV7441A_REG_CSC_13                                      0x5E

#define	ADV7441A_REG_CSC_14                                      0x5F

#define	ADV7441A_REG_CSC_15                                      0x60

#define	ADV7441A_REG_CSC_16                                      0x61

#define	ADV7441A_REG_CSC_17                                      0x62

#define	ADV7441A_REG_CSC_18                                      0x63

#define	ADV7441A_REG_CSC_19                                      0x64

#define	ADV7441A_REG_CSC_20                                      0x65

#define	ADV7441A_REG_CSC_21                                      0x66

#define	ADV7441A_REG_CSC_22                                      0x67
#define	ADV7441A_BIT_EMBSYNCONALL                                0x20
#define	ADV7441A_BIT_SOFT_FILT                                   0x10
#define	ADV7441A_BIT_DS_ONLY                                     0x08

#define	ADV7441A_REG_CSC_23                                      0x68
#define	ADV7441A_BIT_ALT_DATA_SAT                                0x04
#define	ADV7441A_BIT_RGB_OUT                                     0x02
#define	ADV7441A_BIT_CSC_ALT_GAMMA                               0x01

#define	ADV7441A_REG_CONFIG_1                                    0x69
#define	ADV7441A_BIT_TRI_LEVEL                                   0x80
#define	ADV7441A_BIT_SYN_LOTRIG                                  0x40
#define	ADV7441A_BIT_INV_DINCLK                                  0x20
#define	ADV7441A_BIT_EIA_861B_COMPLIANCE                         0x04

#define	ADV7441A_REG_TLLC_PHASE_ADJUST                           0x6A
#define	ADV7441A_BIT_PLL_DLL_UPD_VS_EN                           0x40
#define	ADV7441A_BIT_BYP_DLL                                     0x20

#define	ADV7441A_REG_CP_OUTPUT_SELECTION                         0x6B
#define	ADV7441A_BIT_HS_OUT_SEL                                  0x80
#define	ADV7441A_BIT_VS_OUT_SEL                                  0x40
#define	ADV7441A_BIT_DE_OUT_SEL                                  0x20
#define	ADV7441A_BIT_OP_656_RANGE                                0x10

#define	ADV7441A_REG_CP_CLAMP_1                                  0x6C
#define	ADV7441A_BIT_CLMP_A_MAN                                  0x80
#define	ADV7441A_BIT_CLMP_BC_MAN                                 0x40
#define	ADV7441A_BIT_CLMP_FREEZE                                 0x20

#define	ADV7441A_REG_CP_CLAMP_2                                  0x6D

#define	ADV7441A_REG_CP_CLAMP_3                                  0x6E

#define	ADV7441A_REG_CP_CLAMP_4                                  0x6F

#define	ADV7441A_REG_CP_CLAMP_5                                  0x70

#define	ADV7441A_REG_CP_AGC_1                                    0x71
#define	ADV7441A_BIT_AGC_TAR_MAN                                 0x20
#define	ADV7441A_BIT_AGC_FREEZE                                  0x10
#define	ADV7441A_BIT_HS_NORM                                     0x08

#define	ADV7441A_REG_CP_AGC_2                                    0x72

#define	ADV7441A_REG_CP_AGC_3                                    0x73
#define	ADV7441A_BIT_GAIN_MAN                                    0x80
#define	ADV7441A_BIT_AGC_MODE_MAN                                0x40

#define	ADV7441A_REG_CP_AGC_4                                    0x74

#define	ADV7441A_REG_CP_AGC_5                                    0x75

#define	ADV7441A_REG_CP_AGC_6                                    0x76

#define	ADV7441A_REG_CP_OFFSET_1                                 0x77

#define	ADV7441A_REG_CP_OFFSET_2                                 0x78

#define	ADV7441A_REG_CP_OFFSET_3                                 0x79

#define	ADV7441A_REG_CP_OFFSET_4                                 0x7A

#define	ADV7441A_REG_CP_AV_CONTROL                               0x7B
#define	ADV7441A_BIT_AV_INV_F                                    0x80
#define	ADV7441A_BIT_AV_INV_V                                    0x40
#define ADV7441A_BIT_AV_INTERLACED                               0x20
#define	ADV7441A_BIT_AV_BLANK_EN                                 0x08
#define	ADV7441A_BIT_AV_POS_SEL                                  0x04
#define	ADV7441A_BIT_AV_CODE_EN                                  0x02
#define	ADV7441A_BIT_DE_WITH_AVCODE                              0x01

#define	ADV7441A_REG_CP_HVF_CONTROL_1                            0x7C
#define	ADV7441A_BIT_PIN_INV_HS                                  0x80
#define	ADV7441A_BIT_PIN_INV_VS                                  0x40
#define	ADV7441A_BIT_PIN_INV_F                                   0x20
#define	ADV7441A_BIT_PIN_INV_DE                                  0x10

#define	ADV7441A_REG_CP_HVF_CONTROL_2                            0x7D

#define	ADV7441A_REG_CP_HVF_CONTROL_3                            0x7E

#define	ADV7441A_REG_CP_HVF_CONTROL_4                            0x7F

#define	ADV7441A_REG_CP_HVF_CONTROL_5                            0x80

#define	ADV7441A_REG_CP_MEASURE_CONTROL_1                        0x81
#define	ADV7441A_BIT_GR_AV_BL_EN                                 0x10

#define	ADV7441A_REG_CP_MEASURE_CONTROL_2                        0x82

#define	ADV7441A_REG_CP_MEASURE_CONTROL_3                        0x83

#define	ADV7441A_REG_CP_MEASURE_CONTROL_4                        0x84
#define	ADV7441A_BIT_IFSD_AVG                                    0x01

#define	ADV7441A_REG_CP_DETECTION_CONTROL_1                      0x85
#define	ADV7441A_BIT_POL_MAN_EN                                  0x80
#define	ADV7441A_BIT_POL_VS                                      0x40
#define	ADV7441A_BIT_POL_HSCS                                    0x20
#define	ADV7441A_BIT_TRIG_SSPD                                   0x04
#define	ADV7441A_BIT_SSPD_CONT                                   0x02
#define	ADV7441A_BIT_DS_OUT                                      0x01

#define	ADV7441A_REG_CP_MISC_CONTROL_1                           0x86
#define	ADV7441A_BIT_CPOP_INV_PRPB                               0x10
#define	ADV7441A_BIT_STDI_LINE_COUNT_MODE                        0x08
#define	ADV7441A_BIT_TRIG_STDI                                   0x04
#define	ADV7441A_BIT_STDI_CONT                                   0x02

#define	ADV7441A_REG_CP_TLLC_CONTROL_1                           0x87
#define	ADV7441A_BIT_PLL_DIV_MAN_EN                              0x80

#define	ADV7441A_REG_CP_TLLC_CONTROL_2                           0x88

#define	ADV7441A_REG_CP_TLLC_CONTROL_3                           0x89
#define	ADV7441A_BIT_SWP_CR_CB_WB                                0x10

#define	ADV7441A_REG_CP_TLLC_CONTROL_4                           0x8A
#define	ADV7441A_BIT_VCO_RANGE_MAN                               0x80

#define	ADV7441A_REG_CP_DE_CTRL_1__58                            0x8B

#define	ADV7441A_REG_CP_DE_CTRL_2__59                            0x8C

#define	ADV7441A_REG_CP_DE_CTRL_3__60                            0x8D

#define	ADV7441A_REG_CP_DE_CTRL_4__61                            0x8E

#define	ADV7441A_REG_FREE_RUN_LINE_LENGTH_1                      0x8F
#define	ADV7441A_BIT_LLC_PAD_SEL_MAN                             0x40

#define	ADV7441A_REG_VBI_INFO                                    0x90
#define	ADV7441A_BIT_CGMSD                                       0x08
#define	ADV7441A_BIT_EDTVD                                       0x04
#define	ADV7441A_BIT_CCAPD                                       0x02
#define	ADV7441A_BIT_WSSD                                        0x01

#define	ADV7441A_REG_FREE_RUN_LINE_LENGH_2                       0x90

#define	ADV7441A_REG_WSS_1                                       0x91

#define	ADV7441A_REG_DPP_CP_64                                   0x91
#define	ADV7441A_BIT_INTERLACED                                  0x40

#define	ADV7441A_REG_WSS_2                                       0x92

#define	ADV7441A_REG_EDTV_1                                      0x93

#define	ADV7441A_REG_EDTV_2                                      0x94

#define	ADV7441A_REG_EDTV_3                                      0x95

#define	ADV7441A_REG_CGMS_1                                      0x96

#define	ADV7441A_REG_CGMS_2                                      0x97

#define	ADV7441A_REG_CGMS_3                                      0x98

#define	ADV7441A_REG_CCAP_1                                      0x99

#define	ADV7441A_REG_CCAP_2                                      0x9A

#define	ADV7441A_REG_LETTERBOX_1                                 0x9B

#define	ADV7441A_REG_LETTERBOX_2                                 0x9C

#define	ADV7441A_REG_LETTERBOX_3                                 0x9D

#define	ADV7441A_REG_RB_CP_AGC_1                                 0xA0

#define	ADV7441A_REG_RB_CP_AGC_2                                 0xA1

#define	ADV7441A_REG_RB_CP_MEASURE_1                             0xA2

#define	ADV7441A_REG_DPP_CP_81                                   0xA2

#define	ADV7441A_REG_RB_CP_MEASURE_2                             0xA3

#define	ADV7441A_REG_DPP_CP_82                                   0xA3

#define	ADV7441A_REG_RB_CP_MEASURE_3                             0xA4

#define	ADV7441A_REG_DPP_CP_83                                   0xA4

#define	ADV7441A_REG_RB_CP_MEASURE_4                             0xA5

#define	ADV7441A_REG_DPP_CP_84                                   0xA5

#define	ADV7441A_REG_RB_CP_MEASURE_5                             0xA6

#define	ADV7441A_REG_DPP_CP_85                                   0xA6

#define	ADV7441A_REG_RB_CP_HSYNC_DEPTH_1                         0xA7

#define	ADV7441A_REG_DPP_CP_86                                   0xA7

#define	ADV7441A_REG_RB_CP_HSYNC_DEPTH_2                         0xA8

#define	ADV7441A_REG_DPP_CP_87                                   0xA8

#define	ADV7441A_REG_RB_CP_HSYNC_DEPTH_3                         0xA9

#define	ADV7441A_REG_DPP_CP_88                                   0xA9

#define	ADV7441A_REG_RB_CP_HSYNC_DEPTH_4                         0xAA

#define	ADV7441A_REG_DPP_CP_89                                   0xAA

#define	ADV7441A_REG_RB_CP_HSYNC_DEPTH_5                         0xAB

#define	ADV7441A_REG_DPP_CP_90                                   0xAB

#define	ADV7441A_REG_RB_CP_HSYNC_DEPTH_6                         0xAC

#define	ADV7441A_REG_DPP_CP_91                                   0xAC

#define	ADV7441A_REG_RB_CP_PEAK_VIDEO_1                          0xAD

#define	ADV7441A_REG_RB_CP_PEAK_VIDEO_2                          0xAE

#define	ADV7441A_REG_RB_CP_PEAK_VIDEO_3                          0xAF

#define	ADV7441A_REG_RB_CP_PEAK_VIDEO_4                          0xB0

#define	ADV7441A_REG_RB_STANDARD_IDENT_1                         0xB1
#define	ADV7441A_BIT_STDI_DVALID                                 0x80
#define	ADV7441A_BIT_STDI_INTLCD                                 0x40

#define	ADV7441A_REG_RB_STANDARD_IDENT_2                         0xB2

#define	ADV7441A_REG_DPP_CP_97                                   0xB2
#define	ADV7441A_BIT_CRC_ENABLE                                  0x04

#define	ADV7441A_REG_RB_STANDARD_IDENT_3                         0xB3

#define	ADV7441A_REG_DPP_CP_98                                   0xB3

#define	ADV7441A_REG_RB_STANDARD_IDENT_4                         0xB4

#define	ADV7441A_REG_RB_STANDARD_IDENT_5                         0xB5
#define	ADV7441A_BIT_SSPD_DVALID                                 0x80
#define	ADV7441A_BIT_VS_ACT                                      0x40
#define	ADV7441A_BIT_CUR_POL_VS                                  0x20
#define	ADV7441A_BIT_HS_ACT                                      0x10
#define	ADV7441A_BIT_CUR_POL_HS                                  0x08

#define	ADV7441A_REG_DPP_CP_105                                  0xBA
#define	ADV7441A_BIT_ADC_HDMI_SIMULTANEOUS_MODE                  0x80
#define	ADV7441A_BIT_HDMI_FRUN_MODE                              0x02
#define	ADV7441A_BIT_HDMI_FRUN_EN                                0x01

#define	ADV7441A_REG_DPP_CP_107                                  0xBC

#define	ADV7441A_REG_CP_DEBUG_0                                  0xBD
#define	ADV7441A_BIT_DPP_BYPASS_EN                               0x10

#define	ADV7441A_REG_CP_DEF_COL_1                                0xBF
#define	ADV7441A_BIT_EXT_VCLMP_POS_EDGE_SEL                      0x10
#define	ADV7441A_BIT_CP_DEF_COL_MAN_VAL                          0x04
#define	ADV7441A_BIT_CP_DEF_COL_AUTO                             0x02
#define	ADV7441A_BIT_CP_FORCE_FREERUN                            0x01

#define	ADV7441A_REG_CP_DEF_COL_2                                0xC0

#define	ADV7441A_REG_CP_DEF_COL_3                                0xC1

#define	ADV7441A_REG_CP_DEF_COL_4                                0xC2

#define	ADV7441A_REG_ADC_SWITCH_1                                0xC3

#define	ADV7441A_REG_ADC_SWITCH_2                                0xC4
#define	ADV7441A_BIT_ADC_SW_MAN                                  0x80
#define	ADV7441A_BIT_SOG_SEL                                     0x40

#define	ADV7441A_REG_CP_CLAMP_POS_HS_CTRL_1                      0xC5
#define	ADV7441A_BIT_EXT_VCLMP_REGEN                             0x10

#define	ADV7441A_REG_CP_CLAMP_POS_HS_CTRL_2                      0xC6

#define	ADV7441A_REG_CP_CLAMP_POS_HS_CTRL_3                      0xC7

#define	ADV7441A_REG_CP_CLAMP_POS_HS_CTRL_4                      0xC8

#define	ADV7441A_REG_DPP_CP_118                                  0xC9
#define	ADV7441A_BIT_DDR_2X_CLK                                  0x10
#define	ADV7441A_BIT_DDR_EN                                      0x08
#define	ADV7441A_BIT_DDR_I2C_RC_FIRST                            0x04
#define	ADV7441A_BIT_EXT_CLK_EN                                  0x02
#define	ADV7441A_BIT_DPP_CP_BYPASS                               0x01

#define	ADV7441A_REG_FIELD_LENGTH_COUNT_1                        0xCA

#define	ADV7441A_REG_FIELD_LENGTH_COUNT_2                        0xCB

#define	ADV7441A_REG_LETTERBOX_CONTROL_1                         0xDC

#define	ADV7441A_REG_LETTERBOX_CONTROL_2                         0xDD

#define	ADV7441A_REG_ST_NOISE_READBACK_1                         0xDE
#define	ADV7441A_BIT_ST_NOISE_VLD                                0x08

#define	ADV7441A_REG_ST_NOISE_READBACK_2                         0xDF

#define	ADV7441A_REG_SD_OFFSET_U                                 0xE1

#define	ADV7441A_REG_SD_OFFSET_V                                 0xE2

#define	ADV7441A_REG_SD_SATURATION_U                             0xE3

#define	ADV7441A_REG_SD_SATURATION_V                             0xE4

#define	ADV7441A_REG_NTSC_V_BIT_BEGIN                            0xE5
#define	ADV7441A_BIT_NVBEGDELO                                   0x80
#define	ADV7441A_BIT_NVBEGDELE                                   0x40
#define	ADV7441A_BIT_NVBEGSIGN                                   0x20

#define	ADV7441A_REG_NTSC_V_BIT_END                              0xE6
#define	ADV7441A_BIT_NVENDDELO                                   0x80
#define	ADV7441A_BIT_NVENDDELE                                   0x40
#define	ADV7441A_BIT_NVENDSIGN                                   0x20

#define	ADV7441A_REG_NTSC_F_BIT_TOGGLE                           0xE7
#define	ADV7441A_BIT_NFTOGDELO                                   0x80
#define	ADV7441A_BIT_NFTOGDELE                                   0x40
#define	ADV7441A_BIT_NFTOGSIGN                                   0x20

#define	ADV7441A_REG_PAL_V_BIT_BEGIN                             0xE8
#define	ADV7441A_BIT_PVBEGDELO                                   0x80
#define	ADV7441A_BIT_PVBEGDELE                                   0x40
#define	ADV7441A_BIT_PVBEGSIGN                                   0x20

#define	ADV7441A_REG_PAL_V_BIT_END                               0xE9
#define	ADV7441A_BIT_PVENDDELO                                   0x80
#define	ADV7441A_BIT_PVENDDELE                                   0x40
#define	ADV7441A_BIT_PVENDSIGN                                   0x20

#define	ADV7441A_REG_PAL_F_BIT_TOGGLE                            0xEA
#define	ADV7441A_BIT_PFTOGDELO                                   0x80
#define	ADV7441A_BIT_PFTOGDELE                                   0x40
#define	ADV7441A_BIT_PFTOGSIGN                                   0x20

#define	ADV7441A_REG_VBLANK_CONTROL_1                            0xEB

#define	ADV7441A_REG_VBLANK_CONTROL_2                            0xEC

#define	ADV7441A_REG_FB_STATUS                                   0xED

#define	ADV7441A_REG_FB_CONTROL1                                 0xED
#define	ADV7441A_BIT_FB_INV                                      0x08
#define	ADV7441A_BIT_CVBS_RGB_SEL                                0x04

#define	ADV7441A_REG_FB_CONTROL_2                                0xEE
#define	ADV7441A_BIT_FB_CSC_MAN                                  0x80

#define	ADV7441A_REG_FB_CONTROL_3                                0xEF
#define	ADV7441A_BIT_CNTR_ENABLE                                 0x08

#define	ADV7441A_REG_FB_CONTROL_4                                0xF0

#define	ADV7441A_REG_FB_CONTROL_5                                0xF1
#define	ADV7441A_BIT_RGB_IP_SEL                                  0x01

#define	ADV7441A_REG_AFE_CONTROL_1                               0xF3

#define	ADV7441A_REG_DRIVE_STRENGTH                              0xF4

#define	ADV7441A_REG_IF_COMP_CONTROL                             0xF8

#define	ADV7441A_REG_VS_MODE_CONTROL                             0xF9
#define	ADV7441A_BIT_EXTEND_VS_MIN_FREQ                          0x02
#define	ADV7441A_BIT_EXTEND_VS_MAX_FREQ                          0x01

#define	ADV7441A_REG_PEAKING_CONTROL                             0xFB

#define	ADV7441A_REG_CORING_THRESHOLD_2                          0xFC

#define	ADV7441A_REG_GENERAL_CHIP_CTL                            0xFD
#define	ADV7441A_BIT_PIN_CHECKER_EN                              0x01

#define	ADV7441A_REG_PIN_CHECKER_OP                              0xFE

#endif
