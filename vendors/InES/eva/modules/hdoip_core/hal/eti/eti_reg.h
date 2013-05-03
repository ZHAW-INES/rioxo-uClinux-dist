/*
 * eti_reg.h
 *
 * Created on: 08.12.2010
 *     Author: buan
 */

#ifndef ETI_REG_H_
#define ETI_REG_H_

/** register byte offset
 *
 */

#define ETI_ADDR_SPACE              212

#define ETI_CONFIG_SET_REG          0
#define ETI_CONFIG_CLR_REG          4
#define ETI_STATUS_REG              8
#define ETI_BURST_SIZE_REG          12
#define ETI_CPU_START_REG           16
#define ETI_CPU_STOP_REG            20
#define ETI_CPU_WRITE_REG           24
#define ETI_CPU_READ_REG            28
#define ETI_CPU_FILTER_MASK_REG     64
#define ETI_AUD_EMB_FILTER_MASK_REG 68
#define ETI_VID_FILTER_MASK_REG     72
#define ETI_DEVICE_IP_REG           76
#define ETI_AUD_SRC_IP_REG          80
#define ETI_VID_SRC_IP_REG          84
#define ETI_AUD_EMB_DST_UDP_PORT_REG 88
#define ETI_VID_DST_UDP_PORT_REG    92
#define ETI_AES_SESSION_KEY0_REG    96
#define ETI_AES_SESSION_KEY1_REG    100
#define ETI_AES_SESSION_KEY2_REG    104
#define ETI_AES_SESSION_KEY3_REG    108
#define ETI_AES_RIV0_REG            112
#define ETI_AES_RIV1_REG            116
#define ETI_DEBUG_REG               124
#define ETI_AUD_EMB_PACKET_CNT_REG  136
#define ETI_VID_PACKET_CNT_REG      140
#define ETI_CPU_PACKET_CNT_REG      144
#define ETI_INV_PACKET_CNT_REG      148

#define ETI_AUD_EMB_FEC_COL_FILTER_MASK_REG     168
#define ETI_AUD_EMB_FEC_ROW_FILTER_MASK_REG     172
#define ETI_VID_FEC_COL_FILTER_MASK_REG         176
#define ETI_VID_FEC_ROW_FILTER_MASK_REG         180
#define ETI_AUD_BOARD_FILTER_MASK_REG           192
#define ETI_AUD_BOARD_FEC_COL_FILTER_MASK_REG   196
#define ETI_AUD_BOARD_FEC_ROW_FILTER_MASK_REG   200
#define ETI_AUD_BOARD_DST_UDP_PORT_REG          204
#define ETI_AUD_BOARD_PACKET_CNT_REG            208


/** bit indexes 
 *
 */
#define ETI_STATUS_FIFO_EMPTY       0x00000001
#define ETI_STATUS_FSM_IDLE         0x00000002
#define ETI_STATUS_INVALID_FRAME    0x00000004
#define ETI_STATUS_CPU_FRAME        0x00000008
#define ETI_STATUS_VIDEO_FRAME      0x00000010
#define ETI_STATUS_AUDIO_FRAME      0x00000020
#define ETI_STATUS_MAC_ERROR        0x00000080
#define ETI_STATUS_IRQ1             0x00001000

#define ETI_CONFIG_VIDEO_ENC_EN     0x00000001
#define ETI_CONFIG_AUDIO_ENC_EN     0x00000002
#define ETI_CONFIG_FSM_EN           0x00000004
#define ETI_CONFIG_AUD_PKT_DROP     0x00000200
#define ETI_CONFIG_VID_PKT_DROP     0x00000400

#endif /* ETI_REG_H_ */
