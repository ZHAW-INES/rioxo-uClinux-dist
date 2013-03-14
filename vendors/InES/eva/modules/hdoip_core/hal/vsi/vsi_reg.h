//! \file vsi_reg.h
#ifndef VSI_REG_H_
#define VSI_REG_H_

/** register byte offset
 * 
 */
#define VSI_ADDR_SPACE                  (0x0058)

#define VSI_OFF_STATUS_RD               (0x0000)
#define VSI_OFF_STATUS_CLR              (0x0004)
#define VSI_OFF_CONTROL_WR              (0x0008)
#define VSI_OFF_CONTROL_CLR             (0x000C)
#define VSI_OFF_CONTROL_RD              (0x0010)
#define VSI_OFF_ETH_WORD_LEN_RW         (0x0014)
#define VSI_OFF_BURST_SIZE_RW           (0x0018)
#define VSI_OFF_DISC_VFRAMES_RD         (0x001C)

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
