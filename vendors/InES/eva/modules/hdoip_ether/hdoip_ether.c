/*
 * Ethernet driver for the Altera TSE MAC using custom DMAs in the HD over IP
 * design.
 *
 * Loosely based on altera_tse.c and at91ether.c
 *
 * Copyright (c) 2010, Institute of Embedded Systems,
 *                     Zurich University of Applied Sciences
 * Copyright (c) 2010, Tobias Klauser <klto@zhaw.ch>
 */

#define DEBUG

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/cacheflush.h>

#include "hdoip.h"
#include "hdoip_ether.h"

#define DRV_NAME			"hdoip_ether"
#define DRV_VERSION			"0.0"

#define mark_entry()			pr_debug("entering %s\n", __func__)
#define mark_leave()			pr_debug("leaving %s\n", __func__)

/*
 * The default MAC address to set in case the bootloader did not set a valid MAC
 * address.
 */
static const u8 mac_addr_default[ETH_ALEN] =
	{ 0x00, 0x15, 0x12, 0x00, 0x00, 0x42 };

/*
 * HD over IP board specific: Perform a hardware reset.
 */
static int hdoip_ether_board_reset(struct hdoip_ether *hde)
{
	u32 regval = ioread32(hde->ext_pio_base);

	iowrite32(regval & 0xFE, hde->ext_pio_base);
	udelay(50000);
	iowrite32(regval | 0x01, hde->ext_pio_base);
	udelay(50000);

	return 0;
}

/*
 * Read from a TSE MAC register.
 */
static inline u32 hdoip_mac_read(struct hdoip_ether *hde, unsigned int reg)
{
	return ioread32(hde->tse_mac_base + reg);
}

/*
 * Write to a TSE MAC register.
 */
static inline void hdoip_mac_write(struct hdoip_ether *hde, unsigned int reg, u32 value)
{
	iowrite32(value, hde->tse_mac_base + reg);
}

/*
 * Check whether writing len bytes to descriptor desc would be a valid
 * operation.
 */
static inline bool hdoip_desc_accepts(struct hdoip_descriptor *desc, u32 len)
{
	u32 new_write = desc->write + len;

	if (desc->write < desc->read && new_write < desc->read)
		return true;
	if (desc->write >= desc->read && new_write >= desc->read
	    && !(desc->read == desc->start && new_write > desc->stop))
		return true;

	return false;
}

/*
 * Read from eth_st_in register.
 */
static inline u32 hdoip_ethi_read(struct hdoip_ether *hde, unsigned int reg)
{
	return ioread32(hde->ethi_base + reg);
}

/*
 * Write to eth_st_in register.
 */
static inline void hdoip_ethi_write(struct hdoip_ether *hde, unsigned reg,
                                    u32 value)
{
	iowrite32(value, hde->ethi_base + reg);
}

/*
 * Start the eth_st_in DMA controller state machine.
 */
static inline void hdoip_ethi_start(struct hdoip_ether *hde)
{
	u32 regval = hdoip_ethi_read(hde, ETHIO_CONFIG);

	regval |= ETHI_CONFIG_RUN;
	hdoip_ethi_write(hde, ETHIO_CONFIG, regval);
}

/*
 * Stop the eth_st_in DMA controller state machine.
 */
static inline void hdoip_ethi_stop(struct hdoip_ether *hde)
{
	u32 regval = hdoip_ethi_read(hde, ETHIO_CONFIG);

	regval &= ~ETHI_CONFIG_RUN;
	hdoip_ethi_write(hde, ETHIO_CONFIG, regval);
}

/*
 * Update eth_st_in descriptor struct with values from the CPU descriptor
 * registers.
 */
static inline void hdoip_ethi_desc_get_cpu(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	desc->start = hdoip_ethi_read(hde, ETHIO_CPU_START_DESC);
	desc->stop = hdoip_ethi_read(hde, ETHIO_CPU_STOP_DESC);
	desc->write = hdoip_ethi_read(hde, ETHIO_CPU_WRITE_DESC);
	desc->read = hdoip_ethi_read(hde, ETHIO_CPU_READ_DESC);
}

/*
 * Update eth_st_in CPU descriptor registers with values from the descriptor
 * struct.
 */
static inline void hdoip_ethi_desc_set_cpu(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	hdoip_ethi_write(hde, ETHIO_CPU_START_DESC, desc->start);
	hdoip_ethi_write(hde, ETHIO_CPU_STOP_DESC, desc->stop);
	hdoip_ethi_write(hde, ETHIO_CPU_WRITE_DESC, desc->write);
	hdoip_ethi_write(hde, ETHIO_CPU_READ_DESC, desc->read);
}

/*
 * Update eth_st_in descriptor struct with values from the video descriptor
 * registers.
 */
static inline void hdoip_ethi_desc_get_vid(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	desc->start = hdoip_ethi_read(hde, ETHIO_VID_START_DESC);
	desc->stop = hdoip_ethi_read(hde, ETHIO_VID_STOP_DESC);
	desc->write = hdoip_ethi_read(hde, ETHI_VID_WRITE_DESC);
}

/*
 * Update eth_st_in video descriptor registers with values from the descriptor
 * struct.
 */
static inline void hdoip_ethi_desc_set_vid(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	hdoip_ethi_write(hde, ETHIO_VID_START_DESC, desc->start);
	hdoip_ethi_write(hde, ETHIO_VID_STOP_DESC, desc->stop);
	hdoip_ethi_write(hde, ETHI_VID_WRITE_DESC, desc->write);
}

/*
 * Update eth_st_in descriptor struct with values from the audio descriptor
 * registers.
 */
static inline void hdoip_ethi_desc_get_aud(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	desc->start = hdoip_ethi_read(hde, ETHIO_AUD_START_DESC);
	desc->stop = hdoip_ethi_read(hde, ETHIO_AUD_STOP_DESC);
	desc->write = hdoip_ethi_read(hde, ETHI_AUD_WRITE_DESC);
}

/*
 * Update eth_st_in audio descriptor registers with values from the descriptor
 * struct.
 */
static inline void hdoip_ethi_desc_set_aud(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	hdoip_ethi_write(hde, ETHIO_AUD_START_DESC, desc->start);
	hdoip_ethi_write(hde, ETHIO_AUD_STOP_DESC, desc->stop);
	hdoip_ethi_write(hde, ETHI_AUD_WRITE_DESC, desc->write);
}

/*
 * Initialize eth_st_in
 */
static inline void hdoip_ethi_init(struct hdoip_ether *hde,
                                   u16 dma_burst_size,
                                   __be32 addr)
{
	u32 regval;

	/* Stop the state machines */
	hdoip_ethi_stop(hde);

	/* Reset IRQ and statistics register */
	regval = hdoip_ethi_read(hde, ETHIO_CONFIG);
	regval &= ~ETHI_CONFIG_IRQ1_RESET;
	regval |= ETHI_CONFIG_STATS_RESET;
	hdoip_ethi_write(hde, ETHIO_CONFIG, regval);

	/* Setup descriptors */
	hdoip_ethi_desc_set_cpu(hde, &hde->rx_desc.cpu);
	hdoip_ethi_desc_set_vid(hde, &hde->rx_desc.vid);
	hdoip_ethi_desc_set_aud(hde, &hde->rx_desc.aud);

	/* Setup DMA... */
	regval = hdoip_ethi_read(hde, ETHI_DMA_CONFIG);
	/* ...burst size */
	regval |= (dma_burst_size << ETHI_CONFIG_DMA_BURST_SIZE_INDEX)
	          & ETHI_CONFIG_DMA_BURST_SIZE_MASK;
	hdoip_ethi_write(hde, ETHI_DMA_CONFIG, regval);

	/* Setup IP Src and Dest filter */
	hdoip_ethi_write(hde, ETHI_IP_FILTER, 0xc0a801c8);
	hdoip_ethi_write(hde, ETHI_IP_FILTER_SRC, 0xc0a80165);
}

/*
 * Read from eth_st_out register.
 */
static inline u32 hdoip_etho_read(struct hdoip_ether *hde, unsigned int reg)
{
	return ioread32(hde->etho_base + reg);
}

/*
 * Write to eth_st_out register.
 */
static inline void hdoip_etho_write(struct hdoip_ether *hde, unsigned reg,
                                    u32 value)
{
	iowrite32(value, hde->etho_base + reg);
}

/*
 * Start the eth_st_out DMA controller and priorization state machine.
 */
static inline void hdoip_etho_start(struct hdoip_ether *hde)
{
	u32 regval = hdoip_etho_read(hde, ETHIO_CONFIG);

	regval |= ETHO_CONFIG_RUN; // | ETHO_CONFIG_PRIO;
	regval &= ~ETHO_CONFIG_PRIO;
	hdoip_etho_write(hde, ETHIO_CONFIG, regval);
}

/*
 * Stop the eth_st_out DMA controller and priorization state machine.
 */
static inline void hdoip_etho_stop(struct hdoip_ether *hde)
{
	u32 regval = hdoip_etho_read(hde, ETHIO_CONFIG);

	regval &= ~(ETHO_CONFIG_RUN); //| ETHO_CONFIG_PRIO);
	hdoip_etho_write(hde, ETHIO_CONFIG, regval);
}

/*
 * Update eth_st_out descriptor struct with values from the CPU descriptor
 * registers.
 */
static inline void hdoip_etho_desc_get_cpu(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	desc->start = hdoip_etho_read(hde, ETHIO_CPU_START_DESC) << 2;
	desc->stop = hdoip_etho_read(hde, ETHIO_CPU_STOP_DESC) << 2;
	desc->write = hdoip_etho_read(hde, ETHIO_CPU_WRITE_DESC) << 2;
	desc->read = hdoip_etho_read(hde, ETHIO_CPU_READ_DESC) << 2;
}

/*
 * Update eth_st_out CPU descriptor registers with values from the descriptor
 * struct.
 */
static inline void hdoip_etho_desc_set_cpu(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	hdoip_etho_write(hde, ETHIO_CPU_START_DESC, desc->start >> 2);
	hdoip_etho_write(hde, ETHIO_CPU_STOP_DESC, desc->stop >> 2);
	hdoip_etho_write(hde, ETHIO_CPU_WRITE_DESC, desc->write >> 2);
	hdoip_etho_write(hde, ETHIO_CPU_READ_DESC, desc->read >> 2);
}

/*
 * Update eth_st_out descriptor struct with values from the video descriptor
 * registers.
 */
static inline void hdoip_etho_desc_get_vid(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	desc->start = hdoip_etho_read(hde, ETHIO_VID_START_DESC);
	desc->stop = hdoip_etho_read(hde, ETHIO_VID_STOP_DESC);
	desc->read = hdoip_etho_read(hde, ETHO_VID_READ_DESC);
}

/*
 * Update eth_st_out video descriptor registers with values from the descriptor
 * struct.
 */
static inline void hdoip_etho_desc_set_vid(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	hdoip_etho_write(hde, ETHIO_VID_START_DESC, desc->start);
	hdoip_etho_write(hde, ETHIO_VID_STOP_DESC, desc->stop);
	hdoip_etho_write(hde, ETHO_VID_READ_DESC, desc->read);
}

/*
 * Update eth_st_out descriptor struct with values from the audio descriptor
 * registers.
 */
static inline void hdoip_etho_desc_get_aud(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	desc->start = hdoip_etho_read(hde, ETHIO_AUD_START_DESC);
	desc->stop = hdoip_etho_read(hde, ETHIO_AUD_STOP_DESC);
	desc->read = hdoip_etho_read(hde, ETHO_AUD_READ_DESC);
}

/*
 * Update eth_st_out audio descriptor registers with values from the descriptor
 * struct.
 */
static inline void hdoip_etho_desc_set_aud(struct hdoip_ether *hde,
                                           struct hdoip_descriptor *desc)
{
	hdoip_etho_write(hde, ETHIO_AUD_START_DESC, desc->start);
	hdoip_etho_write(hde, ETHIO_AUD_STOP_DESC, desc->stop);
	hdoip_etho_write(hde, ETHO_AUD_READ_DESC, desc->read);
}

/*
 * Initialize the eth_st_out block.
 *
 * @hde:             private data of the device to operate on
 * @dma_burst_size:  DMA burst size
 * @dma_fifo_thresh: DMA fifo threshold
 */
static inline void hdoip_etho_init(struct hdoip_ether *hde,
                                   u16 dma_burst_size, u16 dma_fifo_thresh)
{
	u32 regval;

	/* Stop the state machines */
	hdoip_etho_stop(hde);

	/* Setup descriptors */
	hdoip_etho_desc_set_cpu(hde, &hde->tx_desc.cpu);
	hdoip_etho_desc_set_vid(hde, &hde->tx_desc.vid);
	hdoip_etho_desc_set_aud(hde, &hde->tx_desc.aud);

	/* Setup DMA... */
	regval = hdoip_etho_read(hde, ETHO_DMA_CONFIG);
	/* ...burst size */
	regval &= ~ETHO_CONFIG_DMA_BURST_SIZE_MASK;
	regval |= (dma_burst_size << ETHO_CONFIG_DMA_BURST_SIZE_INDEX)
	          & ETHO_CONFIG_DMA_BURST_SIZE_MASK;
	/* ...FIFO threshold */
	regval &= ~ETHO_CONFIG_DMA_FIFO_THRESH_MASK;
	regval |= (dma_fifo_thresh << ETHO_CONFIG_DMA_FIFO_THRESH_INDEX)
	          & ETHO_CONFIG_DMA_FIFO_THRESH_MASK;
	hdoip_etho_write(hde, ETHO_DMA_CONFIG, regval);

	regval = hdoip_etho_read(hde, ETHO_DMA_CONFIG);
}

/*
 * Read from a MDIO register.
 */
static int hdoip_ether_mdio_read(struct mii_bus *mii, int mii_id, int reg)
{
	struct hdoip_ether *hde = mii->priv;
	u32 value;

	/* Check register offset is within range */
	if (reg < 0 || reg > 0x7c)
		return -1;

	/* Set MDIO address */
	hdoip_mac_write(hde, TSE_MAC_MDIO_ADDR_0, mii_id);
	/* Get data from the MDIO register */
	value = hdoip_mac_read(hde, TSE_MAC_MDIO_0 + (reg * 4));

	/* only bits 0-15 are significant */
	return (int) (value & 0xffff);
}

/*
 * Write to a MDIO register.
 */
static int hdoip_ether_mdio_write(struct mii_bus *mii, int mii_id, int reg, u16 value)
{
	struct hdoip_ether *hde = mii->priv;

	/* Check register offset is within range */
	if (reg < 0 || reg > 0x7c)
		return -1;

	/* Set MDIO address */
	hdoip_mac_write(hde, TSE_MAC_MDIO_ADDR_0, mii_id);
	/* Write data to MDIO register */
	hdoip_mac_write(hde, TSE_MAC_MDIO_0 + (reg * 4), value);

	return 0;
}

/*
 * HD over IP board specific: Perform a PHY hardware reset.
 */
static int hdoip_ether_mdio_reset(struct mii_bus *mii)
{
	struct hdoip_ether *hde = mii->priv;

	hdoip_ether_board_reset(hde);

	return 0;
}

/* XXX: For debugging only */
#ifdef DEBUG
static void hdoip_ether_print_status(struct hdoip_ether *hde)
{
	u32 regval;

	pr_debug("%s PHY register status\n", DRV_NAME);
	regval = hdoip_mac_read(hde, 0x204);
	pr_debug(" Status register                : %04x\n\n", regval);
	pr_debug(" Jabber condition               : %s\n", regval & 0x0002 ? "detected" : "not detected");
	pr_debug(" Copper link status             : %s\n", regval & 0x0004 ? "up" : "down");
	pr_debug(" Copper remote fault condition  : %s\n", regval & 0x0010 ? "detected" : "not detected");
	pr_debug(" Autonegotiation process        : %s\n", regval & 0x0020 ? "completed" : "not completed");
	pr_debug(" 10 Mbps Half-Duplex            : %s\n", regval & 0x0800 ? "available" : "not available");
	pr_debug(" 10 Mbps Full-Duplex            : %s\n", regval & 0x1000 ? "available" : "not available");
	pr_debug(" 100 Mbps Half-Duplex           : %s\n", regval & 0x2000 ? "available" : "not available");
	pr_debug(" 100 Mbps Full-Duplex           : %s\n", regval & 0x4000 ? "available" : "not available");

	pr_debug("\n===\n\n");

	regval = hdoip_mac_read(hde, 0x244);
	pr_debug(" PHY specific status register   : %04x\n\n", regval);
	pr_debug(" Jabber (real time)             : %s\n", regval & 0x0001 ? "jabber" : "no jabber");
	pr_debug(" Polarity (real time)           : %s\n", regval & 0x0002 ? "reversed" : "normal");
	pr_debug(" Receive Pause                  : %s\n", regval & 0x0004 ? "enabled" : "disabled");
	pr_debug(" Transmit Pause                 : %s\n", regval & 0x0008 ? "enabled" : "disabled");
	pr_debug(" Copper Energy Detect Status    : %s\n", regval & 0x0010 ? "sleep" : "active");
	pr_debug(" Downshift Status               : %s\n", regval & 0x0020 ? "downshift" : "no downshift");
	pr_debug(" Link (real time)               : %s\n", regval & 0x0400 ? "up" : "down");
	pr_debug(" Speed and duplex resolved      : %s\n", regval & 0x0800 ? "resolved" : "not resolved");
	pr_debug(" Page received                  : %s\n", regval & 0x1000 ? "received" : "not received");
	pr_debug(" Duplex                         : %s\n", regval & 0x2000 ? "full" : "half");

	pr_debug(" Speed                          : ");
	switch (regval & 0xC000 >> 14) {
	case 0:
		pr_cont("10 Mbps\n");
		break;
	case 1:
		pr_cont("100 Mbps\n");
		break;
	case 2:
		pr_cont("1000 Mbps\n");
		break;
	default:
		pr_cont("reserved\n");
		break;
	}

	pr_debug("\n===\n\n");

	regval = hdoip_mac_read(hde, 0x26C);
	pr_debug(" Extended PHY specific status  : %04x\n", regval);
	pr_debug(" HWCFG_MODE                    : ");
	switch (regval & 0xF) {
		case 0xF:
			pr_cont("GMII to copper");
			break;
		case 0xB:
			pr_cont("RGMII/Modified MII to Copper");
			break;
		case 0x7:
			pr_cont("GMII to Fiber");
		default:
			pr_cont("reserved");
			break;
	}
	pr_cont("\n");
	pr_debug(" Fiber/copper resolution       : %s\n", regval & 0x2000 ? "fiber" : "copper");
	pr_debug(" Fiber/copper auto selection   : %s\n", regval & 0x8000 ? "disabled" : "enabled");
}
#else
static void hdoip_ether_print_status(struct hdoip_ether *hde)
{
}
#endif

static void hdoip_ether_get_drvinfo(struct net_device *dev,
                                    struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, DRV_NAME, sizeof(info->driver));
	strlcpy(info->version, DRV_VERSION, sizeof(info->version));
	strlcpy(info->bus_info, "avalon", sizeof(info->bus_info));
}

static const struct ethtool_ops hdoip_ether_ethtool_ops = {
	.get_drvinfo	= hdoip_ether_get_drvinfo,
};

static struct net_device_stats *hdoip_ether_get_stats(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	struct net_device_stats *stats = &dev->stats;

	if (netif_running(dev)) {
		stats->rx_packets = hdoip_ethi_read(hde, ETHI_CPU_PACKET_COUNT);
		stats->rx_bytes	= hdoip_ethi_read(hde, ETHI_CPU_DATA_COUNT);
		stats->rx_crc_errors = hdoip_ethi_read(hde, ETHI_CPU_CRC_ERR_COUNT);
	}

	return stats;
}

static void hdoip_ether_set_multicast_list(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	u32 value;

	/* Promiscuous mode */
	value = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	if (dev->flags & IFF_PROMISC)
		value |= TSE_CMD_PROMIS_EN;
	else
		value &= ~TSE_CMD_PROMIS_EN;
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG, value);
}

/*
 * Read MAC address from hardware.
 */
static void __init hdoip_ether_get_mac_address(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	u8 addr[ETH_ALEN];
	u32 mac0, mac1;

	mac0 = hdoip_mac_read(hde, TSE_MAC_ADDR_0);
	mac1 = hdoip_mac_read(hde, TSE_MAC_ADDR_1);

	addr[0] = (mac0 & 0xff);
	addr[1] = (mac0 & 0xff00) >> 8;
	addr[2] = (mac0 & 0xff0000) >> 16;
	addr[3] = (mac0 & 0xff000000) >> 24;
	addr[4] = (mac1 & 0xff);
	addr[5] = (mac1 & 0xff00) >> 8;

	memcpy(dev->dev_addr, &addr, ETH_ALEN);
}

/*
 * Set the hardware MAC address.
 */
static void __hdoip_ether_set_mac_address(struct hdoip_ether *hde,
                                          unsigned char *dev_addr)
{
	hdoip_mac_write(hde, TSE_MAC_ADDR_0,
	                (dev_addr[3] << 24) | (dev_addr[2] << 16) |
			(dev_addr[1] << 8) | dev_addr[0]);
	hdoip_mac_write(hde, TSE_MAC_ADDR_1,
			(dev_addr[5] << 8) | dev_addr[4]);

	/* XXX: There are supplemental MAC address registers available in the
	 * TSE MAC configuration region. Set them too? */
}

/*
 * Set the MAC address in the device structure and in hardware.
 */
static int hdoip_ether_set_mac_address(struct net_device *dev, void *addr)
{
	struct hdoip_ether *hde = netdev_priv(dev);

	if (addr) {
		struct sockaddr *sa = addr;

		if (!is_valid_ether_addr(sa->sa_data))
			return -EADDRNOTAVAIL;

		memcpy(dev->dev_addr, sa->sa_data, dev->addr_len);
	} else {
		/* Set the default address */
		memcpy(dev->dev_addr, mac_addr_default, dev->addr_len);
	}

	/* Write the new address to hardware */
	__hdoip_ether_set_mac_address(hde, dev->dev_addr);

	return 0;
}

/*
 * Change the Maximum Transfer Unit (MTU) of the device.
 */
static int hdoip_ether_change_mtu(struct net_device *dev, int mtu)
{
	/* Changing MTU not supported yet */
	return 0;
}

static void hdoip_ether_adjust_link(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	struct phy_device *phydev = hde->phydev;
	u32 regval;
	bool new_state = false;

	if (phydev->link != hde->old_link) {
		new_state = true;
		hde->old_link = phydev->link;
	}

	if (phydev->link) {
		regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);

		if (phydev->duplex != hde->old_duplex) {
			new_state = true;
			hde->old_duplex = phydev->duplex;

			if (!phydev->duplex)
				regval |= TSE_CMD_HD_EN;
			else
				regval &= ~TSE_CMD_HD_EN;
		}

		if (phydev->speed != hde->old_speed) {
			new_state = true;
			hde->old_speed = phydev->speed;

			switch (phydev->speed) {
			case 1000:
				regval |= TSE_CMD_ETH_SPEED;
				regval &= ~TSE_CMD_ENA_10;
				break;
			case 100:
				regval &= ~TSE_CMD_ETH_SPEED;
				regval &= ~TSE_CMD_ENA_10;
				break;
			case 10:
				regval &= ~TSE_CMD_ETH_SPEED;
				regval |= TSE_CMD_ENA_10;
				break;
			default:
				printk(KERN_WARNING "%s: Invalid speed value: %d\n", dev->name, phydev->speed);
				break;
			}
		}

		/* Only write if something has changed */
		if (new_state)
			hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG, regval);

		netif_carrier_on(dev);
	}

	if (new_state)
		phy_print_status(phydev);
}

static inline void hdoip_ether_write_frame(volatile u32 *dest, volatile u32 *src, size_t n)
{
	while (n) {
		iowrite32(cpu_to_be32(*src++), dest++);
		n--;
	}
}

#ifdef DEBUG
static void dump_desc(struct hdoip_ether *hde)
{
	pr_debug("TX CPU:\n");
	pr_debug("  start: 0x%08x\n", hdoip_etho_read(hde, ETHIO_CPU_START_DESC) << 2);
	pr_debug("  stop:  0x%08x\n", hdoip_etho_read(hde, ETHIO_CPU_STOP_DESC) << 2);
	pr_debug("  write: 0x%08x\n", hdoip_etho_read(hde, ETHIO_CPU_WRITE_DESC) << 2);
	pr_debug("  read:  0x%08x\n", hdoip_etho_read(hde, ETHIO_CPU_READ_DESC) << 2);

#if 0
	pr_debug("RX CPU:\n");
	pr_debug("  start: 0x%08x\n", hde->rx_desc.cpu.start);
	pr_debug("  stop:  0x%08x\n", hde->rx_desc.cpu.stop);
	pr_debug("  write: 0x%08x\n", hde->rx_desc.cpu.write);
	pr_debug("  read:  0x%08x\n", hde->rx_desc.cpu.read);
#endif
}
#endif

/*
 * Transmit a packet.
 *
 * Called by the network subsystem of the Linux Kernel.
 */
static int hdoip_ether_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	struct hdoip_descriptor *tx_desc = &hde->tx_desc.cpu;
	unsigned long flags;
	u32 *tx_buf;
	u32 len = skb->len;
	u32 newlen;
	int i;

	mark_entry();

	/* check oversized frames */
	if (unlikely(len > MAX_PKT_SIZE)) {
		dev->stats.tx_dropped++;
		dev_kfree_skb(skb);
		printk(KERN_ERR "%s: Packet too large\n", dev->name);
		return NETDEV_TX_OK;
	}

	/* pad short packets */
	if (len < ETH_ZLEN + 4) {
		if (skb_padto(skb, ETH_ZLEN + 4)) {
			printk(KERN_ERR "%s: Packet padding failed\n", dev->name);
			return NETDEV_TX_OK;
		}
		len = ETH_ZLEN + 4;
	}

	/* align frame data to 32bit boundaries */
	if ((unsigned long) skb->data & 0x2) {
		pr_debug("skb->data not aligned\n");
		skb_push(skb, 2);
		len = skb->len;
	}

	/* pad packet to multiple of 32 bits */
	newlen = len + 3;
	newlen &= ~3UL;
	if (newlen != len) {
		pr_debug("padding to multiple of 32bit (len = %lu, newlen = %lu)\n", len, newlen);
		if (skb_padto(skb, newlen)) {
			printk(KERN_ERR "%s: Packet padding failed\n", dev->name);
			return NETDEV_TX_OK;
		}
		len = newlen;
	}

	spin_lock_irqsave(&hde->tx_lock, flags);

	/* Is there space in the ringbuffer? */
	if (!hdoip_desc_accepts(tx_desc, len)) {
		/* The ringbuffer is full */
		netif_stop_queue(dev);
		spin_unlock_irqrestore(&hde->tx_lock, flags);
		printk(KERN_ERR "%s: TX Ringbuffer is full\n", dev->name);
		return NETDEV_TX_BUSY;
	}

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += len;

	pr_debug("before:\n");
	dump_desc(hde);
	tx_buf = (u32 *) tx_desc->write;
	/* First 32 bits of the ringbuffer entry hold the frame size */
	iowrite32(cpu_to_le32(len), tx_buf);
	hdoip_ether_write_frame(tx_buf + 1, (u32 *) skb->data, len / sizeof(u32));
	/* Make sure everything gets written to memory */
	flush_dcache_range((unsigned long) tx_buf, (unsigned long) (((u8 *) tx_buf) + len));

#if 0
	tx_buf = (u32 *) ((unsigned long) tx_buf | 0x80000000);
	for (i = 0; i < (len + 4) / sizeof(u32); i++) {
		printk(" %08x", tx_buf[i]);
		if (i && i % 16 == 0)
			printk("\n");
	}
#endif

	/* Update write pointer (with wrap around) */
	tx_desc->write += len + sizeof(u32);
	if (tx_desc->write > tx_desc->stop)
		tx_desc->write = tx_desc->start;

	/* Write descriptors */
	hdoip_etho_desc_set_cpu(hde, tx_desc);

	pr_debug("after:\n");
	dump_desc(hde);
	spin_unlock_irqrestore(&hde->tx_lock, flags);

	dev->trans_start = jiffies;

	mark_leave();

	return NETDEV_TX_OK;
}

/*
 * Receive a packet.
 *
 * Called from interrupt context.
 */
static void hdoip_ether_rx(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	struct sk_buff *skb;
	unsigned int len;
	unsigned long flags;

	spin_lock_irqsave(&hde->rx_lock, flags);

	/* Update the RX descriptor struct */
	hdoip_ethi_desc_get_cpu(hde, &hde->rx_desc.cpu);

	/* Get the frame length (first byte) */
	len = ioread32(hde->rx_desc.cpu.read);
	pr_debug("RX frame: %d bytes\n", len);

	skb = dev_alloc_skb(len + 2);
	if (!skb) {
		spin_unlock_irqrestore(&hde->rx_lock, flags);
		dev->stats.rx_dropped++;
		printk(KERN_NOTICE "%s: Memory squeeze, dropping packet.\n", dev->name);
		return;
	}

	/* TODO */

	spin_unlock_irqrestore(&hde->rx_lock, flags);
}

/*
 * MAC interrupt handler.
 */
static irqreturn_t hdoip_ether_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct hdoip_ether *hde = netdev_priv(dev);
	u32 status;

	pr_debug("Got an interrupt\n");

	status = hdoip_ethi_read(hde, ETHIO_STATUS);
	/* Check the status register for the IRQ */
	if (!(status & ETHI_STATUS_IRQ1_FLAG))
		return IRQ_HANDLED;

	/* Reset IRQ */
	hdoip_ethi_write(hde, ETHIO_STATUS, status | ETHI_CONFIG_IRQ1_RESET);

	pr_debug("There was really an interrupt\n");

	hdoip_ether_rx(dev);

	return IRQ_HANDLED;
}

/*
 * Initialize the Altera Triple Speed MAC.
 */
static int hdoip_ether_mac_init(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	u32 regval;
	unsigned int wdog_cnt = 0;

	/* Software reset the TSE MAC */
	regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG, regval | TSE_CMD_SW_RESET);

	/* Wait for the MAC to become active again */
	while (hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG) & TSE_CMD_SW_RESET &&
	       wdog_cnt < TSE_WDOG_COUNT_RESET) {
		udelay(1);
		++wdog_cnt;
	}

	if (wdog_cnt >= TSE_WDOG_COUNT_RESET) {
		printk(KERN_ERR "%s: Failed to reset the TSE MAC: Reset bit was never cleared.\n",
		       dev->name);
		return -ENODEV;
	}

	/* Set default MTU */
	dev->mtu = HDOIP_DEFAULT_MTU;
	hdoip_mac_write(hde, TSE_MAC_MTU, HDOIP_DEFAULT_MTU);

	/* Set FIFO thresholds */
	hdoip_mac_write(hde, TSE_MAC_RX_SECTION_EMPTY, TSE_MAC_RX_FIFO_DEPTH - 16);
	hdoip_mac_write(hde, TSE_MAC_RX_SECTION_FULL, 0);	/* XXX: This is 0 in the test software, but should be 16 according to datasheet, why? */
	hdoip_mac_write(hde, TSE_MAC_TX_SECTION_EMPTY, TSE_MAC_TX_FIFO_DEPTH - 16);
	hdoip_mac_write(hde, TSE_MAC_TX_SECTION_FULL, 16);
	hdoip_mac_write(hde, TSE_MAC_RX_ALMOST_EMPTY, 8);
	hdoip_mac_write(hde, TSE_MAC_RX_ALMOST_FULL, 8);
	hdoip_mac_write(hde, TSE_MAC_TX_ALMOST_EMPTY, 8);
	hdoip_mac_write(hde, TSE_MAC_TX_ALMOST_FULL, 3);	/* XXX: This is 3 in the test software and altera_tse.c, why? */

	/*
	 * Enable CRC insertion and IP payload alignment in TX.
	 *
	 * "Align packet header to 32-bit boundary" must be enabled for the TSE
	 * MAC in SOPC Builder. Otherwise the driver won't work.
	 */
	regval = hdoip_mac_read(hde, TSE_MAC_TX_CMD_STAT);
	regval |= TSE_TX_CMD_STAT_TX_SHIFT16;
	regval &= ~TSE_TX_CMD_STAT_OMIT_CRC;
	hdoip_mac_write(hde, TSE_MAC_TX_CMD_STAT, regval);

	/*
	 * Enable IP payload alignment in RX.
	 *
	 * "Align packet header to 32-bit boundary" must be enabled for the TSE
	 * MAC in SOPC Builder. Otherwise the driver won't work.
	 */
	regval = hdoip_mac_read(hde, TSE_MAC_RX_CMD_STAT);
	hdoip_mac_write(hde, TSE_MAC_RX_CMD_STAT, regval | TSE_RX_CMD_STAT_RX_SHIFT16);

	/* Set command_config register */
	regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	regval = regval & (~TSE_CMD_CRC_FWD)		/* don't forward CRC */
	                & (~TSE_CMD_LOOP_EN)		/* no loopback */
	                & (~TSE_CMD_TX_ADDR_INS)	/* don't set MAC address on TX */
	                & (~TSE_CMD_RX_ERR_DISC)	/* don't discard erroneous frames */
	                & (~TSE_CMD_NO_LENGTH_CHECK);	/* check payload length against length/type field on RX */
	regval = regval | TSE_CMD_ETH_SPEED		/* enable Gigabit ethernet operation */
	                | TSE_CMD_PROMIS_EN		/* promiscous enable */
	                | TSE_CMD_PAD_EN;		/* enable padding removal */
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG, regval);

	/* Reset statistics counter */
	regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG, regval | TSE_CMD_CNT_RESET);

	return 0;
}

/*
 * Open the ethernet interface.
 */
static int hdoip_ether_open(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	int ret = 0;
	u32 regval;

	/* Check and hard set the MAC address, in case the user has changed it */
	if (!is_valid_ether_addr(dev->dev_addr))
		return -EADDRNOTAVAIL;

	__hdoip_ether_set_mac_address(hde, dev->dev_addr);

	ret = hdoip_ether_mac_init(dev);
	if (ret)
		return ret;

	/* Setup memory for TX/RX ringbuffers */
	hde->tx_buf = dma_alloc_coherent(&dev->dev, TX_RING_ENTRIES * MAX_PKTBUF_SIZE,
	                                 &hde->tx_buf_dma, GFP_KERNEL);
	if (!hde->tx_buf)
		return -ENOMEM;

	hde->rx_buf = dma_alloc_coherent(&dev->dev, RX_RING_ENTRIES * MAX_PKTBUF_SIZE,
	                                 &hde->rx_buf_dma, GFP_KERNEL);
	if (!hde->rx_buf) {
		dma_free_coherent(&dev->dev, TX_RING_ENTRIES * MAX_PKTBUF_SIZE,
		                  hde->tx_buf, hde->tx_buf_dma);
		return -ENOMEM;
	}

	/* Setup CPU descriptors */
	hde->tx_desc.cpu.start = (u32) hde->tx_buf_dma;
	hde->tx_desc.cpu.stop = (u32) (hde->tx_buf_dma
	                               + (TX_RING_ENTRIES * MAX_PKTBUF_SIZE)
	                               - MAX_PKT_SIZE);
	hde->tx_desc.cpu.write = hde->tx_desc.cpu.start;
	hde->tx_desc.cpu.read = hde->tx_desc.cpu.start;

	hde->rx_desc.cpu.start = (u32) hde->rx_buf_dma;
	hde->rx_desc.cpu.stop = (u32) (hde->rx_buf_dma
	                               + (RX_RING_ENTRIES * MAX_PKTBUF_SIZE)
	                               - MAX_PKT_SIZE);
	hde->rx_desc.cpu.write = (u32) hde->rx_desc.cpu.start;
	hde->rx_desc.cpu.read = (u32) hde->rx_desc.cpu.start;

	/* XXX: Initalize the audio/video descriptors */
	hde->tx_desc.vid.start = (u32) 0;
	hde->tx_desc.vid.stop = (u32) 0+ 0x4000;
	hde->tx_desc.vid.write = (u32) 0;
	hde->tx_desc.vid.read = (u32) 0;
	iowrite32(hde->tx_desc.vid.start, ACB_VID_ST_IN_BASE + 0x40);

	hde->tx_desc.aud.start = (u32) 0;
	hde->tx_desc.aud.stop = (u32) 0 + 0x4000;
	hde->tx_desc.aud.write = (u32) 0;
	hde->tx_desc.aud.read = (u32) 0;
	iowrite32(hde->tx_desc.aud.start, 0x80000000 + ACB_AUD_ST_IN_BASE + 0x94);

	hde->rx_desc.vid.start = (u32) 0;
	hde->rx_desc.vid.stop = (u32) 0x00440000 + 0x4000;
	hde->rx_desc.vid.write = (u32) 0x00440000;
	hde->rx_desc.vid.read = (u32) 0x00440000;

	hde->rx_desc.aud.start = (u32) 0x00450000;
	hde->rx_desc.aud.stop = (u32) 0x00450000 + 0x4000;
	hde->rx_desc.aud.write = (u32) 0x00450000;
	hde->rx_desc.aud.read = (u32) 0x00450000;

	/* Initialize ethernet in/out blocks */
	hdoip_etho_init(hde, ETHIO_DMA_BURST_SIZE, ETHO_DMA_FIFO_THRESH);
	hdoip_ethi_init(hde, ETHIO_DMA_BURST_SIZE, 0);

	/* Enable TX/RX on MAC */
	regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG,
	                regval | TSE_CMD_TX_EN | TSE_CMD_RX_EN);

	phy_start(hde->phydev);
	hdoip_etho_start(hde);
	hdoip_ethi_start(hde);
	netif_start_queue(dev);

	return 0;
}

static int hdoip_ether_close(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	u32 regval;

	/* Start the eth_st_in DMA controller */
	hdoip_ethi_stop(hde);

	/* Disable TX/RX on MAC */
	regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG,
	                regval & ~(TSE_CMD_TX_EN | TSE_CMD_RX_EN));

	/* Free ringbuffer memory */
	dma_free_coherent(&dev->dev, TX_RING_ENTRIES * MAX_PKTBUF_SIZE,
	                  hde->tx_buf, hde->tx_buf_dma);
	dma_free_coherent(&dev->dev, RX_RING_ENTRIES * MAX_PKTBUF_SIZE,
	                  hde->rx_buf, hde->rx_buf_dma);

	netif_stop_queue(dev);

	return 0;
}

static const struct net_device_ops hdoip_ether_netdev_ops = {
	.ndo_open		= hdoip_ether_open,
	.ndo_stop		= hdoip_ether_close,
	.ndo_start_xmit		= hdoip_ether_start_xmit,
	.ndo_set_multicast_list	= hdoip_ether_set_multicast_list,
	.ndo_set_mac_address	= hdoip_ether_set_mac_address,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_change_mtu		= hdoip_ether_change_mtu,
	.ndo_get_stats		= hdoip_ether_get_stats,
};

/* Device notifier for IP address change */

static int hdoip_ether_inetaddr_event(struct notifier_block *nb,
                                      unsigned long event, void *ptr)
{
	struct in_ifaddr *ifa = ptr;
	struct net_device *dev = ifa->ifa_dev ? ifa->ifa_dev->dev : NULL;
	struct hdoip_ether *hde;

	if (!dev || dev_net(dev) != &init_net)
		return NOTIFY_DONE;

	hde = netdev_priv(dev);

	switch (event) {
	case NETDEV_UP:
		//printk(KERN_INFO "IP address %pI4 set\n", &ifa->ifa_address);
		//hdoip_ethi_write(hde, ETHI_IP_FILTER, ifa->ifa_address);
		break;
	case NETDEV_DOWN:
		//printk(KERN_INFO "IP address %pI4 unset\n", &ifa->ifa_address);
		//hdoip_ethi_write(hde, ETHI_IP_FILTER, 0);
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block hdoip_ether_notifier = {
	.notifier_call = hdoip_ether_inetaddr_event,
};

/*
 * Initialize the PHY.
 */
static int __init hdoip_ether_phy_init(struct net_device *dev)
{
	struct phy_device *phydev;
	struct hdoip_ether *hde = netdev_priv(dev);
	char mii_id[MII_BUS_ID_SIZE];
	char phy_id[MII_BUS_ID_SIZE+3];

	snprintf(mii_id, MII_BUS_ID_SIZE, "%x", HDOIP_MII_ID);
	snprintf(phy_id, MII_BUS_ID_SIZE + 3, PHY_ID_FMT, mii_id, HDOIP_PHY_ID);

	hde->old_link = 0;
	hde->old_speed = 0;
	hde->old_duplex = -1;

	phydev = phy_connect(dev, phy_id, &hdoip_ether_adjust_link, 0,
	                     PHY_INTERFACE_MODE_RGMII_TXID);
	if (IS_ERR(phydev)) {
		printk(KERN_ERR "%s: Failed to attach to PHY at %s\n",
		       DRV_NAME, phy_id);
		return PTR_ERR(phydev);
	}

	phydev->autoneg = AUTONEG_ENABLE;
	/* Only Gbit supported */
	phydev->supported = SUPPORTED_1000baseT_Full;
	phydev->advertising = phydev->supported;

	hde->phydev = phydev;

	return 0;
}

static int __init hdoip_mii_probe(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	struct mii_bus *mdio_bus;
	int ret;

	mdio_bus = mdiobus_alloc();
	if (!mdio_bus)
		return -ENOMEM;

	mdio_bus->name = "HD over IP TSE MII Bus";
	mdio_bus->read = &hdoip_ether_mdio_read;
	mdio_bus->write = &hdoip_ether_mdio_write;
	mdio_bus->reset = &hdoip_ether_mdio_reset;
	snprintf(mdio_bus->id, MII_BUS_ID_SIZE, "%x", HDOIP_MII_ID);

	mdio_bus->priv = hde;

	ret = mdiobus_register(mdio_bus);
	if (ret) {
		printk(KERN_ERR "Failed to register MDIO bus\n");
		mdiobus_free(mdio_bus);
		return ret;
	}

	hde->mdio_bus = mdio_bus;

	return 0;
}

/*
 * Detect MAC and PHY and perform initialization
 */
static int __init hdoip_ether_probe(struct platform_device *pdev)
{
	struct net_device *dev;
	struct hdoip_ether *hde;
	int ret;

	dev = alloc_etherdev(sizeof(struct hdoip_ether));
	if (!dev) {
		printk(KERN_ERR "%s: Failed to alloc etherdev\n", DRV_NAME);
		return -ENOMEM;
	}

	hde = netdev_priv(dev);

	/* Remap the register/memory areas */
	ret = -ENOMEM;
	hde->tse_mac_base = ioremap_nocache(TSE_MAC_BASE, TSE_MAC_SIZE);
	if (!hde->tse_mac_base)
		goto out_free;
	hde->ethi_base = ioremap_nocache(ACB_ETH_IN_BASE, ACB_ETH_IN_SIZE);
	if (!hde->ethi_base)
		goto out_unmap_tse_mac;
	hde->etho_base = ioremap_nocache(ACB_ETH_OUT_BASE, ACB_ETH_OUT_SIZE);
	if (!hde->etho_base)
		goto out_unmap_ethi;
	hde->ext_pio_base = ioremap_nocache(EXT_RESET_PIO_BASE, 4);
	if (!hde->ext_pio_base)
		goto out_unmap_etho;

	/* Perform a hardware reset */
	hdoip_ether_board_reset(hde);

	/* Probe the MII bus */
	ret = hdoip_mii_probe(dev);
	if (ret)
		goto out_unmap_ext_pio;

	/* Initialize the PHY */
	ret = hdoip_ether_phy_init(dev);
	if (ret)
		goto out_mii_remove;

	/* Register interrupt handler */
	if (request_irq(ACB_ETH_IN_IRQ, hdoip_ether_interrupt, 0, dev->name, dev)) {
		ret = -EBUSY;
		goto out_phy_disconnect;
	}

	dev->irq = ACB_ETH_IN_IRQ;
	dev->base_addr = TSE_MAC_BASE;

	/*
	 * Get MAC address and write a default address, in case an invalid
	 * address was configured previously.
	 */
	hdoip_ether_get_mac_address(dev);
	if (!is_valid_ether_addr(dev->dev_addr)) {
		printk(KERN_ERR
		       "%s: Bootloader did not configure a valid MAC address, using default.\n",
		       DRV_NAME);
		hdoip_ether_set_mac_address(dev, NULL);
	}

	spin_lock_init(&hde->rx_lock);
	spin_lock_init(&hde->tx_lock);

	ether_setup(dev);
	dev->netdev_ops = &hdoip_ether_netdev_ops;
	SET_ETHTOOL_OPS(dev, &hdoip_ether_ethtool_ops);

	ret = register_netdev(dev);
	if (ret)
		goto out_irq_free;

	SET_NETDEV_DEV(dev, &pdev->dev);
	platform_set_drvdata(pdev, dev);

	printk(KERN_INFO "%s: HD over IP Ethernet at 0x%08x, irq=%d\n",
	       dev->name, (unsigned int) dev->base_addr, dev->irq);

	return 0;

out_irq_free:
	free_irq(ACB_ETH_IN_IRQ, dev);
out_phy_disconnect:
	phy_disconnect(hde->phydev);
out_mii_remove:
	mdiobus_unregister(hde->mdio_bus);
	mdiobus_free(hde->mdio_bus);
out_unmap_ext_pio:
	iounmap(hde->ext_pio_base);
out_unmap_etho:
	iounmap(hde->etho_base);
out_unmap_ethi:
	iounmap(hde->ethi_base);
out_unmap_tse_mac:
	iounmap(hde->tse_mac_base);
out_free:
	free_netdev(dev);
	return ret;
}

static int __devexit hdoip_ether_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct hdoip_ether *hde = netdev_priv(dev);

	unregister_netdev(dev);
	free_irq(dev->irq, dev);

	/* Disconnect from PHY and MDIO bus */
	phy_disconnect(hde->phydev);
	mdiobus_unregister(hde->mdio_bus);
	mdiobus_free(hde->mdio_bus);

	iounmap(hde->ext_pio_base);
	iounmap(hde->etho_base);
	iounmap(hde->ethi_base);
	iounmap(hde->tse_mac_base);

	platform_set_drvdata(pdev, NULL);
	free_netdev(dev);

	return 0;
}

static struct platform_driver hdoip_ether_driver = {
	.driver = {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
		.pm	= NULL,
	},
	.remove	= __devexit_p(hdoip_ether_remove),
};

static int __init hdoip_ether_init(void)
{
	printk(KERN_INFO "%s Ethernet Driver, Version %s\n",
	       DRV_NAME, DRV_VERSION);

	register_inetaddr_notifier(&hdoip_ether_notifier);
	return platform_driver_probe(&hdoip_ether_driver, hdoip_ether_probe);
}

static void __exit hdoip_ether_exit(void)
{
	platform_driver_unregister(&hdoip_ether_driver);
	unregister_inetaddr_notifier(&hdoip_ether_notifier);
}

module_init(hdoip_ether_init);
module_exit(hdoip_ether_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HD over IP Ethernet driver");
MODULE_AUTHOR("Tobias Klauser <klto@zhaw.ch>");
MODULE_ALIAS("platfrom:" DRV_NAME);
