//! \file vso_reg.h
#ifndef VSO_REG_H_
#define VSO_REG_H_

/** register byte offset 
 * 
 */

#define VSO_BASE_STATISTIC          (0x0080)
#define VSO_ADDR_SPACE              (VSO_BASE_STATISTIC + 0x0C)

#define VSO_OFF_CTRL			    (0x0000)
#define VSO_OFF_STATUS			    (0x0004)
#define VSO_OFF_TIMESTAMP_RO		(0x0008)
#define VSO_OFF_MARKER_TIMEOUT		(0x000C)
#define VSO_OFF_PACKET_TIMEOUT		(0x0010)
#define VSO_OFF_VS_DELAY		    (0x0014)
#define VSO_OFF_VS_DURATION		    (0x0018)
#define VSO_OFF_VFRAME_LOST_RO		(0x001C)
#define VSO_OFF_VFRAME_CNT_RO		(0x0020)
#define VSO_OFF_PACKET_LOST_RO		(0x0024)
#define VSO_OFF_PACKET_CNT_RO		(0x0028)
#define VSO_OFF_DMA_BURST_SIZE		(0x002C)
#define VSO_OFF_DMA_ALMOST_FULL		(0x0030)
#define VSO_OFF_PACKET_IN_CNT_RO	(0x0034)
#define VSO_OFF_VSYNC_DELAY		    (0x0038)
#define VSO_OFF_SCOMM5_DELAY		(0x003C)
#define VSO_OFF_DSC_START_RO		(0x0040)
#define VSO_OFF_DSC_STOP_RO		    (0x0044)
#define VSO_OFF_DSC_WRITE_RO		(0x0048)
#define VSO_OFF_DSC_READ		    (0x004C)
#define VSO_OFF_TIMESTAMP_MIN		(0x0050)
#define VSO_OFF_TIMESTAMP_MAX		(0x0054)
#define VSO_OFF_TIMESTAMP_DIFF_RO	(0x0058)
#define VSO_OFF_CLKMUX              (0x005C)
#define VSO_OFF_STAT_STATUS         (VSO_BASE_STATISTIC)
#define VSO_OFF_STAT_DELAY          (VSO_BASE_STATISTIC + 0x04)
#define VSO_OFF_STAT_MISS_CNT       (VSO_BASE_STATISTIC + 0x08)

/** bit indexes 
 *
 */
#define VSO_CTRL_RUN			    (1<<0)
#define VSO_CTRL_CLEAR			    (1<<1)
#define VSO_ST_MSK			        (0x3F)
#define VSO_ST_PAYLOAD_EMPTY		(1<<0)
#define VSO_ST_PAYLOAD_MTIMEOUT		(1<<1)
#define VSO_ST_IDLE			        (1<<2)
#define VSO_ST_ACTIVE			    (1<<3)
#define VSO_ST_CHOKED			    (1<<4)
#define VSO_ST_TIMESTAMP_ERROR		(1<<5)
#define VSO_CLKMUX_75_MHZ           (1<<0)
#define VSO_CLKMUX_HDMI_RX          (0x00)


/** register masks
 *
 */
#define VSO_MSK_MARKER_TIMEOUT		(0x0FFFFFFF)
#define VSO_MSK_PACKET_TIMEOUT		(0x0FFFFFFF)

#endif /* VSO_REG_H_ */
