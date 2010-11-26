
#ifndef ADV7441A_USER_1_MAP_REG_H
#define ADV7441A_USER_1_MAP_REG_H

#define	ADV7441A_REG_INTERRUPT_CONFIGURATION_0                   0x40
#define	ADV7441A_BIT_INTERNAL_MUTE_INT                           0x08

#define	ADV7441A_REG_INTERRUPT_CONFIGURATION_1                   0x41
#define	ADV7441A_BIT_INTERNAL_MUTE_INT2                          0x08

#define	ADV7441A_REG_INTERRUPT_STATUS_1                          0x42
#define	ADV7441A_BIT_SSPD_RESULT_Q                               0x80
#define	ADV7441A_BIT_MV_PS_CS_Q                                  0x40
#define	ADV7441A_BIT_SD_FR_CHNG_Q                                0x20
#define	ADV7441A_BIT_STDI_DVALID_Q                               0x10
#define	ADV7441A_BIT_CP_UNLOCK_Q                                 0x08
#define	ADV7441A_BIT_CP_LOCK_Q                                   0x04
#define	ADV7441A_BIT_SD_UNLOCK_Q                                 0x02
#define	ADV7441A_BIT_SD_LOCK_Q                                   0x01

#define	ADV7441A_REG_INTERRUPT_CLEAR_1                           0x42
#define	ADV7441A_BIT_SSPD_RESULT_CLR                             0x80
#define	ADV7441A_BIT_MV_PS_CS_CLR                                0x40
#define	ADV7441A_BIT_SD_FR_CHNG_CLR                              0x20
#define	ADV7441A_BIT_STDI_DVALID_CLR                             0x10
#define	ADV7441A_BIT_CP_UNLOCK_CLR                               0x08
#define	ADV7441A_BIT_CP_LOCK_CLR                                 0x04
#define	ADV7441A_BIT_SD_UNLOCK_CLR                               0x02
#define	ADV7441A_BIT_SD_LOCK_CLR                                 0x01

#define	ADV7441A_REG_INTERRUPT_2_MASKB_1                         0x43
#define	ADV7441A_BIT_SSPD_RESULT_MSKB2                           0x80
#define	ADV7441A_BIT_MV_PS_CS_MSKB2                              0x40
#define	ADV7441A_BIT_SD_FR_CHNG_MSKB2                            0x20
#define	ADV7441A_BIT_STDI_DVALID_MSKB2                           0x10
#define	ADV7441A_BIT_CP_UNLOCK_MSKB2                             0x08
#define	ADV7441A_BIT_CP_LOCK_MSKB2                               0x04
#define	ADV7441A_BIT_SD_UNLOCK_MSKB2                             0x02
#define	ADV7441A_BIT_SD_LOCK_MSKB2                               0x01

#define	ADV7441A_REG_INTERRUPT_MASKB_1                           0x44
#define	ADV7441A_BIT_SSPD_RESULT_MSKB                            0x80
#define	ADV7441A_BIT_MV_PS_CS_MSKB                               0x40
#define	ADV7441A_BIT_SD_FR_CHNG_MSKB                             0x20
#define	ADV7441A_BIT_STDI_DVALID_MSKB                            0x10
#define	ADV7441A_BIT_CP_UNLOCK_MSKB                              0x08
#define	ADV7441A_BIT_CP_LOCK_MSKB                                0x04
#define	ADV7441A_BIT_SD_UNLOCK_MSKB                              0x02
#define	ADV7441A_BIT_SD_LOCK_MSKB                                0x01

#define	ADV7441A_REG_RAW_STATUS_2                                0x45
#define	ADV7441A_BIT_MPU_STIM_INTRQ                              0x80
#define	ADV7441A_BIT_EVEN_FIELD                                  0x10

#define	ADV7441A_REG_INTERRUPT_STATUS_2                          0x46
#define	ADV7441A_BIT_MPU_STIM_INTRQ_Q                            0x80
#define	ADV7441A_BIT_SD_FIELD_CHNGD_Q                            0x10
#define	ADV7441A_BIT_WSS_CHNGD_Q                                 0x08
#define	ADV7441A_BIT_CGMS_CHNGD_Q                                0x04
#define	ADV7441A_BIT_GEMD_Q                                      0x02
#define	ADV7441A_BIT_CCAPD_Q                                     0x01

#define	ADV7441A_REG_INTERRUPT_CLEAR_2                           0x46
#define	ADV7441A_BIT_MPU_STIM_INTRQ_CLR                          0x80
#define	ADV7441A_BIT_SD_FIELD_CHNGD_CLR                          0x10
#define	ADV7441A_BIT_WSS_CHNGD_CLR                               0x08
#define	ADV7441A_BIT_CGMS_CHNGD_CLR                              0x04
#define	ADV7441A_BIT_GEMD_CLR                                    0x02
#define	ADV7441A_BIT_CCAPD_CLR                                   0x01

#define	ADV7441A_REG_INTERRUPT_2_MASKB_2                         0x47
#define	ADV7441A_BIT_MPU_STIM_INT2_MSKB2                         0x80
#define	ADV7441A_BIT_SD_FIELD_CHN_MSKB2                          0x10
#define	ADV7441A_BIT_WSS_CHNGD_MSKB2                             0x08
#define	ADV7441A_BIT_CGMS_CHN_MSKB2                              0x04
#define	ADV7441A_BIT_GEMD_CLR_MSKB2                              0x02
#define	ADV7441A_BIT_CCAPD_CLR_MSKB2                             0x01

#define	ADV7441A_REG_INTERRUPT_MASKB_2                           0x48
#define	ADV7441A_BIT_MPU_STIM_INTRQ_MSKB                         0x80
#define	ADV7441A_BIT_SD_FIELD_CHNGD_MSKB                         0x10
#define	ADV7441A_BIT_WSS_CHNGD_MSKB                              0x08
#define	ADV7441A_BIT_CGMS_CHNGD_MSKB                             0x04
#define	ADV7441A_BIT_GEMD_MSKB                                   0x02
#define	ADV7441A_BIT_CCAPD_MSKB                                  0x01

#define	ADV7441A_REG_RAW_STATUS_3                                0x49
#define	ADV7441A_BIT_FL_PLL_LOCKED                               0x80
#define	ADV7441A_BIT_SCM_LOCK                                    0x10
#define	ADV7441A_BIT_SD_H_LOCK                                   0x04
#define	ADV7441A_BIT_SD_V_LOCK                                   0x02

#define	ADV7441A_REG_INTERRUPT_STATUS_3                          0x4A
#define	ADV7441A_BIT_FL_PLL_LOCKED_Q                             0x80
#define	ADV7441A_BIT_PAL_SW_LK_CHNG_Q                            0x20
#define	ADV7441A_BIT_SCM_LOCK_CHNG_Q                             0x10
#define	ADV7441A_BIT_SD_AD_CHNG_Q                                0x08
#define	ADV7441A_BIT_SD_H_LOCK_CHNG_Q                            0x04
#define	ADV7441A_BIT_SD_V_LOCK_CHNG_Q                            0x02
#define	ADV7441A_BIT_SD_OP_CHNG_Q                                0x01

#define	ADV7441A_REG_INTERRUPT_CLEAR_3                           0x4A
#define	ADV7441A_BIT_FL_PLL_LOCKED_CLR                           0x80
#define	ADV7441A_BIT_PAL_SW_LK_CHNG_CLR                          0x20
#define	ADV7441A_BIT_SCM_LOCK_CHNG_CLR                           0x10
#define	ADV7441A_BIT_SD_AD_CHNG_CLR                              0x08
#define	ADV7441A_BIT_SD_H_LOCK_CHNG_CLR                          0x04
#define	ADV7441A_BIT_SD_V_LOCK_CHNG_CLR                          0x02
#define	ADV7441A_BIT_SD_OP_CHNG_CLR                              0x01

#define	ADV7441A_REG_INTERRUPT_2_MASKB_3                         0x4B
#define	ADV7441A_BIT_FL_PLL_LOCKED_MSKB2                         0x80
#define	ADV7441A_BIT_PAL_SW_LK_CHNG_MSKB2                        0x20
#define	ADV7441A_BIT_SCM_LOCK_CHNG_MSKB2                         0x10
#define	ADV7441A_BIT_SD_AD_CHNG_MSKB2                            0x08
#define	ADV7441A_BIT_SD_H_LOCK_CHNG_MSKB2                        0x04
#define	ADV7441A_BIT_SD_V_LOCK_CHNG_MSKB2                        0x02
#define	ADV7441A_BIT_SD_OP_CHNG_MSKB2                            0x01

#define	ADV7441A_REG_INTERRUPT_MASKB_3                           0x4C
#define	ADV7441A_BIT_FL_PLL_LOCKED_MSKB                          0x80
#define	ADV7441A_BIT_PAL_SW_LK_CHNG_MSKB                         0x20
#define	ADV7441A_BIT_SCM_LOCK_CHNG_MSKB                          0x10
#define	ADV7441A_BIT_SD_AD_CHNG_MSKB                             0x08
#define	ADV7441A_BIT_SD_H_LOCK_CHNG_MSKB                         0x04
#define	ADV7441A_BIT_SD_V_LOCK_CHNG_MSKB                         0x02
#define	ADV7441A_BIT_SD_OP_CHNG_MSKB                             0x01

#define	ADV7441A_REG_INTERRUPT_STATUS_4                          0x4E
#define	ADV7441A_BIT_VDP_VITC_Q                                  0x40
#define	ADV7441A_BIT_VDP_GS_VPS_PDC_UTC_CHNG_Q                   0x10
#define	ADV7441A_BIT_VDP_CGMS_WSS_CHNGD_Q                        0x04
#define	ADV7441A_BIT_VDP_CCAPD_Q                                 0x01

#define	ADV7441A_REG_INTERRUPT_CLEAR_4                           0x4E
#define	ADV7441A_BIT_VDP_VITC_CLR                                0x40

#define	ADV7441A_REG_INTERRUPT_2_MASKB_4                         0x4F
#define	ADV7441A_BIT_VDP_VITC_MSKB2                              0x40
#define	ADV7441A_BIT_VDP_GS_VPS_PDC_UTC_CHNG_MSKB2               0x10
#define	ADV7441A_BIT_VDP_CGMS_WSS_CHNGD_MSKB2                    0x04
#define	ADV7441A_BIT_VDP_CCAPD_MSKB2                             0x01

#define	ADV7441A_REG_INTERRUPT_MASKB_4                           0x50
#define	ADV7441A_BIT_VDP_VITC_MSKB                               0x40
#define	ADV7441A_BIT_VDP_GS_VPS_PDC_UTC_CHNG_MSKB                0x10
#define	ADV7441A_BIT_VDP_CGMS_WSS_CHNGD_MSKB                     0x04
#define	ADV7441A_BIT_VDP_CCAPD_MSKB                              0x01

#define	ADV7441A_REG_HDMI_RAW_STATUS_1                           0x60
#define	ADV7441A_BIT_DSD_PCKT_RAW                                0x80
#define	ADV7441A_BIT_ISRC2_PCKT_RAW                              0x40
#define	ADV7441A_BIT_ISRC1_PCKT_RAW                              0x20
#define	ADV7441A_BIT_ACP_PCKT_RAW                                0x10
#define	ADV7441A_BIT_MS_INFO_RAW                                 0x08
#define	ADV7441A_BIT_SPD_INFO_RAW                                0x04
#define	ADV7441A_BIT_AUDIO_INFO_RAW                              0x02
#define	ADV7441A_BIT_AVI_INFO_RAW                                0x01

#define	ADV7441A_REG_HDMI_INT_STATUS_1                           0x61
#define	ADV7441A_BIT_DSD_PCKT_ST                                 0x80
#define	ADV7441A_BIT_ISRC2_PCKT_ST                               0x40
#define	ADV7441A_BIT_ISRC1_PCKT_ST                               0x20
#define	ADV7441A_BIT_ACP_PCKT_ST                                 0x10
#define	ADV7441A_BIT_MS_INFO_ST                                  0x08
#define	ADV7441A_BIT_SPD_INFO_ST                                 0x04
#define	ADV7441A_BIT_AUDIO_INFO_ST                               0x02
#define	ADV7441A_BIT_AVI_INFO_ST                                 0x01

#define	ADV7441A_REG_HDMI_INT_CLR_1                              0x61
#define	ADV7441A_BIT_DSD_PCKT_CLR                                0x80
#define	ADV7441A_BIT_ISRC2_PCKT_CLR                              0x40
#define	ADV7441A_BIT_ISRC1_PCKT_CLR                              0x20
#define	ADV7441A_BIT_ACP_PCKT_CLR                                0x10
#define	ADV7441A_BIT_MS_INFO_CLR                                 0x08
#define	ADV7441A_BIT_SPD_INFO_CLR                                0x04
#define	ADV7441A_BIT_AUDIO_INFO_CLR                              0x02
#define	ADV7441A_BIT_AVI_INFO_CLR                                0x01

#define	ADV7441A_REG_HDMI_INT2_MASKB_1                           0x62
#define	ADV7441A_BIT_DSD_PCKT_MB2                                0x80
#define	ADV7441A_BIT_ISRC2_PCKT_MB2                              0x40
#define	ADV7441A_BIT_ISRC1_PCKT_MB2                              0x20
#define	ADV7441A_BIT_ACP_PCKT_MB2                                0x10
#define	ADV7441A_BIT_MS_INFO_MB2                                 0x08
#define	ADV7441A_BIT_SPD_INFO_MB2                                0x04
#define	ADV7441A_BIT_AUDIO_INFO_MB2                              0x02
#define	ADV7441A_BIT_AVI_INFO_MB2                                0x01

#define	ADV7441A_REG_HDMI_INT_MASKB_1                            0x63
#define	ADV7441A_BIT_DSD_PCKT_MB1                                0x80
#define	ADV7441A_BIT_ISRC2_PCKT_MB1                              0x40
#define	ADV7441A_BIT_ISRC1_PCKT_MB1                              0x20
#define	ADV7441A_BIT_ACP_PCKT_MB1                                0x10
#define	ADV7441A_BIT_MS_INFO_MB1                                 0x08
#define	ADV7441A_BIT_SPD_INFO_MB1                                0x04
#define	ADV7441A_BIT_AUDIO_INFO_MB1                              0x02
#define	ADV7441A_BIT_AVI_INFO_MB1                                0x01

#define	ADV7441A_REG_HDMI_RAW_STATUS_2                           0x64
#define	ADV7441A_BIT_AUDIO_PLL_LCK_RAW                           0x80
#define	ADV7441A_BIT_HDMI_ENCRPT_RAW                             0x40
#define	ADV7441A_BIT_DE_REGEN_LCK_RAW                            0x20
#define	ADV7441A_BIT_AV_MUTE_RAW                                 0x10
#define	ADV7441A_BIT_INFO_FR_PCKT_RAW                            0x08
#define	ADV7441A_BIT_GEN_CTL_PCKT_RAW                            0x04
#define	ADV7441A_BIT_AUDIO_C_PCKT_RAW                            0x02
#define	ADV7441A_BIT_AUDIO_S_PCKT_RAW                            0x01

#define	ADV7441A_REG_HDMI_INT_STATUS_2                           0x65
#define	ADV7441A_BIT_AUDIO_PLL_LCK_ST                            0x80
#define	ADV7441A_BIT_HDMI_ENCRPT_ST                              0x40
#define	ADV7441A_BIT_DE_REGEN_LCK_ST                             0x20
#define	ADV7441A_BIT_AV_MUTE_ST                                  0x10
#define	ADV7441A_BIT_INFO_FR_PCKT_ST                             0x08
#define	ADV7441A_BIT_GEN_CTL_PCKT_ST                             0x04
#define	ADV7441A_BIT_AUDIO_C_PCKT_ST                             0x02
#define	ADV7441A_BIT_AUDIO_S_PCKT_ST                             0x01

#define	ADV7441A_REG_HDMI_INT_CLR_2                              0x65
#define	ADV7441A_BIT_AUDIO_PLL_LCK_CLR                           0x80
#define	ADV7441A_BIT_HDMI_ENCRPT_CLR                             0x40
#define	ADV7441A_BIT_DE_REGEN_LCK_CLR                            0x20
#define	ADV7441A_BIT_AV_MUTE_CLR                                 0x10
#define	ADV7441A_BIT_INFO_FR_PCKT_CLR                            0x08
#define	ADV7441A_BIT_GEN_CTL_PCKT_CLR                            0x04
#define	ADV7441A_BIT_AUDIO_C_PCKT_CLR                            0x02
#define	ADV7441A_BIT_AUDIO_S_PCKT_CLR                            0x01

#define	ADV7441A_REG_HDMI_INT2_MASKB_2                           0x66
#define	ADV7441A_BIT_AUDIO_PLL_LCK_MB2                           0x80
#define	ADV7441A_BIT_HDMI_ENCRPT_MB2                             0x40
#define	ADV7441A_BIT_DE_REGEN_LCK_MB2                            0x20
#define	ADV7441A_BIT_AV_MUTE_MB2                                 0x10
#define	ADV7441A_BIT_INFO_FR_PCKT_MB2                            0x08
#define	ADV7441A_BIT_GEN_CTL_PCKT_MB2                            0x04
#define	ADV7441A_BIT_AUDIO_C_PCKT_MB2                            0x02
#define	ADV7441A_BIT_AUDIO_S_PCKT_MB2                            0x01

#define	ADV7441A_REG_HDMI_INT_MASKB_2                            0x67
#define	ADV7441A_BIT_AUDIO_PLL_LCK_MB1                           0x80
#define	ADV7441A_BIT_HDMI_ENCRPT_MB1                             0x40
#define	ADV7441A_BIT_DE_REGEN_LCK_MB1                            0x20
#define	ADV7441A_BIT_AV_MUTE_MB1                                 0x10
#define	ADV7441A_BIT_INFO_FR_PCKT_MB1                            0x08
#define	ADV7441A_BIT_GEN_CTL_PCKT_MB1                            0x04
#define	ADV7441A_BIT_AUDIO_C_PCKT_MB1                            0x02
#define	ADV7441A_BIT_AUDIO_S_PCKT_MB1                            0x01

#define	ADV7441A_REG_HDMI_RAW_STATUS_3                           0x68
#define	ADV7441A_BIT_V_LOCKED_RAW                                0x80
#define	ADV7441A_BIT_GAMUT_MDATA_RAW                             0x40
#define	ADV7441A_BIT_INTERNAL_MUTE_RAW                           0x20
#define	ADV7441A_BIT_TMDS_CLK_A_RAW                              0x10
#define	ADV7441A_BIT_TMDS_CLK_B_RAW                              0x08
#define	ADV7441A_BIT_AUDIO_CH_MODE_R                             0x04
#define	ADV7441A_BIT_HDMI_MODE_RAW                               0x02
#define	ADV7441A_BIT_VIDEO_PLL_LCK_RAW                           0x01

#define	ADV7441A_REG_HDMI_INT_STATUS_3                           0x69
#define	ADV7441A_BIT_V_LOCKED_ST                                 0x80
#define	ADV7441A_BIT_GAMUT_MDATA_ST                              0x40
#define	ADV7441A_BIT_INTERNAL_MUTE_ST                            0x20
#define	ADV7441A_BIT_TMDS_CLK_A_ST                               0x10
#define	ADV7441A_BIT_TMDS_CLK_B_ST                               0x08
#define	ADV7441A_BIT_AUDIO_CH_MD_ST                              0x04
#define	ADV7441A_BIT_HDMI_MODE_ST                                0x02
#define	ADV7441A_BIT_VIDEO_PLL_LCK_ST                            0x01

#define	ADV7441A_REG_HDMI_INT_CLR_3                              0x69
#define	ADV7441A_BIT_V_LOCKED_CL                                 0x80
#define	ADV7441A_BIT_GAMUT_MDATA_CLR                             0x40
#define	ADV7441A_BIT_INTERNAL_MUTE_CLR                           0x20
#define	ADV7441A_BIT_TMDS_CLK_A_CLR                              0x10
#define	ADV7441A_BIT_TMDS_CLK_B_CLR                              0x08
#define	ADV7441A_BIT_AUDIO_CH_MD_CLR                             0x04
#define	ADV7441A_BIT_HDMI_MODE_CLR                               0x02
#define	ADV7441A_BIT_VIDEO_PLL_LCK_CLR                           0x01

#define	ADV7441A_REG_HDMI_INT2_MASKB_3                           0x6A
#define	ADV7441A_BIT_V_LOCKED_MB2                                0x80
#define	ADV7441A_BIT_GAMUT_MDATA_MB2                             0x40
#define	ADV7441A_BIT_INTERNAL_MUTE_MB2                           0x20
#define	ADV7441A_BIT_TMDS_CLK_A_MB2                              0x10
#define	ADV7441A_BIT_TMDS_CLK_B_MB2                              0x08
#define	ADV7441A_BIT_AUDIO_CH_MD_MB2                             0x04
#define	ADV7441A_BIT_HDMI_MODE_MB2                               0x02
#define	ADV7441A_BIT_VIDEO_PLL_LCK_MB2                           0x01

#define	ADV7441A_REG_HDMI_INT_MASKB_3                            0x6B
#define	ADV7441A_BIT_V_LOCKED_MB1                                0x80
#define	ADV7441A_BIT_GAMUT_MDATA_MB1                             0x40
#define	ADV7441A_BIT_INTERNAL_MUTE_MB1                           0x20
#define	ADV7441A_BIT_TMDS_CLK_A_MB1                              0x10
#define	ADV7441A_BIT_TMDS_CLK_B_MB1                              0x08
#define	ADV7441A_BIT_AUDIO_CH_MD_MB1                             0x04
#define	ADV7441A_BIT_HDMI_MODE_MB1                               0x02
#define	ADV7441A_BIT_VIDEO_PLL_LCK_MB1                           0x01

#define	ADV7441A_REG_HDMI_INT_STATUS_4                           0x6C
#define	ADV7441A_BIT_NEW_GAMUT_MDATA_ST                          0x80
#define	ADV7441A_BIT_NEW_ISRC2_PCKT_ST                           0x40
#define	ADV7441A_BIT_NEW_ISRC1_PCKT_ST                           0x20
#define	ADV7441A_BIT_NEW_ACP_PCKT_ST                             0x10
#define	ADV7441A_BIT_NEW_MS_INFO_ST                              0x08
#define	ADV7441A_BIT_NEW_SPD_INFO_ST                             0x04
#define	ADV7441A_BIT_NEW_AUDIO_INFO_ST                           0x02
#define	ADV7441A_BIT_NEW_AVI_INFO_ST                             0x01

#define	ADV7441A_REG_HDMI_INT_CLR_4                              0x6C
#define	ADV7441A_BIT_NEW_GAMUT_MDATA_CLR                         0x80
#define	ADV7441A_BIT_NEW_ISRC2_PCKT_CL                           0x40
#define	ADV7441A_BIT_NEW_ISRC1_PCKT_CL                           0x20
#define	ADV7441A_BIT_NEW_ACP_PCKT_CLR                            0x10
#define	ADV7441A_BIT_NEW_MS_INFO_CLR                             0x08
#define	ADV7441A_BIT_NEW_SPD_INFO_CLR                            0x04
#define	ADV7441A_BIT_NEW_AUDIO_INFO_CL                           0x02
#define	ADV7441A_BIT_NEW_AVI_INFO_CLR                            0x01

#define	ADV7441A_REG_HDMI_INT2_MASKB_4                           0x6D
#define	ADV7441A_BIT_NEW_GAMUT_MDATA_MB2                         0x80
#define	ADV7441A_BIT_NEW_ISRC2_PKT_M2                            0x40
#define	ADV7441A_BIT_NEW_ISRC1_PKT_M2                            0x20
#define	ADV7441A_BIT_NEW_ACP_PKT_MB2                             0x10
#define	ADV7441A_BIT_NEW_MS_INFO_MB2                             0x08
#define	ADV7441A_BIT_NEW_SPD_INFO_MB2                            0x04
#define	ADV7441A_BIT_NEW_AUDIO_INF_M2                            0x02
#define	ADV7441A_BIT_NEW_AVI_INFO_MB2                            0x01

#define	ADV7441A_REG_HDMI_INT_MASKB_4                            0x6E
#define	ADV7441A_BIT_NEW_GAMUT_MDATA_MB1                         0x80
#define	ADV7441A_BIT_NEW_ISRC2_PKT_M1                            0x40
#define	ADV7441A_BIT_NEW_ISRC1_PKT_M1                            0x20
#define	ADV7441A_BIT_NEW_ACP_PKT_MB1                             0x10
#define	ADV7441A_BIT_NEW_MS_INFO_MB1                             0x08
#define	ADV7441A_BIT_NEW_SPD_INFO_MB1                            0x04
#define	ADV7441A_BIT_NEW_AUDIO_IN_M1                             0x02
#define	ADV7441A_BIT_NEW_AVI_INFO_MB1                            0x01

#define	ADV7441A_REG_HDMI_INT_STATUS_5                           0x6F
#define	ADV7441A_BIT_CTS_PASS_THRSH_ST                           0x10
#define	ADV7441A_BIT_CHANGE_N_ST                                 0x08
#define	ADV7441A_BIT_INFOFRAME_ERR_ST                            0x04
#define	ADV7441A_BIT_PACKET_ERROR_ST                             0x02
#define	ADV7441A_BIT_AUDIO_PCKT_ERR_ST                           0x01

#define	ADV7441A_REG_HDMI_INT_CLR_5                              0x6F
#define	ADV7441A_BIT_CTS_PASS_THRSH_CL                           0x10
#define	ADV7441A_BIT_CHANGE_N_CLR                                0x08
#define	ADV7441A_BIT_INFOFRAME_ERR_CL                            0x04
#define	ADV7441A_BIT_PACKET_ERROR_CLR                            0x02
#define	ADV7441A_BIT_AUDIO_PCKT_ERR_CL                           0x01

#define	ADV7441A_REG_HDMI_INT2_MASKB_5                           0x70
#define	ADV7441A_BIT_CTS_PASS_THRS_M2                            0x10
#define	ADV7441A_BIT_CHANGE_N_MB2                                0x08
#define	ADV7441A_BIT_INFOFRAME_ERR_M2                            0x04
#define	ADV7441A_BIT_PACKET_ERROR_MB2                            0x02
#define	ADV7441A_BIT_AUDIO_PKT_ERR_M2                            0x01

#define	ADV7441A_REG_HDMI_INT_MASKB_5                            0x71
#define	ADV7441A_BIT_CTS_PASS_THRS_M1                            0x10
#define	ADV7441A_BIT_CHANGE_N_MB1                                0x08
#define	ADV7441A_BIT_INFOFRAME_ERR_M1                            0x04
#define	ADV7441A_BIT_PACKET_ERROR_MB1                            0x02
#define	ADV7441A_BIT_AUDIO_PKT_ERR_M1                            0x01

#define	ADV7441A_REG_HDMI_STATUS_6                               0x72
#define	ADV7441A_BIT_DEEP_COLOR_CHNG_ST                          0x80
#define	ADV7441A_BIT_VCLK_CHNG_ST                                0x40
#define	ADV7441A_BIT_AKSV_UPDATE_ST                              0x20
#define	ADV7441A_BIT_PARRITY_ERROR_ST                            0x10
#define	ADV7441A_BIT_NEW_SAMP_RT_ST                              0x08
#define	ADV7441A_BIT_AUDIO_FLT_LINE_ST                           0x04
#define	ADV7441A_BIT_NEW_TMDS_FRQ_ST                             0x02

#define	ADV7441A_REG_HDMI_INT_CLR_6                              0x72
#define	ADV7441A_BIT_DEEP_COLOR_CHNG_CLR                         0x80
#define	ADV7441A_BIT_VCLK_CHNG_CL                                0x40
#define	ADV7441A_BIT_AKSV_UPDATE_CL                              0x20
#define	ADV7441A_BIT_PARRITY_ERROR_CL                            0x10
#define	ADV7441A_BIT_NEW_SAMP_RT_CL                              0x08
#define	ADV7441A_BIT_AUDIO_FLT_LINE_CL                           0x04
#define	ADV7441A_BIT_NEW_TMDS_FRQ_CL                             0x02

#define	ADV7441A_REG_HDMI_INT2_MASKB_6                           0x73
#define	ADV7441A_BIT_DEEP_COLOR_CHNG_MB2                         0x80
#define	ADV7441A_BIT_VCLK_CHNG_MB2                               0x40
#define	ADV7441A_BIT_AKSV_UPDATE_M2                              0x20
#define	ADV7441A_BIT_PARRITY_ERROR_M2                            0x10
#define	ADV7441A_BIT_NEW_SAMP_RT_MB2                             0x08
#define	ADV7441A_BIT_AUDIO_FLT_LINE_M2                           0x04
#define	ADV7441A_BIT_NEW_TMDS_FRQ_M2                             0x02

#define	ADV7441A_REG_HDMI_INT_MASKB_6                            0x74
#define	ADV7441A_BIT_DEEP_COLOR_CHNG_MB1                         0x80
#define	ADV7441A_BIT_VCLK_CHNG_MB1                               0x40
#define	ADV7441A_BIT_AKSV_UPDATE_M1                              0x20
#define	ADV7441A_BIT_PARRITY_ERROR_M1                            0x10
#define	ADV7441A_BIT_NEW_SAMP_RT_MB1                             0x08
#define	ADV7441A_BIT_AUDIO_FLT_LINE_M1                           0x04
#define	ADV7441A_BIT_NEW_TMDS_FRQ_M1                             0x02

#define	ADV7441A_REG_DLL_ON_LLC_PATH                             0x75
#define	ADV7441A_BIT_DLL_ON_LLC_EN                               0x40
#define	ADV7441A_BIT_DLL_ON_LLC_MUX                              0x20

#define	ADV7441A_REG_CP_CONTRAST                                 0x9A

#define	ADV7441A_REG_CP_SATURATION                               0x9B

#define	ADV7441A_REG_CP_BRIGHTNESS                               0x9C

#define	ADV7441A_REG_CP_HUE                                      0x9D

#endif
