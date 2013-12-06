/*
 * eto_reg.h
 *
 * Created on: 08.12.2010
 *     Author: buan
 */

#ifndef ETO_REG_H_
#define ETO_REG_H_

/** register byte offset
 *
 */

#define ETO_ADDR_SPACE                  272

#define ETO_CONFIG_SET_REG              0
#define ETO_CONFIG_CLR_REG              4
#define ETO_STATUS_REG                  8
#define ETO_BURST_SIZE_REG              12
#define ETO_CPU_START_REG               16
#define ETO_CPU_STOP_REG                20
#define ETO_CPU_WRITE_REG               24
#define ETO_CPU_READ_REG                28
#define ETO_AUD_START_REG               32
#define ETO_AUD_STOP_REG                36
#define ETO_AUD_WRITE_REG               40
#define ETO_AUD_READ_REG                44
#define ETO_VID_START_REG               48
#define ETO_VID_STOP_REG                52
#define ETO_VID_WRITE_REG               56
#define ETO_VID_READ_REG                60

#define ETO_AES_SESSION_KEY0_REG        96
#define ETO_AES_SESSION_KEY1_REG        100
#define ETO_AES_SESSION_KEY2_REG        104
#define ETO_AES_SESSION_KEY3_REG        108
#define ETO_AES_RIV0_REG                112
#define ETO_AES_RIV1_REG                116

#define ETO_DMA_ALMOST_FULL_REG         120
#define ETO_AES_COUNT_START_L_REG       128
#define ETO_AES_COUNT_START_H_REG       132
#define ETO_AUD_EMB_PACKET_CNT_REG      136
#define ETO_VID_PACKET_CNT_REG          140
#define ETO_CPU_PACKET_CNT_REG          144
#define ETO_INV_PACKET_CNT_REG          148
#define ETO_FRAME_PERIOD_10NS           164

#define ETO_TF_DIVIDER                  184
#define ETO_TF_MULTIPLIER               188

#define ETO_AUD_BOARD_PACKET_CNT_REG    208

#define ETO_PRIORITY_CONFIG_REG         256
#define ETO_CPU_PRIORITY_REG            260	
#define ETO_AUD_PRIORITY_REG            264	
#define ETO_VID_PRIORITY_REG            268	

/** bit indexes 
 *
 */
#define ETO_CONFIG_FSM_EN                   0x00000004
#define ETO_CONFIG_AUDIO_ENC_EN             0x00000002
#define ETO_CONFIG_VIDEO_ENC_EN             0x00000001
#define ETO_CONFIG_AES_COUNT_RST            0x00000020
#define ETO_CONFIG_SEND_CPU_FRAME_DISABLE   0x00000040
#define ETO_CONFIG_SEND_VID_FRAME_DISABLE   0x00000080
#define ETO_CONFIG_SEND_AUD_FRAME_DISABLE   0x00000100
#define ETO_CONFIG_REDUCE_FPS_0             0x00000400
#define ETO_CONFIG_REDUCE_FPS_1             0x00000800
#define ETO_CONFIG_TRAFFIC_SHAPING          0x00001000

#define ETO_STATUS_FSM_IDLE                 0x00000002
#define ETO_STATUS_FRAME_ERROR              0x00000040
#define ETO_STATUS_CPU_IDLE                 0x04000000
#define ETO_STATUS_VID_IDLE                 0x08000000
#define ETO_STATUS_AUD_IDLE                 0x10000000

#define ETO_PRIO_CFG_SHIFT_MASK             0x70000000
#define ETO_PRIO_CFG_CLOCK_DIV_MASK         0x0FFFFFFF
#define ETO_PRIO_SCALE_MASK                 0xFF000000
#define ETO_PRIO_LOAD_MASK                  0x00FFFFFF

#endif /* ETO_REG_H_ */
