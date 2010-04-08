/*
 * Ethernet driver for the Altera TSE MAC using custom DMAs in the HD over IP
 * design.
 *
 * Copyright (c) 2010, Institute of Embedded Systems,
 *                     Zurich University of Applied Sciences
 * Copyright (c) 2010, Tobias Klauser <klto@zhaw.ch>
 */

#ifndef _HDOIP_ETHER_H_
#define _HDOIP_ETHER_H_

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>
#include <linux/phy.h>

/* Offsets into TSE MAC configuration register region */
#define TSE_MAC_COMMAND_CONFIG		0x0008
#define TSE_MAC_ADDR_0			0x000c
#define TSE_MAC_ADDR_1			0x0010
#define TSE_MAC_MTU			0x0014
#define TSE_MAC_PAUSE_QUANT		0x0018
#define TSE_MAC_RX_SECTION_EMPTY	0x001c
#define TSE_MAC_RX_SECTION_FULL		0x0020
#define TSE_MAC_TX_SECTION_EMPTY	0x0024
#define TSE_MAC_TX_SECTION_FULL		0x0028
#define TSE_MAC_RX_ALMOST_EMPTY		0x002c
#define TSE_MAC_RX_ALMOST_FULL		0x0030
#define TSE_MAC_TX_ALMOST_EMPTY		0x0034
#define TSE_MAC_TX_ALMOST_FULL		0x0038
#define TSE_MAC_MDIO_ADDR_0		0x003c
#define TSE_MAC_MDIO_ADDR_1		0x0040
/* MDIO spaces */
#define TSE_MAC_MDIO_0			0x0200
#define TSE_MAC_MDIO_1			0x0280

#define TSE_MAC_TX_CMD_STAT		0x00e8
#define TSE_MAC_RX_CMD_STAT		0x00ec

/* Bitmasks for the command_config register */
#define TSE_CMD_TX_EN			0x00000001
#define TSE_CMD_RX_EN			0x00000002
#define TSE_CMD_XON_GEN			0x00000004
#define TSE_CMD_ETH_SPEED		0x00000008
#define TSE_CMD_PROMIS_EN		0x00000010
#define TSE_CMD_PAD_EN			0x00000020
#define TSE_CMD_CRC_FWD			0x00000040
#define TSE_CMD_PAUSE_FWD		0x00000080
#define TSE_CMD_PAUSE_IGNORE		0x00000100
#define TSE_CMD_TX_ADDR_INS		0x00000200
#define TSE_CMD_HD_EN			0x00000400
#define TSE_CMD_EXCESS_COL		0x00000800
#define TSE_CMD_LATE_COL		0x00001000
#define TSE_CMD_SW_RESET		0x00002000
#define TSE_CMD_MHASH_SEL		0x00004000
#define TSE_CMD_LOOP_EN			0x00008000
/* bits 18-16: TX_ADDR_SEL */
#define TSE_CMD_TX_ADDR_SEL_MASK	0x00070000
#define TSE_CMD_MAGIC_EN		0x00080000
#define TSE_CMD_SLEEP			0x00100000
#define TSE_CMD_WAKEUP			0x00200000
#define TSE_CMD_NO_LENGTH_CHECK		0x01000000
#define TSE_CMD_ENA_10			0x02000000
#define TSE_CMD_RX_ERR_DISC		0x04000000
#define TSE_CMD_DISABLE_RD_TIMEOUT	0x08000000
/* bits 27-30: reserved */
#define TSE_CMD_CNT_RESET		0x80000000

/* Bitmasks for the tx_cmd_stat register */
#define TSE_TX_CMD_STAT_OMIT_CRC	0x00020000
#define TSE_TX_CMD_STAT_TX_SHIFT16	0x00040000

/* Bitmask for the rx_cmd_stat register */
#define TSE_RX_CMD_STAT_RX_SHIFT16	0x02000000

/* Default values for MAC registers */
#define HDOIP_DEFAULT_MTU		1518
#define HDOIP_MII_ID			0
#define HDOIP_PHY_ID			0x12

/* Default values for TSE MAC parameters */
#define TSE_MAC_RX_FIFO_DEPTH		1024
#define TSE_MAC_TX_FIFO_DEPTH		1024

/* Various constants */
#define TSE_WDOG_COUNT_RESET		10000

struct hdoip_ether {
	/* PHY */
	struct phy_device *phydev;
	struct mii_bus *mdio_bus;
	int old_link;
	int old_duplex;
	int old_speed;

	/* Locks */
	spinlock_t rx_lock;
	spinlock_t tx_lock;

	/* MAC register base */
	void __iomem *tse_mac_base;

	/* Ethernet in/out register base */
	void __iomem *ethi_base;
	void __iomem *etho_base;

	/* TX/RX ringbuffers */
	void __iomem *tx_buf;
	void __iomem *rx_buf;
	dma_addr_t tx_buf_dma;
	dma_addr_t rx_buf_dma;

#if 0
	/* Ringbuffer descriptors */
	struct hdoip_descriptors tx_desc;
	struct hdoip_descriptors rx_desc;
#endif

	/* External I/O (PHY reset) */
	void __iomem *ext_pio_base;
};

#endif /* _HDOIP_ETHER_H_ */
