#ifndef _HDOIP_H_
#define _HDOIP_H_

#include <linux/types.h>
#include <asm/nios2.h>

#define HDOIP_DEBUG

/* Base addresses defines needed to be consistent across MMU/NOMMU */
#ifndef TSE_MAC_BASE
# define TSE_MAC_BASE			na_tse_mac_control_port
#endif
#ifndef ACB_ETH_OUT_BASE
# define ACB_ETH_OUT_BASE		na_acb_eth_out
#endif
#ifndef ACB_ETH_IN_BASE
# define ACB_ETH_IN_BASE		na_acb_eth_in
#endif
#ifndef ACB_VID_ST_OUT_BASE
# define ACB_VID_ST_OUT_BASE		na_acb_vid_st_out
#endif
#ifndef ACB_VID_ST_IN_BASE
# define ACB_VID_ST_IN_BASE		na_acb_vid_st_in
#endif
#ifndef ACB_AUD_ST_IN_BASE
# define ACB_AUD_ST_IN_BASE		na_acb_aud_st_in
#endif
#ifndef EXT_RESET_PIO_BASE
# define EXT_RESET_PIO_BASE		na_ext_reset_pio
#endif

/* IRQs */
#ifndef ACB_ETH_IN_IRQ
# define ACB_ETH_IN_IRQ			na_acb_eth_in_irq
#endif

/* Register area sizes (XXX: recheck these) */
#define TSE_MAC_SIZE			0x400
#define ACB_ETH_OUT_SIZE		0x54
#define ACB_ETH_IN_SIZE			0x8C
#define ACB_VID_ST_OUT_SIZE		0x170
#define ACB_VID_ST_IN_SIZE		0x80

/* Generic offsets into eth_st_in/eth_st_out configuration register regions */
#define ETHIO_CONFIG			0x00
#define ETHIO_STATUS			0x04
#define ETHIO_CPU_START_DESC		0x10
#define ETHIO_CPU_STOP_DESC		0x14
#define ETHIO_CPU_WRITE_DESC		0x18
#define ETHIO_CPU_READ_DESC		0x1C
#define ETHIO_VID_START_DESC		0x20
#define ETHIO_VID_STOP_DESC		0x24
#define ETHIO_AUD_START_DESC		0x30
#define ETHIO_AUD_STOP_DESC		0x34

/* Offsets into eth_st_in specific configuration register region */
#define ETHI_VID_WRITE_DESC		0x28
#define ETHI_AUD_WRITE_DESC		0x38
#define ETHI_IP_FILTER			0x40
#define ETHI_IP_FILTER_SRC		0x44
#define ETHI_VID_FILTER			0x48
#define ETHI_AUD_FILTER			0x4C
#define ETHI_DMA_CONFIG			0x50
#define ETHI_CPU_PACKET_COUNT		0x54
#define ETHI_CPU_DATA_COUNT		0x58
#define ETHI_CPU_CRC_ERR_COUNT		0x5C
#define ETHI_CPU_INTERAR_JITTER		0x60
#define ETHI_AUD_PACKET_COUNT		0x64
#define ETHI_AUD_DATA_COUNT		0x68
#define ETHI_AUD_CHECKSUM_ERR_COUNT	0x6C
#define ETHI_AUD_INTERAR_JITTER		0x70
#define ETHI_VID_PACKET_COUNT		0x74
#define ETHI_VID_DATA_COUNT		0x78
#define ETHI_VID_CHECKSUM_ERR_COUNT	0x7C
#define ETHI_VID_INTERAR_JITTER		0x80
#define ETHI_DISC_PACKET_COUNT		0x84

/* Offsets into eth_st_out specific configuration register region */
#define ETHO_VID_READ_DESC		0x2C
#define ETHO_AUD_READ_DESC		0x3C
#define ETHO_DMA_CONFIG			0x50

/* Bitmasks for the eth_st_in config register */
#define ETHI_CONFIG_RUN				0x00000001
#define ETHI_CONFIG_IRQ1_RESET			0x00000002
#define ETHI_CONFIG_STATS_RESET			0x00000004
/* Bitmasks for the eth_st_in status register */
#define ETHI_STATUS_RUN				0x00000001
#define ETHI_STATUS_IRQ1_FLAG			0x00000002
/* Bitmasks and indices for the eth_st_int DMA config register */
#define ETHI_CONFIG_DMA_BURST_SIZE_MASK		0x000000FF
#define ETHI_CONFIG_DMA_BURST_SIZE_INDEX	0

/* Bitmasks for the eth_st_out config register */
#define ETHO_CONFIG_RUN				0x00000001
#define ETHO_CONFIG_PRIO			0x00000002
/* Bitmasks for the eth_st_out status register */
#define ETHO_STATUS_RUN				0x00000001
#define ETHO_STATUS_PRIO			0x00000002
/* Bitmasks and indices for the eth_st_out DMA config register */
#define ETHO_CONFIG_DMA_PACKET_SIZE_MASK	0x000003FF
#define ETHO_CONFIG_DMA_PACKET_SIZE_INDEX	0
#define ETHO_CONFIG_DMA_BURST_SIZE_MASK		0x0003FC00
#define ETHO_CONFIG_DMA_BURST_SIZE_INDEX	10
#define ETHO_CONFIG_DMA_FIFO_THRESH_MASK	0x3FFC0000
#define ETHO_CONFIG_DMA_FIFO_THRESH_INDEX	18

/* Avalon will not perform higher bursts as allowed by the Clock Crossing Bridge */
#define ETHIO_DMA_BURST_SIZE			4	/* 64 */

#define ETHO_DMA_FIFO_THRESH			400

/* Maximum size of a ringbuffer entry. Also defines the maximum packet size. */
#define MAX_PKTBUF_SIZE				2048
/* Maximum packet size this driver can handle */
#define MAX_PKT_SIZE				(MAX_PKTBUF_SIZE - 4)

/* Size for the TX/RX ringbuffers */
#define TX_RING_ENTRIES			8
#define RX_RING_ENTRIES			8

/* Length of the frame size preceeding transmitted/received frames */
#define FRAME_SIZE_LENGTH		sizeof(u32)
/* Length of the timestamp preceeding received frames */
#define RX_TIMESTAMP_LENGTH		sizeof(u32)

/* Values in registers are word addresses */
#define DESC_SHIFT			2
#define DESC_TO_CPU(x)			((x) << DESC_SHIFT)
#define CPU_TO_DESC(x)			((x) >> DESC_SHIFT)

struct hdoip_descriptor {
	u32 start;
	u32 stop;
	u32 write;
	u32 read;
};

static inline void hdoip_descriptor_init(struct hdoip_descriptor *desc,
                                         u32 start, u32 len)
{
	desc->start = start;
	desc->stop = start + len;
	desc->read = start;
	desc->write = start;
}

#if 0
struct hdoip_descriptors {
	/* CPU descriptors */
	struct hdoip_descriptor cpu;
	/* Video stream descriptors */
	struct hdoip_descriptor vid;
	/* Audio stream descriptors */
	struct hdoip_descriptor aud;
};
#endif

#endif /* _HDOIP_H_ */
