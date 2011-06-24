
#ifndef ADV7441A_USER_3_MAP_REG_H
#define ADV7441A_USER_3_MAP_REG_H

#define	ADV7441A_REG_VDP_CGMS_TYPEB_DATA_1                       0x3C

#define	ADV7441A_REG_VDP_CGMS_TYPEB_DATA_2                       0x3D

#define	ADV7441A_REG_VDP_CGMS_TYPEB_DATA_3                       0x3E

#define	ADV7441A_REG_VDP_CGMS_TYPEB_DATA_4                       0x3F

#define	ADV7441A_REG_VDP_STATUS                                  0x40
#define	ADV7441A_BIT_STATUS_TTXT                                 0x80
#define	ADV7441A_BIT_STATUS_VITC                                 0x40
#define	ADV7441A_BIT_STATUS_GEMS_TYPE                            0x20
#define	ADV7441A_BIT_STATUS_GEMS_VPS                             0x10
#define	ADV7441A_BIT_STATUS_WSS_CGMS                             0x04
#define	ADV7441A_BIT_STATUS_CCAP_EVEN_FIELD                      0x02
#define	ADV7441A_BIT_STATUS_CCAP                                 0x01

#define	ADV7441A_REG_VDP_CCAP_DATA_0                             0x41

#define	ADV7441A_REG_VDP_CCAP_DATA_1                             0x42

#define	ADV7441A_REG_VDP_CGMS_WSS_DATA_0                         0x43

#define	ADV7441A_REG_VDP_CGMS_WSS_DATA_1                         0x44

#define	ADV7441A_REG_VDP_CGMS_WSS_DATA_2                         0x45

#define	ADV7441A_REG_VDP_WSS_BIPHASE_ERROR_COUNT                 0x46

#define	ADV7441A_REG_VDP_GS_VPS_PDC_UTC_CGMSTB_DATA_0            0x47

#define	ADV7441A_REG_VDP_GS_VPS_PDC_UTC_CGMSTB_DATA_1            0x48

#define	ADV7441A_REG_VDP_GS_VPS_PDC_UTC_CGMSTB_DATA_2            0x49

#define	ADV7441A_REG_VDP_GS_VPS_PDC_UTC_CGMSTB_DATA_3            0x4A

#define	ADV7441A_REG_VDP_VPS_PDC_UTC_CGMSTB_DATA_4               0x4B

#define	ADV7441A_REG_VDP_VPS_PDC_UTC_CGMSTB_DATA_5               0x4C

#define	ADV7441A_REG_VDP_VPS_PDC_UTC_CGMSTB_DATA_6               0x4D

#define	ADV7441A_REG_VDP_VPS_PDC_UTC_CGMSTB_DATA_7               0x4E

#define	ADV7441A_REG_VDP_VPS_PDC_UTC_CGMSTB_DATA_8               0x4F

#define	ADV7441A_REG_VDP_VPS_PDC_UTC_CGMSTB_DATA_9               0x50

#define	ADV7441A_REG_VDP_VPS_PDC_UTC_CGMSTB_DATA_10              0x51

#define	ADV7441A_REG_VDP_VPS_PDC_UTC_CGMSTB_DATA_11              0x52

#define	ADV7441A_REG_VDP_VPS_PDC_UTC_CGMSTB_DATA_12              0x53

#define	ADV7441A_REG_VDP_VPS_BIPHASE_ERROR_COUNT                 0x54

#define	ADV7441A_REG_VDP_VITC_DATA_0                             0x55

#define	ADV7441A_REG_VDP_VITC_DATA_1                             0x56

#define	ADV7441A_REG_VDP_VITC_DATA_2                             0x57

#define	ADV7441A_REG_VDP_VITC_DATA_3                             0x58

#define	ADV7441A_REG_VDP_VITC_DATA_4                             0x59

#define	ADV7441A_REG_VDP_VITC_DATA_5                             0x5A

#define	ADV7441A_REG_VDP_VITC_DATA_6                             0x5B

#define	ADV7441A_REG_VDP_VITC_DATA_7                             0x5C

#define	ADV7441A_REG_VDP_VITC_DATA_8                             0x5D

#define	ADV7441A_REG_VDP_VITC_CALC_CRC                           0x5E

#define	ADV7441A_REG_VDP_CONFIG_1                                0x60
#define	ADV7441A_BIT_HAM_ERR_OP_EN                               0x80
#define	ADV7441A_BIT_EN_FC_WINDOW_AFTER_CRI_DET                  0x08
#define	ADV7441A_BIT_VDP_TTXT_TYPE_MAN_EN                        0x04

#define	ADV7441A_REG_VDP_CONFIG_2                                0x61
#define	ADV7441A_BIT_VDP_CP_CLMP_AVG                             0x80
#define	ADV7441A_BIT_NOISE_CLK_DISABLE                           0x20

#define	ADV7441A_REG_VDP_ADF_CONFIG_1                            0x62
#define	ADV7441A_BIT_ADF_EN                                      0x80

#define	ADV7441A_REG_VDP_ADF_CONFIG_2                            0x63
#define	ADV7441A_BIT_TOGGLE_ADF                                  0x80
#define	ADV7441A_BIT_SYS2_DUP_ADF                                0x40

#define	ADV7441A_REG_VDP_MAN_LINE_1_21                           0x64

#define	ADV7441A_REG_VDP_MAN_LINE_2_22                           0x65

#define	ADV7441A_REG_VDP_MAN_LINE_3_23                           0x66

#define	ADV7441A_REG_VDP_MAN_LINE_4_24                           0x67

#define	ADV7441A_REG_VDP_MAN_LINE_5_25                           0x68

#define	ADV7441A_REG_VDP_MAN_LINE_6_26                           0x69

#define	ADV7441A_REG_VDP_MAN_LINE_7_27                           0x6A

#define	ADV7441A_REG_VDP_MAN_LINE_8_28                           0x6B

#define	ADV7441A_REG_VDP_MAN_LINE_9_29                           0x6C

#define	ADV7441A_REG_VDP_MAN_LINE_10_30                          0x6D

#define	ADV7441A_REG_VDP_MAN_LINE_11_31                          0x6E

#define	ADV7441A_REG_VDP_MAN_LINE_12_32                          0x6F

#define	ADV7441A_REG_VDP_MAN_LINE_13_33                          0x70

#define	ADV7441A_REG_VDP_MAN_LINE_14_34                          0x71

#define	ADV7441A_REG_VDP_MAN_LINE_15_35                          0x72

#define	ADV7441A_REG_VDP_MAN_LINE_16_36                          0x73

#define	ADV7441A_REG_VDP_MAN_LINE_17_37                          0x74

#define	ADV7441A_REG_VDP_MAN_LINE_18_38                          0x75

#define	ADV7441A_REG_VDP_MAN_LINE_19_39                          0x76

#define	ADV7441A_REG_VDP_MAN_LINE_20_40                          0x77

#define	ADV7441A_REG_VDP_STATUS_CLEAR                            0x78
#define	ADV7441A_BIT_STATUS_CLEAR_TTXT                           0x80
#define	ADV7441A_BIT_STATUS_CLEAR_VITC                           0x40
#define	ADV7441A_BIT_STATUS_CLEAR_GEMS_VPS                       0x10
#define	ADV7441A_BIT_STATUS_CLEAR_WSS_CGMS                       0x04
#define	ADV7441A_BIT_STATUS_CLEAR_CCAP                           0x01

#define	ADV7441A_REG_VDP_STATUS_CONFIG                           0x9C
#define	ADV7441A_BIT_GS_VPS_PDC_UTC_CB_CHANGE                    0x20
#define	ADV7441A_BIT_WSS_CGMS_CB_CHANGE                          0x10
#define	ADV7441A_BIT_RAW_STATUS_ENA                              0x08


#endif
