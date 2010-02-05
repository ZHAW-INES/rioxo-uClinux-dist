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

#include "hdoip.h"
#include "hdoip_ether.h"

#define HDOIP_ETHER_NAME	"hdoip_ether"
#define HDOIP_ETHER_VERSION	"0.0"

#define NIOS_NC				0x80000000

#define MII_M1111_PHY_EXT_CR		0x14
#define MII_M1111_PHY_LED_CONTROL	0x18

#define MII_M1111_HWCFG_TX_EN		0x01

/*
 * The default MAC address to set in case the bootloader did not set a valid MAC
 * address.
 */
static const u8 mac_addr_default[ETH_ALEN] =
	{ 0x00, 0x15, 0x12, 0x00, 0x00, 0x42 };

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
static inline u32 hdoip_ethi_read(unsigned int reg)
{
	void __iomem *ethi_base = (void __iomem *) ACB_ETH_IN_BASE;
	return ioread32(ethi_base + reg);
}

/*
 * Write to eth_st_in register.
 */
static inline void hdoip_ethi_write(unsigned reg, u32 value)
{
	void __iomem *ethi_base = (void __iomem *) ACB_ETH_IN_BASE;
	iowrite32(value, ethi_base + reg);
}

/*
 * Start the eth_st_in DMA controller state machine.
 */
static inline void hdoip_ethi_start(void)
{
	u32 regval = hdoip_ethi_read(ETHIO_CONFIG);

	regval |= ETHI_CONFIG_RUN;
	hdoip_ethi_write(ETHIO_CONFIG, regval);
}

/*
 * Stop the eth_st_in DMA controller state machine.
 */
static inline void hdoip_ethi_stop(void)
{
	u32 regval = hdoip_ethi_read(ETHIO_CONFIG);

	regval &= ~ETHI_CONFIG_RUN;
	hdoip_ethi_write(ETHIO_CONFIG, regval);
}

/*
 * Update eth_st_in descriptor struct with values from the CPU descriptor
 * registers.
 */
static inline void hdoip_ethi_desc_get_cpu(struct hdoip_descriptor *desc)
{
	desc->start = hdoip_ethi_read(ETHIO_CPU_START_DESC);
	desc->stop = hdoip_ethi_read(ETHIO_CPU_STOP_DESC);
	desc->write = hdoip_ethi_read(ETHIO_CPU_WRITE_DESC);
	desc->read = hdoip_ethi_read(ETHIO_CPU_READ_DESC);
}

/*
 * Update eth_st_in CPU descriptor registers with values from the descriptor
 * struct.
 */
static inline void hdoip_ethi_desc_set_cpu(struct hdoip_descriptor *desc)
{
	hdoip_ethi_write(ETHIO_CPU_START_DESC, desc->start);
	hdoip_ethi_write(ETHIO_CPU_STOP_DESC, desc->stop);
	hdoip_ethi_write(ETHIO_CPU_WRITE_DESC, desc->write);
	hdoip_ethi_write(ETHIO_CPU_READ_DESC, desc->read);
}

/*
 *
 */
static inline void hdoip_ethi_init(struct hdoip_descriptor *rx_desc,
                                   u16 dma_burst_size,
                                   __be32 addr)
{
	u32 regval;

	/* Stop the state machines */
	hdoip_ethi_stop();

	/* Reset IRQ and statistics register */
	regval = hdoip_ethi_read(ETHIO_CONFIG);
	regval &= ~ETHI_CONFIG_IRQ1_RESET;
	regval |= ETHI_CONFIG_STATS_RESET;
	hdoip_ethi_write(ETHIO_CONFIG, regval);

	/* Setup descriptors */
	hdoip_ethi_desc_set_cpu(rx_desc);

	/* Setup DMA... */
	regval = hdoip_ethi_read(ETHI_DMA_CONFIG);
	/* ...burst size */
	regval |= (dma_burst_size << ETHI_CONFIG_DMA_BURST_SIZE_INDEX)
	          & ETHI_CONFIG_DMA_BURST_SIZE_MASK;

	hdoip_ethi_write(ETHI_DMA_CONFIG, regval);
}

/*
 * Read from eth_st_out register.
 */
static inline u32 hdoip_etho_read(unsigned int reg)
{
	void __iomem *etho_base = (void __iomem *) ACB_ETH_OUT_BASE;
	return ioread32(etho_base + reg);
}

/*
 * Write to eth_st_out register.
 */
static inline void hdoip_etho_write(unsigned reg, u32 value)
{
	void __iomem *etho_base = (void __iomem *) ACB_ETH_OUT_BASE;
	iowrite32(value, etho_base + reg);
}

/*
 * Start the eth_st_out DMA controller and priorization state machine.
 */
static inline void hdoip_etho_start(void)
{
	u32 regval = hdoip_etho_read(ETHIO_CONFIG);

	regval |= ETHO_CONFIG_RUN | ETHO_CONFIG_PRIO;
	hdoip_etho_write(ETHIO_CONFIG, regval);
}

/*
 * Stop the eth_st_out DMA controller and priorization state machine.
 */
static inline void hdoip_etho_stop(void)
{
	u32 regval = hdoip_etho_read(ETHIO_CONFIG);

	regval &= ~(ETHO_CONFIG_RUN | ETHO_CONFIG_PRIO);
	hdoip_etho_write(ETHIO_CONFIG, regval);
}

/*
 * Update eth_st_out descriptor struct with values from the CPU descriptor
 * registers.
 */
static inline void hdoip_etho_desc_get_cpu(struct hdoip_descriptor *desc)
{
	desc->start = hdoip_etho_read(ETHIO_CPU_START_DESC);
	desc->stop = hdoip_etho_read(ETHIO_CPU_STOP_DESC);
	desc->write = hdoip_etho_read(ETHIO_CPU_WRITE_DESC);
	desc->read = hdoip_etho_read(ETHIO_CPU_READ_DESC);
}

/*
 * Update eth_st_out CPU descriptor registers with values from the descriptor
 * struct.
 */
static inline void hdoip_etho_desc_set_cpu(struct hdoip_descriptor *desc)
{
	hdoip_etho_write(ETHIO_CPU_START_DESC, desc->start);
	hdoip_etho_write(ETHIO_CPU_STOP_DESC, desc->stop);
	hdoip_etho_write(ETHIO_CPU_WRITE_DESC, desc->write);
	hdoip_etho_write(ETHIO_CPU_READ_DESC, desc->read);
}

/*
 * Initialize the eth_st_out block.
 *
 * @tx_desc:         CPU descriptors to setup for TX ringbuffer
 * @dma_burst_size:  DMA burst size
 * @dma_fifo_thresh: DMA fifo threshold
 */
static inline void hdoip_etho_init(struct hdoip_descriptor *tx_desc,
                                   u16 dma_burst_size, u16 dma_fifo_thresh)
{
	u32 regval;

	/* Stop the state machines */
	hdoip_etho_stop();

	/* Setup descriptors */
	hdoip_etho_desc_set_cpu(tx_desc);

	/* Setup DMA... */
	regval = hdoip_etho_read(ETHO_DMA_CONFIG);
	/* ...burst size */
	regval |= (dma_burst_size << ETHO_CONFIG_DMA_BURST_SIZE_INDEX)
	          & ETHO_CONFIG_DMA_BURST_SIZE_MASK;
	/* ...FIFO threshold */
	regval |= (dma_fifo_thresh << ETHO_CONFIG_DMA_FIFO_THRESH_INDEX)
	          & ETHO_CONFIG_DMA_FIFO_THRESH_MASK;

	hdoip_etho_write(ETHO_DMA_CONFIG, regval);
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
	u32 regval = ioread32(hde->ext_pio_base);

	iowrite32(regval & 0xFE, hde->ext_pio_base);
	udelay(50);
	iowrite32(regval | 0x01, hde->ext_pio_base);
	udelay(50);

	return 0;
}

static void hdoip_ether_get_drvinfo(struct net_device *dev,
                                    struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, HDOIP_ETHER_NAME, sizeof(info->driver));
	strlcpy(info->version, HDOIP_ETHER_VERSION, sizeof(info->version));
	strlcpy(info->bus_info, "avalon", sizeof(info->bus_info));
}

static const struct ethtool_ops hdoip_ether_ethtool_ops = {
	.get_drvinfo	= hdoip_ether_get_drvinfo,
};

static struct net_device_stats *hdoip_ether_get_stats(struct net_device *dev)
{
	struct net_device_stats *stats = &dev->stats;

	if (netif_running(dev)) {
		stats->rx_packets += hdoip_ethi_read(ETHI_CPU_PACKET_COUNT);
		stats->rx_bytes	+= hdoip_ethi_read(ETHI_CPU_DATA_COUNT);
		stats->rx_crc_errors += hdoip_ethi_read(ETHI_CPU_CRC_ERR_COUNT);
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

	printk(KERN_INFO "%s: Setting MAC address to %pM\n", dev->name,
	       dev->dev_addr);
	/* Write the new address to hardware */
	__hdoip_ether_set_mac_address(hde, dev->dev_addr);

	return 0;
}

/*
 * Change the Maximum Transfer Unit (MTU) of the device.
 */
static int hdoip_ether_change_mtu(struct net_device *dev, int mtu)
{
	/* Changing MTU not supported atm */
	return 0;
}

static void hdoip_ether_adjust_link(struct net_device *dev)
{
}

/*
 * Transmit a packet.
 *
 * Called by the network subsystem of the Linux Kernel.
 */
static int hdoip_ether_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct hdoip_ether *priv = netdev_priv(dev);
	struct hdoip_descriptor *tx_desc = &priv->tx_desc;
	unsigned long flags;
	u32 *tx_buf;
	u32 len = skb->len;

	if (unlikely(len > MAX_PKT_SIZE)) {
		dev->stats.tx_dropped++;
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += len;

	spin_lock_irqsave(&priv->tx_lock, flags);

	/* Is there space in the ringbuffer? */
	if (!hdoip_desc_accepts(tx_desc, len)) {
		/* The ringbuffer is full */
		netif_stop_queue(dev);
		spin_unlock_irqrestore(&priv->tx_lock, flags);
		printk(KERN_ERR "TX Ringbuffer is full\n");
		return NETDEV_TX_BUSY;
	}

	tx_buf = (u32 *) tx_desc->write;
	/* First 32 bits of the ringbuffer entry hold the frame size */
	*tx_buf = len;
	/* XXX: Optimize for 32-bit transfers? */
	memcpy(tx_buf + 1, skb->data, len);

	/* Update write pointer (with wrap around) */
	tx_desc->write += len;
	if (tx_desc->write > tx_desc->stop)
		tx_desc->write = tx_desc->start;

	/* Write descriptors */
	hdoip_etho_desc_set_cpu(tx_desc);

	spin_unlock_irqrestore(&priv->tx_lock, flags);

	dev->trans_start = jiffies;

	return NETDEV_TX_OK;
}

/*
 * Receive a packet.
 *
 * Called from interrupt context.
 */
static void hdoip_ether_rx(struct net_device *dev)
{
	struct hdoip_ether *priv = netdev_priv(dev);
	struct sk_buff *skb;
	unsigned int len;
	unsigned long flags;

	spin_lock_irqsave(&priv->rx_lock, flags);

	/* Update the RX descriptor struct */
	hdoip_ethi_desc_get_cpu(&priv->rx_desc);

	/* Get the frame length (first byte) */
	len = ioread32(priv->rx_desc.read);
	pr_debug("RX frame: %d bytes\n", len);

	skb = dev_alloc_skb(len + 2);
	if (!skb) {
		spin_unlock_irqrestore(&priv->rx_lock, flags);
		dev->stats.rx_dropped++;
		printk(KERN_NOTICE "%s: Memory squeeze, dropping packet.\n", dev->name);
		return;
	}

	spin_unlock_irqrestore(&priv->rx_lock, flags);
}

/*
 * MAC interrupt handler.
 */
static irqreturn_t hdoip_ether_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	u32 status;

	pr_debug("Got an interrupt\n");

	status = hdoip_ethi_read(ETHIO_STATUS);
	/* Check the status register for the IRQ */
	if (!(status & ETHI_STATUS_IRQ1_FLAG))
		return IRQ_HANDLED;

	/* Reset IRQ */
	hdoip_ethi_write(ETHIO_STATUS, status | ETHI_CONFIG_IRQ1_RESET);

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
	hdoip_mac_write(hde, TSE_MAC_MTU, HDOIP_DEFAULT_MTU);

	/* Set FIFO thresholds */
	hdoip_mac_write(hde, TSE_MAC_RX_SECTION_EMPTY, TSE_MAC_RX_FIFO_DEPTH - 16);
	hdoip_mac_write(hde, TSE_MAC_RX_SECTION_FULL, 16);	/* XXX: This is 0 in the test software, why? */
	hdoip_mac_write(hde, TSE_MAC_TX_SECTION_EMPTY, TSE_MAC_TX_FIFO_DEPTH - 16);
	hdoip_mac_write(hde, TSE_MAC_TX_SECTION_FULL, 16);
	hdoip_mac_write(hde, TSE_MAC_RX_ALMOST_EMPTY, 8);
	hdoip_mac_write(hde, TSE_MAC_RX_ALMOST_FULL, 8);
	hdoip_mac_write(hde, TSE_MAC_TX_ALMOST_EMPTY, 8);
	hdoip_mac_write(hde, TSE_MAC_TX_ALMOST_FULL, 8);	/* XXX: This is 3 in the test software and altera_tse.c, why? */

	/* Enable CRC insertion and IP payload alignment in TX.
	 *
	 * "Align packet header to 32-bit boundary" must be enabled for the TSE
	 * MAC in SOPC Builder. Otherwise the driver won't work.
	 */
	regval = TSE_TX_CMD_STAT_TX_SHIFT16 & ~TSE_TX_CMD_STAT_OMIT_CRC;
	hdoip_mac_write(hde, TSE_MAC_TX_CMD_STAT, regval);

	/* Enable IP payload alignment in RX.
	 *
	 * "Align packet header to 32-bit boundary" must be enabled for the TSE
	 * MAC in SOPC Builder. Otherwise the driver won't work.
	 */
	hdoip_mac_write(hde, TSE_MAC_RX_CMD_STAT, TSE_RX_CMD_STAT_RX_SHIFT16);

	/* Set command_config register */
	regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	regval = regval & (~TSE_CMD_CRC_FWD)		/* don't forward CRC */
	                & (~TSE_CMD_LOOP_ENA)		/* no loopback */
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

	/* Enable TX on PHY */
	ret = phy_read(hde->phydev, MII_M1111_PHY_EXT_CR);
	if (ret < 0)
		return ret;
	ret |= MII_M1111_HWCFG_TX_EN;
	if (phy_write(hde->phydev, MII_M1111_PHY_EXT_CR, ret & 0xffff))
		return -EINVAL;

	/* XXX: Do we need to SW reset the PHY here? */

	phy_print_status(hde->phydev);	/* XXX: only for debugging */

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

	/* Setup descriptors */
	hde->tx_desc.start = (u32) hde->tx_buf_dma;
	hde->tx_desc.stop = (u32) (hde->tx_buf_dma
	                           + (TX_RING_ENTRIES * MAX_PKTBUF_SIZE)
	                           - MAX_PKT_SIZE);
	hde->tx_desc.write = hde->tx_desc.start;
	hde->tx_desc.read = hde->tx_desc.start;
	hdoip_etho_init(&hde->tx_desc, ETHIO_DMA_BURST_SIZE,
	                ETHO_DMA_FIFO_THRESH);

	hde->rx_desc.start = (u32) hde->rx_buf_dma;
	hde->rx_desc.stop = (u32) (hde->rx_buf_dma
	                           + (RX_RING_ENTRIES * MAX_PKTBUF_SIZE)
	                           - MAX_PKT_SIZE);
	hde->rx_desc.write = (u32) hde->rx_desc.start;
	hde->rx_desc.read = (u32) hde->rx_desc.start;
	hdoip_ethi_init(&hde->rx_desc, ETHIO_DMA_BURST_SIZE, 0);

	/* Enable TX/RX on MAC */
	regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG,
	                regval | TSE_CMD_TX_ENA | TSE_CMD_RX_ENA);

	/* Start the eth_st_in DMA controller */
	hdoip_ethi_start();

	netif_start_queue(dev);

	return 0;
}

static int hdoip_ether_close(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	int ret;
	u32 regval;

	/* Start the eth_st_in DMA controller */
	hdoip_ethi_stop();

	/* Disable TX/RX on MAC */
	regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG,
	                regval & ~(TSE_CMD_TX_ENA | TSE_CMD_RX_ENA));

	/* Disable TX on PHY */
	ret = phy_read(hde->phydev, MII_M1111_PHY_EXT_CR);
	if (ret < 0)
		return ret;
	ret &= ~MII_M1111_HWCFG_TX_EN;
	phy_write(hde->phydev, MII_M1111_PHY_EXT_CR, ret & 0xffff);

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
	struct hdoip_ether *priv;

	pr_debug("called %s\n", __func__);

	if (!dev || !netif_running(dev) || dev_net(dev) != &init_net)
		return NOTIFY_DONE;

	priv = netdev_priv(dev);

	switch (event) {
	case NETDEV_UP:
		printk(KERN_INFO "IP address %pI4 set\n", &ifa->ifa_address);
		hdoip_ethi_write(ETHI_IP_FILTER, ifa->ifa_address);
		break;
	case NETDEV_DOWN:
		printk(KERN_INFO "IP address %pI4 unset\n", &ifa->ifa_address);
		hdoip_ethi_write(ETHI_IP_FILTER, 0);
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
	struct hdoip_ether *priv = netdev_priv(dev);
	char mii_id[MII_BUS_ID_SIZE];
	char phy_id[MII_BUS_ID_SIZE+3];

	snprintf(mii_id, MII_BUS_ID_SIZE, "%x", HDOIP_MII_ID);
	snprintf(phy_id, MII_BUS_ID_SIZE + 3, PHY_ID_FMT, mii_id, HDOIP_PHY_ID);

	phydev = phy_connect(dev, phy_id, hdoip_ether_adjust_link, 0,
	                     PHY_INTERFACE_MODE_RGMII_RXID);
	if (IS_ERR(phydev)) {
		printk(KERN_ERR "%s: Failed to attach to PHY at %s\n",
		       HDOIP_ETHER_NAME, phy_id);
		return PTR_ERR(phydev);
	}

	priv->phydev = phydev;
	phydev->autoneg = AUTONEG_ENABLE;

	/* Reconfigure PHY LEDs */
	phy_write(phydev, MII_M1111_PHY_LED_CONTROL, 0x43);

	return 0;
}

static int __init hdoip_mii_probe(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	struct mii_bus *mdio_bus;
	int ret;
	int phy_addr;

	mdio_bus = mdiobus_alloc();
	if (!mdio_bus)
		return -ENOMEM;

	mdio_bus->name = "HD over IP TSE MII Bus";
	mdio_bus->read = hdoip_ether_mdio_read;
	mdio_bus->write = hdoip_ether_mdio_write;
	mdio_bus->reset = hdoip_ether_mdio_reset;
	snprintf(mdio_bus->id, MII_BUS_ID_SIZE, "%x", HDOIP_MII_ID);

	mdio_bus->priv = hde;

	ret = mdiobus_register(mdio_bus);
	if (ret) {
		printk(KERN_ERR "Failed to register MDIO bus\n");
		mdiobus_free(mdio_bus);
		return ret;
	}

	hde->mdio_bus = mdio_bus;

#ifdef HDOIP_DEBUG
	/* Report all available PHYs */
	for (phy_addr = 0; phy_addr < PHY_MAX_ADDR; phy_addr++) {
		u32 phy_id1, phy_id2;

		phy_id1 = hdoip_ether_mdio_read(mdio_bus, phy_addr, MII_PHYSID1);
		phy_id2 = hdoip_ether_mdio_read(mdio_bus, phy_addr, MII_PHYSID2);
		if (phy_id1 != phy_id2)
			printk(KERN_INFO "PHY with ID=0x%08x at address 0x%x\n",
			       (phy_id1 << 16) | phy_id2, phy_addr);
	}
#endif

	return 0;
}

/*
 * Detect MAC and PHY and perform initialization
 */
static int __init hdoip_ether_probe(struct platform_device *pdev)
{
	struct net_device *dev;
	struct hdoip_ether *priv;
	int ret = -ENOMEM;
	u32 regval;

	hdoip_etho_stop();
	hdoip_ethi_stop();

	/* HD over IP board specific: Perform a PHY hardware reset */
	regval = ioread32(EXT_RESET_PIO_BASE);
	iowrite32(regval & 0xFE, EXT_RESET_PIO_BASE);
	udelay(50);
	iowrite32(regval | 0x01, EXT_RESET_PIO_BASE);
	udelay(50);

	dev = alloc_etherdev(sizeof(struct hdoip_ether));
	if (!dev) {
		dev_err(&pdev->dev, "Failed to alloc etherdev\n");
		return -ENOMEM;
	}

	priv = netdev_priv(dev);

	/* Remap the register/memory areas */
	priv->tse_mac_base = ioremap_nocache(TSE_MAC_BASE, TSE_MAC_SIZE);
	if (!priv->tse_mac_base)
		goto out_free;
	priv->ext_pio_base = ioremap_nocache(EXT_RESET_PIO_BASE, 4);
	if (!priv->tse_mac_base)
		goto out_free;

	/* Probe the MII bus */
	ret = hdoip_mii_probe(dev);
	if (ret)
		goto out_free;

	/* Initialize the PHY */
	ret = hdoip_ether_phy_init(dev);
	if (ret)
		goto out_mii_remove;

	if (request_irq(ACB_ETH_IN_IRQ, hdoip_ether_interrupt, 0, dev->name, dev)) {
		ret = -EBUSY;
		goto out_phy_disconnect;
	}

	dev->irq = ACB_ETH_IN_IRQ;
	dev->base_addr = TSE_MAC_BASE;

	spin_lock_init(&priv->rx_lock);
	spin_lock_init(&priv->tx_lock);

	ether_setup(dev);
	dev->netdev_ops = &hdoip_ether_netdev_ops;
	SET_ETHTOOL_OPS(dev, &hdoip_ether_ethtool_ops);

	ret = register_netdev(dev);
	if (ret)
		goto out_irq_free;

	SET_NETDEV_DEV(dev, &pdev->dev);
	platform_set_drvdata(pdev, dev);

	/*
	 * Get MAC address and write a default address, in case an invalid
	 * address was configured previously.
	 */
	hdoip_ether_get_mac_address(dev);
	if (!is_valid_ether_addr(dev->dev_addr)) {
		printk(KERN_ERR
		       "%s: Bootloader did not configure a valid MAC address, using default.\n",
		       HDOIP_ETHER_NAME);
		hdoip_ether_set_mac_address(dev, NULL);
	}

	/* Display a banner */
	printk(KERN_INFO "%s: HD over IP Ethernet at 0x%08x, irq=%d\n",
	       dev->name, (unsigned int) dev->base_addr, dev->irq);

	return 0;

out_irq_free:
	free_irq(ACB_ETH_IN_IRQ, dev);
out_phy_disconnect:
	phy_disconnect(priv->phydev);
out_mii_remove:
	mdiobus_unregister(priv->mdio_bus);
	mdiobus_free(priv->mdio_bus);
out_free:
	free_netdev(dev);
	return ret;
}

static int __devexit hdoip_ether_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct hdoip_ether *priv = netdev_priv(dev);

	free_irq(dev->irq, dev);

	/* Disconnect from PHY and MDIO bus */
	phy_disconnect(priv->phydev);
	mdiobus_unregister(priv->mdio_bus);
	mdiobus_free(priv->mdio_bus);

	platform_set_drvdata(pdev, NULL);
	unregister_netdev(dev);
	free_netdev(dev);

	return 0;
}

static struct platform_driver hdoip_ether_driver = {
	.driver = {
		.name	= HDOIP_ETHER_NAME,
		.owner	= THIS_MODULE,
		.pm	= NULL,
	},
	.remove	= __devexit_p(hdoip_ether_remove),
};

static int __init hdoip_ether_init(void)
{
	printk(KERN_INFO "%s Ethernet Driver, Version %s\n",
	       HDOIP_ETHER_NAME, HDOIP_ETHER_VERSION);

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
MODULE_AUTHOR("Tobias Klauser");
MODULE_ALIAS("platfrom:" HDOIP_ETHER_NAME);
