#ifndef GS2972_REG_H_
#define GS2972_REG_H_

// Video Core Registers
//-------------------------------------------------------
#define GS2972_IOPROC                           (0x000)
//-------------------------------------------------------
#define IOPROC_RW_DEFAULT                       (0x0000)
#define IOPROC_RW_TRS_INS                       (0x0001)
#define IOPROC_RW_LNUM_INS                      (0x0002)
#define IOPROC_RW_CRC_INS                       (0x0004)
#define IOPROC_RW_ANC_CSUM_INS                  (0x0008)
#define IOPROC_RW_EDH_CRC_INS                   (0x0010)
#define IOPROC_RW_ILLEGAR_WORD_REMAP            (0x0020)
#define IOPROC_RW_SMPTE_325M_INS                (0x0040)
#define IOPROC_RW_H_CONFIG                      (0x0100)
#define IOPROC_RW_CONV_372                      (0x0200)
#define IOPROC_RW_AUDIO_EMBED                   (0x0400)
#define IOPROC_RW_ANC_INS                       (0x0800)
#define IOPROC_RW_EDH_CRC_UPDATE                (0x1000)
#define IOPROC_RW_AUDIO_LEVELB_STREAM_2_1B      (0x2000)
#define IOPROC_RW_DELAY_LINE_ENABLE             (0x4000)

//-------------------------------------------------------
#define GS2972_ERROR_STAT                       (0x001)
//-------------------------------------------------------
#define ERROR_STAT_R_ERROR_MASK                 (0x007F)
#define ERROR_STAT_R_LOCK_ERR                   (0x0001)
#define ERROR_STAT_R_NO_352M_ERR                (0x0002)
#define ERROR_STAT_R_TIMING_ERR                 (0x0004)
#define ERROR_STAT_R_FORMAT_ERR                 (0x0008)
#define ERROR_STAT_R_Y1_CS_ERR                  (0x0010)
#define ERROR_STAT_R_Y1_EDH_CS_ERR              (0x0020)
#define ERROR_STAT_R_TRS_PERR                   (0x0040)

//-------------------------------------------------------
#define GS2972_EDH_FLAG_EXT                     (0x002)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2972_EDH_FLAG_PGM                     (0x003)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2972_DATA_FORMAT                      (0x004)
//-------------------------------------------------------
#define DATA_FORMAT_VD_STD_MASK                 (0x03E0)
#define DATA_FORMAT_VD_STD_SHIFT                (5)
#define DATA_FORMAT_INT                         (0x0010)
#define DATA_FORMAT_STD_LOCK                    (0x0004)
#define DATA_FORMAT_V_LOCK                      (0x0002)
#define DATA_FORMAT_H_LOCK                      (0x0001)

//-------------------------------------------------------
#define GS2972_VSD_FORCE                          (0x006)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2972_EDH_STATUS                         (0x007)
//-------------------------------------------------------

//-------------------------------------------------------
#define GS2972_FIRST_AVAIL_POSITION               (0x008)
//-------------------------------------------------------


#define GS2972_VIDEO_FORMAT_352_OUT_WORD_1        (0x00A)
#define GS2972_VIDEO_FORMAT_352_OUT_WORD_2        (0x00B)
#define GS2972_VIDEO_FORMAT_352_OUT_WORD_3        (0x00C)
#define GS2972_VIDEO_FORMAT_352_OUT_WORD_4        (0x00D)
#define GS2972_VIDEO_FORMAT_352_IN_WORD_1         (0x00E)
#define GS2972_VIDEO_FORMAT_352_IN_WORD_2         (0x00F)
#define GS2972_VIDEO_FORMAT_352_IN_WORD_3         (0x010)
#define GS2972_VIDEO_FORMAT_352_IN_WORD_4         (0x011)
#define GS2972_RASTER_STRUC_1                     (0x012)
#define GS2972_RASTER_STRUC_2                     (0x013)
#define GS2972_RASTER_STRUC_3                     (0x014)
#define GS2972_RASTER_STRUC_4                     (0x015)

#define GS2972_FIRST_LINE_NUMBER_STATUS           (0x024)
#define GS2972_FIRST_LINE_NUMBER                  (0x025)
#define GS2972_FIRST_LINE_NUMBER_OF_WORDS         (0x026)
#define GS2972_SECOND_LINE_NUMBER                 (0x027)
#define GS2972_SECOND_LINE_NUMBER_OF_WORDS        (0x028)
#define GS2972_THIRD_LINE_NUMBER                  (0x029)
#define GS2972_THIRD_LINE_NUMBER_OF_WORDS         (0x02A)
#define GS2972_FOURTH_LINE_NUMBER                 (0x02B)
#define GS2972_FOURTH_LINE_NUMBER_OF_WORDS        (0x02C)
#define GS2972_STREAM_TYPE_1                      (0x02D)

#define GS2972_ANC_PACKET_BANK_1_START            (0x040)
#define GS2972_ANC_PACKET_BANK_1_END              (0x07F)

#define GS2972_ANC_PACKET_BANK_2_START            (0x080)
#define GS2972_ANC_PACKET_BANK_2_END              (0x0BF)

#define GS2972_ANC_PACKET_BANK_3_START            (0x0C0)
#define GS2972_ANC_PACKET_BANK_3_END              (0x0FF)

#define GS2972_ANC_PACKET_BANK_4_START            (0x100)
#define GS2972_ANC_PACKET_BANK_4_END              (0x13F)

#define GS2972_SDTI_TDM                           (0x20A)
#define GS2972_LEVELB_INDICATION                  (0x20D)

//-------------------------------------------------------
#define GS2972_DRIVE_STRENGTH                   (0x20E)
//-------------------------------------------------------
#define DRIVE_STRENGTH_RW_4_MA                  (0x0000)
#define DRIVE_STRENGTH_RW_6_MA                  (0x0015)
#define DRIVE_STRENGTH_RW_8_MA                  (0x002A)
#define DRIVE_STRENGTH_RW_10_MA                 (0x003F)

//-------------------------------------------------------
#define GS2972_DRIVE_STRENGTH2                  (0x20F)
//-------------------------------------------------------
#define DRIVE_STRENGTH2_RW_4_MA                 (0x0000)
#define DRIVE_STRENGTH2_RW_6_MA                 (0x0001)
#define DRIVE_STRENGTH2_RW_8_MA                 (0x0002)
#define DRIVE_STRENGTH2_RW_10_MA                (0x0003)





// SD Audio Core Registers
#define GS2972_A_CFG_AUD                          (0x400)
#define GS2972_A_FIFO_BUF_SIDE                    (0x401)
#define GS2972_A_AES_EBU_ERR_STATUS               (0x402)
#define GS2972_A_CHANNEL_STAT_REGEN               (0x403)
#define GS2972_A_PACKET_DET_STATUS                (0x404)
#define GS2972_A_AES_EBU_ERR_STATUS1              (0x405)
#define GS2972_A_CASCADE                          (0x406)

#define GS2972_A_SERIAL_AUDIO_FORMAT              (0x40B)
#define GS2972_A_CHANNEL_XP_GRPA                  (0x40C)
#define GS2972_A_CHANNEL_XP_GRPB                  (0x40D)
#define GS2972_A_INTERRUPT_MASK                   (0x40E)
#define GS2972_A_ACTIVE_CHANNEL                   (0x40F)
#define GS2972_A_XPOINT_ERROR                     (0x410)

#define GS2972_A_CHANNEL_STATUS_REG_1             (0x420)
#define GS2972_A_CHANNEL_STATUS_REG_2             (0x421)
#define GS2972_A_CHANNEL_STATUS_REG_3             (0x422)
#define GS2972_A_CHANNEL_STATUS_REG_4             (0x423)
#define GS2972_A_CHANNEL_STATUS_REG_5             (0x424)
#define GS2972_A_CHANNEL_STATUS_REG_6             (0x425)
#define GS2972_A_CHANNEL_STATUS_REG_7             (0x426)
#define GS2972_A_CHANNEL_STATUS_REG_8             (0x427)
#define GS2972_A_CHANNEL_STATUS_REG_9             (0x428)
#define GS2972_A_CHANNEL_STATUS_REG_10            (0x429)
#define GS2972_A_CHANNEL_STATUS_REG_11            (0x42A)
#define GS2972_A_CHANNEL_STATUS_REG_12            (0x42B)
#define GS2972_A_CHANNEL_STATUS_REG_13            (0x42C)

#define GS2972_A_AUDIO_CTRL_GRPA_REG_1            (0x440)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_2            (0x441)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_3            (0x442)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_4            (0x443)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_5            (0x444)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_6            (0x445)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_7            (0x446)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_8            (0x447)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_9            (0x448)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_10           (0x449)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_11           (0x44A)
#define GS2972_A_AUDIO_CTRL_GRPA_REG_12           (0x44B)

#define GS2972_A_AUDIO_CTRL_GRPB_REG_1            (0x44C)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_2            (0x44D)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_3            (0x44E)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_4            (0x44F)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_5            (0x450)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_6            (0x451)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_7            (0x452)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_8            (0x453)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_9            (0x454)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_10           (0x455)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_11           (0x456)
#define GS2972_A_AUDIO_CTRL_GRPB_REG_12           (0x457)





// HD and 3G Audio Core Registers
#define GS2972_B_CFG_AUD                          (0x800)

#define GS2972_B_CHANNEL_STAT_REGREN              (0x802)
#define GS2972_B_PACKET_DET_STATUS                (0x803)
#define GS2972_B_AES_EBU_ERR_STATUS               (0x804)
#define GS2972_B_CASCADE                          (0x805)

#define GS2972_B_SERIAL_AUDIO_FORMAT              (0x80A)
#define GS2972_B_CHANNEL_XP_GRPA                  (0x80B)
#define GS2972_B_CHANNEL_XP_GRPB                  (0x80C)
#define GS2972_B_INTERRUPT_MASK                   (0x80D)
#define GS2972_B_ACTIVE_CHANNEL                   (0x80E)
#define GS2972_B_XPOINT_ERROR                     (0x80F)

#define GS2972_B_CHANNEL_STATUS_REG_1             (0x820)
#define GS2972_B_CHANNEL_STATUS_REG_2             (0x821)
#define GS2972_B_CHANNEL_STATUS_REG_3             (0x822)
#define GS2972_B_CHANNEL_STATUS_REG_4             (0x823)
#define GS2972_B_CHANNEL_STATUS_REG_5             (0x824)
#define GS2972_B_CHANNEL_STATUS_REG_6             (0x825)
#define GS2972_B_CHANNEL_STATUS_REG_7             (0x826)
#define GS2972_B_CHANNEL_STATUS_REG_8             (0x827)
#define GS2972_B_CHANNEL_STATUS_REG_9             (0x828)
#define GS2972_B_CHANNEL_STATUS_REG_10            (0x829)
#define GS2972_B_CHANNEL_STATUS_REG_11            (0x82A)
#define GS2972_B_CHANNEL_STATUS_REG_12            (0x82B)
#define GS2972_B_CHANNEL_STATUS_REG_13            (0x82C)

#define GS2972_B_AUDIO_CTRL_GRPA_REG_1            (0x840)
#define GS2972_B_AUDIO_CTRL_GRPA_REG_2            (0x841)
#define GS2972_B_AUDIO_CTRL_GRPA_REG_3            (0x842)
#define GS2972_B_AUDIO_CTRL_GRPA_REG_4            (0x843)
#define GS2972_B_AUDIO_CTRL_GRPA_REG_5            (0x844)
#define GS2972_B_AUDIO_CTRL_GRPA_REG_6            (0x845)

#define GS2972_B_AUDIO_CTRL_GRPB_REG_1            (0x846)
#define GS2972_B_AUDIO_CTRL_GRPB_REG_2            (0x847)
#define GS2972_B_AUDIO_CTRL_GRPB_REG_3            (0x848)
#define GS2972_B_AUDIO_CTRL_GRPB_REG_4            (0x849)
#define GS2972_B_AUDIO_CTRL_GRPB_REG_5            (0x84A)
#define GS2972_B_AUDIO_CTRL_GRPB_REG_6            (0x84B)

#endif /*GS2972_REG_H_*/
