/*
 *  asi_reg.h
 *
 *  Created on: 20.10.2010
 *      Author: stth
 */

#ifndef ASI_REG_H_
#define ASI_REG_H_

/** register byte offset
 *
 */
#define ASI_BASE_PACKETIZE          (0x00000000)
#define ASI_BASE_CONTROL            (0x00000080)

#define ASI_OFF_CTRL_SET            (ASI_BASE_CONTROL + 0x0000)
#define ASI_OFF_CTRL_CLR            (ASI_BASE_CONTROL + 0x0004)
#define ASI_OFF_CTRL_RD             (ASI_BASE_CONTROL + 0x0008)
#define ASI_OFF_STAT_RD             (ASI_BASE_CONTROL + 0x000C)
#define ASI_OFF_STAT_CLR            (ASI_BASE_CONTROL + 0x0010)
#define ASI_OFF_DSC_START_RO        (ASI_BASE_CONTROL + 0x0014)
#define ASI_OFF_DSC_STOP_RO         (ASI_BASE_CONTROL + 0x0018)
#define ASI_OFF_DSC_WRITE_RW        (ASI_BASE_CONTROL + 0x001C)
#define ASI_OFF_DSC_READ_RO         (ASI_BASE_CONTROL + 0x0020)
#define ASI_OFF_DMA_BURST_SIZE_RW   (ASI_BASE_CONTROL + 0x0024)

#define ASI_OFF_FRAME_SIZE          (ASI_BASE_PACKETIZE + 0x0000)
#define ASI_OFF_DST_MAC_0           (ASI_BASE_PACKETIZE + 0x0004)
#define ASI_OFF_DST_MAC_1           (ASI_BASE_PACKETIZE + 0x0005)
#define ASI_OFF_DST_MAC_2           (ASI_BASE_PACKETIZE + 0x0008)
#define ASI_OFF_DST_MAC_3           (ASI_BASE_PACKETIZE + 0x0009)
#define ASI_OFF_DST_MAC_4           (ASI_BASE_PACKETIZE + 0x000a)
#define ASI_OFF_DST_MAC_5           (ASI_BASE_PACKETIZE + 0x000b)
#define ASI_OFF_SRC_MAC_0           (ASI_BASE_PACKETIZE + 0x000c)
#define ASI_OFF_SRC_MAC_1           (ASI_BASE_PACKETIZE + 0x000d)
#define ASI_OFF_SRC_MAC_2           (ASI_BASE_PACKETIZE + 0x000e)
#define ASI_OFF_SRC_MAC_3           (ASI_BASE_PACKETIZE + 0x000f)
#define ASI_OFF_SRC_MAC_4           (ASI_BASE_PACKETIZE + 0x0010)
#define ASI_OFF_SRC_MAC_5           (ASI_BASE_PACKETIZE + 0x0011)
#define ASI_OFF_TTL_TOS             (ASI_BASE_PACKETIZE + 0x0014)
#define ASI_OFF_SRC_IP              (ASI_BASE_PACKETIZE + 0x0018)
#define ASI_OFF_DST_IP              (ASI_BASE_PACKETIZE + 0x001C)
#define ASI_OFF_SRC_PORT            (ASI_BASE_PACKETIZE + 0x0020)
#define ASI_OFF_DST_PORT            (ASI_BASE_PACKETIZE + 0x0024)
#define ASI_OFF_SSRC                (ASI_BASE_PACKETIZE + 0x0028)
#define ASI_OFF_AUD_INFO            (ASI_BASE_PACKETIZE + 0x002C)
#define ASI_OFF_TIME_PER_WORD       (ASI_BASE_PACKETIZE + 0x0030)
#define ASI_OFF_IP_LENGTH           (ASI_BASE_PACKETIZE + 0x0034)
#define ASI_OFF_UDP_WORD_LENGTH     (ASI_BASE_PACKETIZE + 0x0038)
#define ASI_OFF_PAYLOAD_WORDS       (ASI_BASE_PACKETIZE + 0x003C)
#define ASI_OFF_SIZE_RO             (ASI_BASE_PACKETIZE + 0x0040)
#define ASI_OFF_FS                  (ASI_BASE_PACKETIZE + 0x0044)

/** bit indexes
 *
 */
#define ASI_CTRL_RUN                (1<<0)
#define ASI_STAT_RBFULL             (1<<0)
#define ASI_STAT_IDLE               (1<<1)

/** register masks
 *
 */
#define ASI_TTL_MSK                 (0x0000FF00)
#define ASI_TOS_MSK                 (0x000000FF)
#define ASI_CNT_LEFT_MSK            (0x0000000F)
#define ASI_CNT_RIGHT_MSK           (0x000000F0)
#define ASI_CONTAINER_MSK           (0x00000F00)
#define ASI_FS_CODE                 (0x0000F000)

#endif /* ASI_REG_H_ */
