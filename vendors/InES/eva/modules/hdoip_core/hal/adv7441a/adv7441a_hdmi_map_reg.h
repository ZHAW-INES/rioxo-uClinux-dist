
#ifndef ADV7441A_HDMI_MAP_REG_H
#define ADV7441A_HDMI_MAP_REG_H

#define	ADV7441A_REG_REGISTER_00H                                0x00
#define	ADV7441A_BIT_HDCP_ONLY_MODE                              0x02
#define	ADV7441A_BIT_HDMI_PORT_SELECT                            0x01

#define	ADV7441A_REG_REGISTER_01H                                0x01
#define	ADV7441A_BIT_AUDIOPLL_PDN                                0x80
#define	ADV7441A_BIT_CLOCK_TERM_PORT_A                           0x40
#define	ADV7441A_BIT_CLOCK_TERM_PORT_B                           0x20

#define	ADV7441A_REG_REGISTER_02H                                0x02
#define	ADV7441A_BIT_SPDIF_TRISTATE                              0x20
#define	ADV7441A_BIT_I2S_TRISTATE                                0x10
#define	ADV7441A_BIT_HDCP_A0                                     0x08
#define	ADV7441A_BIT_OVR_AUTO_MUX_DSD_OUT                        0x02
#define	ADV7441A_BIT_MUX_DSD_OUT                                 0x01

#define	ADV7441A_REG_REGISTER_03H                                0x03

#define	ADV7441A_REG_REGISTER_04H                                0x04
#define	ADV7441A_BIT_AV_MUTE                                     0x40
#define	ADV7441A_BIT_HDCP_KEYS_READ                              0x20
#define	ADV7441A_BIT_HDCP_KEY_ERROR                              0x10
#define	ADV7441A_BIT_TMDS_PORT_A_ACTIVE                          0x08
#define	ADV7441A_BIT_TMDS_PORT_B_ACTIVE                          0x04
#define	ADV7441A_BIT_VIDEO_PLL_LOCKED                            0x02
#define	ADV7441A_BIT_AUDIO_PLL_LOCKED                            0x01

#define	ADV7441A_REG_REGISTER_05H                                0x05
#define	ADV7441A_BIT_HDMI_MODE                                   0x80
#define	ADV7441A_BIT_HDMI_CONTENT_ENCRYPTED                      0x40
#define	ADV7441A_BIT_DVI_HSYNC_POLARITY                          0x20
#define	ADV7441A_BIT_DVI_VSYNC_POLARITY                          0x10

#define	ADV7441A_REG_TMDSFREQ                                    0x06

#define	ADV7441A_REG_REGISTER_07H                                0x07
#define	ADV7441A_BIT_VERT_FILTER_LOCKED                          0x40
#define	ADV7441A_BIT_AUDIO_CHANNEL_MODE                          0x20
#define	ADV7441A_BIT_DE_REGEN_FILTER_LOCKED                      0x10

#define	ADV7441A_REG_LINE_WIDTH                                  0x08

#define	ADV7441A_REG_FIELD_0_HEIGHT_MSB                          0x09

#define	ADV7441A_REG_FIELD_0_HEIGHT                              0x0A

#define	ADV7441A_REG_REGISTER_0BH                                0x0B
#define	ADV7441A_BIT_HDMI_INTERLACED                             0x10

#define	ADV7441A_REG_FIELD_1_HEIGHT                              0x0C

#define	ADV7441A_REG_REGISTER_0DH                                0x0D

#define	ADV7441A_REG_AUDIO_MUTE_SPEED                            0x0F
#define	ADV7441A_BIT_MAN_AUDIO_DL_BYPASS                         0x80
#define	ADV7441A_BIT_AUDIO_DELAY_LINE_BYPASS                     0x40

#define	ADV7441A_REG_REGISTER_10H                                0x10

#define	ADV7441A_REG_AUDIO_COAST_MASK                            0x13
#define	ADV7441A_BIT_AC_MSK_VCLK_CHNG                            0x40
#define	ADV7441A_BIT_AC_MSK_VPLL_UNLOCK                          0x20
#define	ADV7441A_BIT_AC_MSK_VFREQ_CHNG                           0x10
#define	ADV7441A_BIT_AC_MSK_NEW_CTS                              0x08
#define	ADV7441A_BIT_AC_MSK_NEW_N                                0x04
#define	ADV7441A_BIT_AC_MSK_CHNG_PORT                            0x02
#define	ADV7441A_BIT_AC_MSK_VCLK_DET                             0x01

#define	ADV7441A_REG_MUTE_MASK_19_16                             0x14
#define	ADV7441A_BIT_MT_MSK_COMPRS_AUD                           0x20
#define	ADV7441A_BIT_MT_MSK_CHNG_DSD_PCM                         0x10
#define	ADV7441A_BIT_MT_MSK_PARITY_ERR                           0x02
#define	ADV7441A_BIT_MT_MSK_VCLK_CHNG                            0x01

#define	ADV7441A_REG_MUTE_MASK_15_8                              0x15
#define	ADV7441A_BIT_MT_MSK_APLL_UNLOCK                          0x80
#define	ADV7441A_BIT_MT_MSK_VPLL_UNLOCK                          0x40
#define	ADV7441A_BIT_MT_MSK_ACR_NOT_DET                          0x20
#define	ADV7441A_BIT_MT_MSK_SAMP_RT_CHNG                         0x10
#define	ADV7441A_BIT_MT_MSK_FLATLINE_DET                         0x08
#define	ADV7441A_BIT_MT_MSK_VFREQ_CHNG                           0x04

#define	ADV7441A_REG_MUTE_MASK_7_0                               0x16
#define	ADV7441A_BIT_MT_MSK_AVMUTE                               0x80
#define	ADV7441A_BIT_MT_MSK_NOT_HDMIMODE                         0x40
#define	ADV7441A_BIT_MT_MSK_NEW_CTS                              0x20
#define	ADV7441A_BIT_MT_MSK_NEW_N                                0x10
#define	ADV7441A_BIT_MT_MSK_CHMODE_CHNG                          0x08
#define	ADV7441A_BIT_MT_MSK_APCKT_ECC_ERR                        0x04
#define	ADV7441A_BIT_MT_MSK_CHNG_PORT                            0x02
#define	ADV7441A_BIT_MT_MSK_VCLK_DET                             0x01

#define	ADV7441A_REG_PACKETS_DETECTED_MSB                        0x17
#define	ADV7441A_BIT_DST_AUDIO_PCKT_DET                          0x40
#define	ADV7441A_BIT_HBR_AUDIO_PCKT_DET                          0x20
#define	ADV7441A_BIT_GAMUT_MDATA_PCKT_DET                        0x10
#define	ADV7441A_BIT_INFOFRAME_PCKT_DET                          0x08
#define	ADV7441A_BIT_GC_PACKET_DET                               0x04
#define	ADV7441A_BIT_ACR_PACKET_DET                              0x02
#define	ADV7441A_BIT_AUDIO_SAMPLE_PCKT_DET                       0x01

#define	ADV7441A_REG_PACKETS_DETECTED                            0x18
#define	ADV7441A_BIT_DSD_PACKET_DET                              0x80
#define	ADV7441A_BIT_ISRC2_PACKET_DET                            0x40
#define	ADV7441A_BIT_ISRC1_PACKET_DET                            0x20
#define	ADV7441A_BIT_ACP_PACKET_DET                              0x10
#define	ADV7441A_BIT_MS_INFOFRAME_DET                            0x08
#define	ADV7441A_BIT_SPD_INFOFRAME_DET                           0x04
#define	ADV7441A_BIT_AUDIO_INFOFRAME_DET                         0x02
#define	ADV7441A_BIT_AVI_INFOFRAME_DET                           0x01

#define	ADV7441A_REG_PACKET_STATUS_FLAGS_20_16                   0x19
#define	ADV7441A_BIT_AUDIO_PARITY_ERR                            0x10
#define	ADV7441A_BIT_SAMP_RT_CHNG                                0x08
#define	ADV7441A_BIT_FLATLINE_BIT_SET                            0x04
#define	ADV7441A_BIT_NEW_TMDS_FREQ                               0x02

#define	ADV7441A_REG_PACKET_STATUS_FLAGS_15_8                    0x1A
#define	ADV7441A_BIT_NEW_CTS                                     0x10
#define	ADV7441A_BIT_NEW_N                                       0x08
#define	ADV7441A_BIT_ERR_IN_INFOFRAME_PCKT                       0x04
#define	ADV7441A_BIT_ERR_IN_UNKNOWN_PCKT                         0x02
#define	ADV7441A_BIT_ERR_IN_AUDIO_PCKT                           0x01

#define	ADV7441A_REG_PACKET_STATUS_FLAGS_7_0                     0x1B
#define	ADV7441A_BIT_NEW_GAMUT_MDATA_PCKT                        0x80
#define	ADV7441A_BIT_NEW_ISRC2_PCKT                              0x40
#define	ADV7441A_BIT_NEW_ISRC1_PCKT                              0x20
#define	ADV7441A_BIT_NEW_ACP_PCKT                                0x10
#define	ADV7441A_BIT_NEW_MS_INFOFRAME                            0x08
#define	ADV7441A_BIT_NEW_SPD_INFOFRAME                           0x04
#define	ADV7441A_BIT_NEW_AUDIO_INFOFRAME                         0x02
#define	ADV7441A_BIT_NEW_AVI_INFOFRAME                           0x01

#define	ADV7441A_REG_REGISTER_1CH                                0x1C
#define	ADV7441A_BIT_MCLKPLLEN                                   0x40

#define	ADV7441A_REG_REGISTER_1DH                                0x1D
#define	ADV7441A_BIT_UP_CONVERSION_MODE                          0x20
#define	ADV7441A_BIT_MUTE_AUDIO                                  0x08
#define	ADV7441A_BIT_NOT_AUTO_UNMUTE                             0x01
#define ADV7441A_BIT_DELAY_UNMUTE_1_SEC                          0x04

#define	ADV7441A_REG_TOTAL_LINE_WIDTH_MSB                        0x1E

#define	ADV7441A_REG_TOTAL_LINE_WIDTH                            0x1F

#define	ADV7441A_REG_HSYNC_FRONT_PORCH_MSB                       0x20

#define	ADV7441A_REG_HSYNC_FRONT_PORCH                           0x21

#define	ADV7441A_REG_HSYNC_PULSE_WIDTH_MSB                       0x22

#define	ADV7441A_REG_HSYNC_PULSE_WIDTH                           0x23

#define	ADV7441A_REG_HSYNC_BACK_PORCH_MSB                        0x24

#define	ADV7441A_REG_HSYNC_BACK_PORCH                            0x25

#define	ADV7441A_REG_FIELD0_TOTAL_HEIGHT_MSB                     0x26

#define	ADV7441A_REG_FIELD0_TOTAL_HEIGHT                         0x27

#define	ADV7441A_REG_FIELD1_TOTAL_HEIGHT_MSB                     0x28

#define	ADV7441A_REG_FIELD1_TOTAL_HEIGHT                         0x29

#define	ADV7441A_REG_FIELD0_VS_FRONT_PORCH_MSB                   0x2A

#define	ADV7441A_REG_FIELD0_VS_FRONT_PORCH                       0x2B

#define	ADV7441A_REG_FIELD1_VS_FRONT_PORCH_MSB                   0x2C

#define	ADV7441A_REG_FIELD1_VS_FRONT_PORCH                       0x2D

#define	ADV7441A_REG_FIELD0_VS_PULSE_WIDTH_MSB                   0x2E

#define	ADV7441A_REG_FIELD0_VS_PULSE_WIDTH                       0x2F

#define	ADV7441A_REG_FIELD1_VS_PULSE_WIDTH_MSB                   0x30

#define	ADV7441A_REG_FIELD1_VS_PULSE_WIDTH                       0x31

#define	ADV7441A_REG_FIELD0_VS_BACK_PORCH_MSB                    0x32

#define	ADV7441A_REG_FIELD0_VS_BACK_PORCH                        0x33

#define	ADV7441A_REG_FIELD1_VS_BACK_PORCH_MSB                    0x34

#define	ADV7441A_REG_FIELD1_VS_BACK_PORCH                        0x35

#define	ADV7441A_REG_CHANNEL_STATUS_DATA_0                       0x36

#define	ADV7441A_REG_CHANNEL_STATUS_DATA_1                       0x37

#define	ADV7441A_REG_CHANNEL_STATUS_DATA_2                       0x38

#define	ADV7441A_REG_CHANNEL_STATUS_DATA_3                       0x39

#define	ADV7441A_REG_CHANNEL_STATUS_DATA_4                       0x3A

#define	ADV7441A_REG_PLL_DIVIDER                                 0x3C

#define	ADV7441A_REG_REGISTER_41H                                0x41
#define	ADV7441A_BIT_DEREP_N_OVERRIDE                            0x10

#define	ADV7441A_REG_REGISTER_50                                 0x50
#define	ADV7441A_BIT_CS_COPYRIGHT_FORCE                          0x02

#define	ADV7441A_REG_REGISTER_5A                                 0x5A
#define	ADV7441A_BIT_PACKET_DET_RESET                            0x02
#define	ADV7441A_BIT_AUDIO_PLL_RESET                             0x01

#define	ADV7441A_REG_CTS_REG1                                    0x5B

#define	ADV7441A_REG_CTS_REG2                                    0x5C

#define	ADV7441A_REG_CTS_REG3_N_REG1                             0x5D

#define	ADV7441A_REG_N_REG2                                      0x5E

#define	ADV7441A_REG_N_REG3                                      0x5F

#define	ADV7441A_REG_GAMUT_MDATA_BODY_0                          0x60

#define	ADV7441A_REG_GAMUT_MDATA_BODY_1                          0x61

#define	ADV7441A_REG_GAMUT_MDATA_BODY_2                          0x62

#define	ADV7441A_REG_GAMUT_MDATA_BODY_3                          0x63

#define	ADV7441A_REG_GAMUT_MDATA_BODY_4                          0x64

#define	ADV7441A_REG_GAMUT_MDATA_BODY_5                          0x65

#define	ADV7441A_REG_GAMUT_MDATA_BODY_6                          0x66

#define	ADV7441A_REG_GAMUT_MDATA_BODY_7                          0x68

#define	ADV7441A_REG_GAMUT_MDATA_BODY_8                          0x69

#define	ADV7441A_REG_GAMUT_MDATA_BODY_9                          0x6A

#define	ADV7441A_REG_GAMUT_MDATA_BODY_10                         0x6B

#define	ADV7441A_REG_GAMUT_MDATA_BODY_11                         0x6C

#define	ADV7441A_REG_GAMUT_MDATA_BODY_12                         0x6D

#define	ADV7441A_REG_GAMUT_MDATA_BODY_13                         0x6E

#define	ADV7441A_REG_GAMUT_MDATA_BODY_14                         0x70

#define	ADV7441A_REG_GAMUT_MDATA_BODY_15                         0x71

#define	ADV7441A_REG_GAMUT_MDATA_BODY_16                         0x72

#define	ADV7441A_REG_GAMUT_MDATA_BODY_17                         0x73

#define	ADV7441A_REG_GAMUT_MDATA_BODY_18                         0x74

#define	ADV7441A_REG_GAMUT_MDATA_BODY_19                         0x75

#define	ADV7441A_REG_GAMUT_MDATA_BODY_20                         0x76

#define	ADV7441A_REG_GAMUT_MDATA_BODY_21                         0x78

#define	ADV7441A_REG_GAMUT_MDATA_BODY_22                         0x79

#define	ADV7441A_REG_GAMUT_MDATA_BODY_23                         0x7A

#define	ADV7441A_REG_GAMUT_MDATA_BODY_24                         0x7B

#define	ADV7441A_REG_GAMUT_MDATA_BODY_25                         0x7C

#define	ADV7441A_REG_GAMUT_MDATA_BODY_26                         0x7D

#define	ADV7441A_REG_GAMUT_MDATA_BODY_27                         0x7E

#define	ADV7441A_REG_AVI_INF_VERS                                0x80

#define	ADV7441A_REG_AVI_DATA_BYTE_1                             0x81
#define	ADV7441A_BIT_A                                           0x10

#define	ADV7441A_REG_AVI_INF_BYTE_2                              0x82

#define	ADV7441A_REG_AVI_INF_BYTE_3                              0x83
#define	ADV7441A_BIT_ITC                                         0x80

#define	ADV7441A_REG_AVI_INF_BYTE_4                              0x84

#define	ADV7441A_REG_AVI_INF_BYTE_5                              0x85

#define	ADV7441A_REG_AVI_INF_BYTE_6                              0x86

#define	ADV7441A_REG_AVI_INF_BYTE_7                              0x88

#define	ADV7441A_REG_AVI_INF_BYTE_8                              0x89

#define	ADV7441A_REG_AVI_INF_BYTE_9                              0x8A

#define	ADV7441A_REG_AVI_INF_BYTE_10                             0x8B

#define	ADV7441A_REG_AVI_INF_BYTE_11                             0x8C

#define	ADV7441A_REG_AVI_INF_BYTE_12                             0x8D

#define	ADV7441A_REG_AVI_INF_BYTE_13                             0x8E

#define	ADV7441A_REG_AUDIO_INF_VERS                              0x90

#define	ADV7441A_REG_AUDIO_INF_BYTE_1                            0x91

#define	ADV7441A_REG_AUDIO_INF_BYTE_2                            0x92

#define	ADV7441A_REG_AUDIO_INF_BYTE_3                            0x93

#define	ADV7441A_REG_AUDIO_INF_BYTE_4                            0x94

#define	ADV7441A_REG_AUDIO_INF_BYTE_5                            0x95
#define	ADV7441A_BIT_DM_INH                                      0x80

#define	ADV7441A_REG_SOURCE_PROD_INF_VERS                        0x98

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_1                      0x99

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_2                      0x9A

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_3                      0x9B

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_4                      0x9C

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_5                      0x9D

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_6                      0x9E

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_7                      0xA0

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_8                      0xA1

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_9                      0xA2

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_10                     0xA3

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_11                     0xA4

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_12                     0xA5

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_13                     0xA6

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_14                     0xA8

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_15                     0xA9

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_16                     0xAA

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_17                     0xAB

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_18                     0xAC

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_19                     0xAD

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_20                     0xAE

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_21                     0xB0

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_22                     0xB1

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_23                     0xB2

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_24                     0xB3

#define	ADV7441A_REG_SOURCE_PROD_INF_BYTE_25                     0xB4

#define	ADV7441A_REG_MPEG_SOURCE_INF_VERS                        0xB8

#define	ADV7441A_REG_MPEG_SOURCE_INF_BYTE_1                      0xB9

#define	ADV7441A_REG_MPEG_SOURCE_INF_BYTE_2                      0xBA

#define	ADV7441A_REG_MPEG_SOURCE_INF_BYTE_3                      0xBB

#define	ADV7441A_REG_MPEG_SOURCE_INF_BYTE_4                      0xBC

#define	ADV7441A_REG_MPEG_SOURCE_INF_BYTE_5                      0xBD
#define	ADV7441A_BIT_FR                                          0x10

#define	ADV7441A_REG_ACP_TYPE                                    0xC0

#define	ADV7441A_REG_ACP_PACKET_BYTE_0                           0xC1

#define	ADV7441A_REG_ACP_PACKET_BYTE_1                           0xC2

#define	ADV7441A_REG_ACP_PACKET_BYTE_2                           0xC3

#define	ADV7441A_REG_ACP_PACKET_BYTE_3                           0xC4

#define	ADV7441A_REG_ACP_PACKET_BYTE_4                           0xC5

#define	ADV7441A_REG_ACP_PACKET_BYTE_5                           0xC6

#define	ADV7441A_REG_ACP_PACKET_BYTE_6                           0xC8

#define	ADV7441A_REG_ACP_PACKET_BYTE_7                           0xC9

#define	ADV7441A_REG_ACP_PACKET_BYTE_8                           0xCA

#define	ADV7441A_REG_ACP_PACKET_BYTE_9                           0xCB

#define	ADV7441A_REG_ACP_PACKET_BYTE_10                          0xCC

#define	ADV7441A_REG_ACP_PACKET_BYTE_11                          0xCD

#define	ADV7441A_REG_ACP_PACKET_BYTE_12                          0xCE

#define	ADV7441A_REG_ACP_PACKET_BYTE_13                          0xD0

#define	ADV7441A_REG_ACP_PACKET_BYTE_14                          0xD1

#define	ADV7441A_REG_ACP_PACKET_BYTE_15                          0xD2

#define	ADV7441A_REG_ACP_PACKET_BYTE_16                          0xD3

#define	ADV7441A_REG_ACP_PACKET_BYTE_17                          0xD4

#define	ADV7441A_REG_ACP_PACKET_BYTE_18                          0xD5

#define	ADV7441A_REG_ACP_PACKET_BYTE_19                          0xD6

#define	ADV7441A_REG_ISRC1_INF                                   0xD8
#define	ADV7441A_BIT_ISRC1_CONTINUED                             0x80
#define	ADV7441A_BIT_ISRC1_VALID                                 0x40

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_0                         0xD9

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_1                         0xDA

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_2                         0xDB

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_3                         0xDC

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_4                         0xDD

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_5                         0xDE

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_6                         0xE0

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_7                         0xE1

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_8                         0xE2

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_9                         0xE3

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_10                        0xE4

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_11                        0xE5

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_12                        0xE6

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_13                        0xE8

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_14                        0xE9

#define	ADV7441A_REG_ISRC1_PACKET_BYTE_15                        0xEA

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_0                         0xEB

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_1                         0xEC

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_2                         0xED

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_3                         0xEE

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_4                         0xF0

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_5                         0xF1

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_6                         0xF2

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_7                         0xF3

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_8                         0xF4

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_9                         0xF5

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_10                        0xF6

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_11                        0xF8

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_12                        0xF9

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_13                        0xFA

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_14                        0xFB

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_15                        0xFC

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_16                        0xFD

#define	ADV7441A_REG_ISRC2_PACKET_BYTE_17                        0xFE

#endif
