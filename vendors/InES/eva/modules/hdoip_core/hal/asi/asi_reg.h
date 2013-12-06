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
#define ASI_BASE_CONTROL            (0x00000100)

#define ASI_OFF_CTRL_SET            (ASI_BASE_CONTROL + 0x0000)
#define ASI_OFF_CTRL_CLR            (ASI_BASE_CONTROL + 0x0004)
#define ASI_OFF_CTRL_RD             (ASI_BASE_CONTROL + 0x0008)
#define ASI_OFF_STAT_RD             (ASI_BASE_CONTROL + 0x000C)
#define ASI_OFF_STAT_CLR            (ASI_BASE_CONTROL + 0x0010)
#define ASI_OFF_DSC_WRITE_RW        (ASI_BASE_CONTROL + 0x001C)
#define ASI_OFF_DSC_READ_RO         (ASI_BASE_CONTROL + 0x0020)
#define ASI_OFF_DMA_BURST_SIZE_RW   (ASI_BASE_CONTROL + 0x0024)
#define ASI_OFF_SEND_REQUEST        (ASI_BASE_CONTROL + 0x0028)
#define ASI_OFF_ACTIVE_STREAM       (ASI_BASE_CONTROL + 0x002C)

#define asi_reg2off(stream, reg)    ((0x80 * stream) + reg)
#define ASI_REG_FRAME_SIZE          (0x0000)
#define ASI_REG_SSRC                (0x0028)
#define ASI_REG_AUD_INFO            (0x002C)
#define ASI_REG_TIME_PER_WORD       (0x0030)
#define ASI_REG_PAYLOAD_WORDS       (0x003C)
#define ASI_REG_SIZE_RO             (0x0040)
#define ASI_REG_FS_DETECT           (0x0044)
#define ASI_REG_CH_MAP_DETECT       (0x0048)

/** single bit mask
 *
 */
#define asi_ctrl_en_mask(stream)    (1 << stream)
#define ASI_STAT_RBFULL             (1 << AUD_STREAM_CNT)
#define asi_stat_idle_mask(stream)  (1 << stream)

/** register masks
 *
 */
#define ASI_TTL_MSK                 (0x0000FF00)
#define ASI_TOS_MSK                 (0x000000FF)
#define ASI_CH_MAP_MSK              (0x000000FF)
#define ASI_CONTAINER_MSK           (0x00000F00)

#endif /* ASI_REG_H_ */
