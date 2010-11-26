//! \file vsi_reg.h
#ifndef VSI_REG_H_
#define VSI_REG_H_

/** register byte offset
 * 
 */
#define VSI_OFF_STATUS_RD               (0x0000)
#define VSI_OFF_STATUS_CLR              (0x0004)
#define VSI_OFF_CONTROL_WR              (0x0008)
#define VSI_OFF_CONTROL_CLR             (0x000C)
#define VSI_OFF_CONTROL_RD              (0x0010)
#define VSI_OFF_ETH_WORD_LEN_RW         (0x0014)
#define VSI_OFF_DST_MAC_FIRST_2_RW      (0x0018)
#define VSI_OFF_DST_MAC_LAST_4_RW       (0x001C)
#define VSI_OFF_SRC_MAC_FIRST_4_RW      (0x0020)
#define VSI_OFF_SRC_MAC_LAST_2_RW       (0x0024)
#define VSI_OFF_TTL_TOS_RW              (0x0028)
#define VSI_OFF_SRC_IP_RW               (0x002C)
#define VSI_OFF_DST_IP_RW               (0x0030)
#define VSI_OFF_SRC_PORT_RW             (0x0034)
#define VSI_OFF_DST_PORT_RW             (0x0038)
#define VSI_OFF_SSRC_RW                 (0x003C)
#define VSI_OFF_DSC_START_RD            (0x0040)
#define VSI_OFF_DSC_STOP_RD             (0x0044)
#define VSI_OFF_DSC_WRITE_RW            (0x0048)
#define VSI_OFF_DSC_READ_RD             (0x004C)
#define VSI_OFF_BURST_SIZE_RW           (0x0050)
#define VSI_OFF_DISC_VFRAMES_RD         (0x0054)


/** bit indexes
 *
 */
#define VSI_CFG_RUN	                    (1<<0)
#define VSI_ST_FIFO2_FULL               (1<<1)
#define VSI_ST_CDFIFO_FULL              (1<<2)
#define VSI_ST_FSM_IDLE                 (1<<3) 

/** register masks
 *
 */
#define VSI_ST_MSK                      (0x0000000E)
#define VSI_ETH_WORD_LEN_MSK            (0x000000FF)
#define VSI_MAC_FIRST_2_MSK             (0x0000FFFF)
#define VSI_TTL_MSK	                    (0x0000FF00)
#define VSI_TOS_MSK                     (0x000000FF)
#define VSI_UDP_PORT_MSK                (0x0000FFFF)

#endif /* VSI_REG_H_ */
