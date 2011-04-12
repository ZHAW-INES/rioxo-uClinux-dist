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

/* #define DEBUG 1 */
#undef DEBUG

#include <linux/kernel.h>
#include <linux/module.h>
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

#include "hdoip_ether_main.h"
#include "hdoip_ether_tse.h"
#include "config.h"
#include "stdrbf.h"

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
 * Read from a TSE MAC register.
 */
static inline u32 hdoip_mac_read(struct hdoip_ether *hde, unsigned int reg)
{
	return ioread32(hde->tse_mac_base + reg);
}

/*
 * Write to a TSE MAC register.
 */
static inline void hdoip_mac_write(struct hdoip_ether *hde, unsigned int reg,
                                   u32 value)
{
	iowrite32(value, hde->tse_mac_base + reg);
}

/*
 * Check whether writing len bytes to CPU ringbuffer would be a valid operation.
 */
static inline bool hdoip_etho_accepts_cpu_write(struct hdoip_ether *hde, u32 len)
{
	t_rbf_dsc desc;
	u32 new_write;

    eto_get_cpu_desc(hde->eto.ptr, &desc);
	new_write = desc.write + len;

	if (desc.write < desc.read && new_write < desc.read)
		return true;
	if (desc.write >= desc.read && new_write >= desc.read
	    && !(desc.read == desc.start && new_write > desc.stop))
		return true;

	return false;
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
	/* struct hdoip_ether *hde = mii->priv; */

    /* TO DO: insert mdio reset */

	return 0;
}

static int hdoip_ether_ethtool_get_settings(struct net_device *dev,
                                            struct ethtool_cmd *cmd)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	struct phy_device *phy = hde->phydev;

	if (!phy)
		return -EINVAL;

	return phy_ethtool_gset(phy, cmd);
}

static int hdoip_ether_ethtool_set_settings(struct net_device *dev,
                                            struct ethtool_cmd *cmd)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	struct phy_device *phy = hde->phydev;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!phy)
		return -EINVAL;

	return phy_ethtool_sset(phy, cmd);
}

static void hdoip_ether_ethtool_get_drvinfo(struct net_device *dev,
                                            struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, DRV_NAME, sizeof(info->driver));
	strlcpy(info->version, DRV_VERSION, sizeof(info->version));
	strlcpy(info->bus_info, "avalon", sizeof(info->bus_info));
}

static const struct ethtool_ops hdoip_ether_ethtool_ops = {
	.get_settings	= hdoip_ether_ethtool_get_settings,
	.set_settings	= hdoip_ether_ethtool_set_settings,
	.get_link	= ethtool_op_get_link,
	.get_drvinfo	= hdoip_ether_ethtool_get_drvinfo,
};

static struct net_device_stats *hdoip_ether_get_stats(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	struct net_device_stats *stats = &dev->stats;

	if (netif_running(dev)) {
		/* Read statistic registers of ethi */
#if 0
		stats->rx_packets = hdoip_ethi_read(hde, ETHI_CPU_PACKET_COUNT);
		stats->rx_bytes	= hdoip_ethi_read(hde, ETHI_CPU_DATA_COUNT);
#endif
        stats->rx_crc_errors = eti_get_inv_packet_cnt(hde->eti.ptr);

		/* Read TSE MAC statistic registers */
		stats->multicast = hdoip_mac_read(hde, TSE_MAC_IF_IN_MULTICAST_PKTS);
	}

	return stats;
}

/*
 * Set multicast MAC addresses in MAC hash lookup table
 */
static void hdoip_ether_mac_set_mcast_hash(struct hdoip_ether *hde,
                                           struct dev_mc_list *addrs)
{
	struct dev_mc_list *cur;

	for (cur = addrs; cur; cur = cur->next) {
		unsigned int hash = 0;
		int i;
		u8 octet, xor_bit;

		if (!is_multicast_ether_addr(cur->dmi_addr))
			continue;

		/* Triple Speed Ethernet User Guide p. 4-6 */
		for (i = 5; i >= 0; i--) {
			xor_bit = 0;
			octet = cur->dmi_addr[i];
			xor_bit = ((octet >> 0) ^
			           (octet >> 1) ^
				   (octet >> 2) ^
				   (octet >> 3) ^
				   (octet >> 4) ^
				   (octet >> 5) ^
				   (octet >> 6) ^
				   (octet >> 7)) & 0x01;
			hash = (hash << 1) | xor_bit;
		}

		hdoip_mac_write(hde, TSE_MAC_MCAST_HASH_TABLE + ((hash % 0x3f) * 4), 1);
	}
}

static void hdoip_ether_set_multicast_list(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	u32 value;
	unsigned int off;

	/* Promiscuous mode */
	value = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	if (dev->flags & IFF_PROMISC)
		value |= TSE_CMD_PROMIS_EN;
	else
		value &= ~TSE_CMD_PROMIS_EN;
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG, value);

	/* Multicast */
	if (dev->flags & IFF_ALLMULTI) {
		/* Accept all multicast */
		for (off = 0; off < TSE_MAC_MCAST_HASH_TABLE_SIZE; off += 4)
			hdoip_mac_write(hde, TSE_MAC_MCAST_HASH_TABLE + off, 1);
	} else {
		/* Clear all entries */
		for (off = 0; off < TSE_MAC_MCAST_HASH_TABLE_SIZE; off += 4)
			hdoip_mac_write(hde, TSE_MAC_MCAST_HASH_TABLE + off, 0);

		if (dev->mc_count > 0)
			hdoip_ether_mac_set_mcast_hash(hde, dev->mc_list);
	}
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
	struct hdoip_ether *hde = netdev_priv(dev);

	if (netif_running(dev))
		return -EBUSY;

	if (mtu < HDOIP_MIN_MTU || mtu > MAX_PKT_SIZE)
		return -EINVAL;

	dev->mtu = mtu;
	hdoip_mac_write(hde, TSE_MAC_MTU, mtu);

	return 0;
}

/*
 * Propagate changes of link, duplex and speed. Called by the PHY subystem.
 */
static void hdoip_ether_adjust_link(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	struct phy_device *phydev = hde->phydev;
	u32 regval;
	bool new_state = false;

    esc_handler(&(hde->esc));

	if (phydev->link != hde->old_link) {
		new_state = true;
		hde->old_link = phydev->link;

        if(phydev->link) { 
            eti_drv_start(&(hde->eti));
            eto_drv_start(&(hde->eto));
        } else {
            eti_drv_stop(&(hde->eti));
            eto_drv_stop(&(hde->eto));
        }
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
			hde->old_speed = phydev->speed;
		}

		/* Only write if something has changed */
		if (new_state)
			hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG, regval);

		netif_carrier_on(dev);
	}

	if (new_state)
		phy_print_status(phydev);
}

/* ring buffer frame RAM write, optimized for 32bit write */
static inline void hdoip_ether_write_frame(volatile u32 *dest, volatile u32 *src,
                                           const size_t n)
{
	size_t count = n >> 2;

	while (count) {
		iowrite32(cpu_to_be32(*src++), dest++);
		count--;
	}
}

/* ring buffer frame RAM read, optimized for 32bit read */
static inline void hdoip_ether_read_frame(volatile u32 *dest, volatile u32 *src,
                                          const size_t n)
{
	size_t count = n >> 2;

	while (count) {
		*dest++ = be32_to_cpu(ioread32(src++));
		count--;
	}
}

/*
 * Transmit a packet.
 *
 * Called by the network subsystem of the Linux Kernel.
 */
static int hdoip_ether_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	t_rbf_dsc tx_desc;
	unsigned long flags;
	u32 *tx_buf;
	u32 len = skb->len;
	u32 newlen;

	/* check oversized frames */
	if (unlikely(len > MAX_PKT_SIZE)) {
		dev->stats.tx_dropped++;
		printk(KERN_ERR "%s: Packet too large\n", dev->name);
		goto out_free;
	}

	/* pad short packets */
	if (len < ETH_ZLEN + 4) {
		if (skb_padto(skb, ETH_ZLEN + 4)) {
			printk(KERN_ERR "%s: Packet padding failed\n", dev->name);
			goto out_free;
		}
		len = ETH_ZLEN + 4;
	}

	/* align frame data to 32bit boundaries */
	if (((unsigned long) skb->data) & 0x2) {
		skb_push(skb, 2);
		len += 2;
	}

	/* pad packet to multiple of 32 bits */
	newlen = len + 3;
	newlen &= ~3UL;
	if (newlen != len) {
		if (skb_padto(skb, newlen)) {
			dev->stats.tx_dropped++;
			printk(KERN_ERR "%s: Packet padding failed\n", dev->name);
			goto out_free;
		}
		len = newlen;
	}

	spin_lock_irqsave(&hde->tx_lock, flags);

	/* Is there space in the ringbuffer? */
	if (!hdoip_etho_accepts_cpu_write(hde, len + 4)) {
		t_rbf_dsc desc;

        eto_get_cpu_desc(hde->eto.ptr, &desc);

		spin_unlock_irqrestore(&hde->tx_lock, flags);
		printk(KERN_ERR "%s: TX Ringbuffer is full (start: %08x stop: %08x read: %08x write: %08x)\n",
			dev->name, desc.start, desc.stop, desc.read, desc.write);
		/* TODO: Stop netif queue */
		return NETDEV_TX_BUSY;
	}

	/* Get current descriptor values */
    eto_get_cpu_desc(hde->eto.ptr, &tx_desc);

	tx_buf = (u32 *) ioremap_nocache(tx_desc.write, MAX_PKT_SIZE);
	if (!tx_buf) {
		dev->stats.tx_dropped++;
		printk(KERN_ERR "%s: Failed to remap TX ringbuffer memory.\n", dev->name);
		goto out_unlock;
	}

	/* First 32 bits of the ringbuffer entry hold the frame size */
	iowrite32(len, tx_buf);
	hdoip_ether_write_frame(tx_buf + 1, (u32 *) skb->data, len);
	/* Make sure everything gets written to memory */
	flush_dcache_range((unsigned long) tx_buf, (unsigned long) (((u8 *) tx_buf) + len));

	/* Update write pointer (with wrap around) */
	tx_desc.write += len + FRAME_SIZE_LENGTH;
	if (tx_desc.write > tx_desc.stop)
		tx_desc.write = tx_desc.start;

	/* Set write descriptor */
    eto_set_cpu_write_desc(hde->eto.ptr, tx_desc.write);

	dev->trans_start = jiffies;
	dev->stats.tx_packets++;
	dev->stats.tx_bytes += len;

out_unlock:
	spin_unlock_irqrestore(&hde->tx_lock, flags);
out_free:
	dev_kfree_skb(skb);

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
	t_rbf_dsc rx_desc;
	struct sk_buff *skb;
	unsigned int len;
	unsigned long flags;
	u32 *rx_buf;

	spin_lock_irqsave(&hde->rx_lock, flags);

	/* Get the current descriptor values */
    eti_clr_irq1(hde->eti.ptr);
    eti_get_cpu_desc(hde->eti.ptr, &rx_desc);
   
again:
    
	if (rx_desc.read == rx_desc.write) {
		/* TODO: Stop netif queue */
		spin_unlock_irqrestore(&hde->rx_lock, flags);
		printk(KERN_NOTICE "%s: RX ring buffer empty.\n", dev->name);
		return;
	}

	rx_buf = (u32 *) ioremap_nocache(rx_desc.read, MAX_PKT_SIZE);
	if (!rx_buf) {
		spin_unlock_irqrestore(&hde->rx_lock, flags);
		printk(KERN_ERR "%s: Failed to remap RX ringbuffer memory.\n", dev->name);
		return;
	}

	/* Get the frame length (first byte) */
	len = ioread32(rx_buf);

	len -= RX_TIMESTAMP_LENGTH;
	rx_buf = rx_buf + (RX_TIMESTAMP_LENGTH / sizeof(u32));

#if 0
	if (len > dev->mtu)
		pr_warning("%s: Frame size larger than MTU: %u\n", dev->name, len);

	if (len < ETH_ZLEN + 4)
		pr_warning("%s: Received undersized frame of %u bytes\n", dev->name, len);
#endif

	skb = netdev_alloc_skb(dev, len + NET_IP_ALIGN);
	if (!skb) {
		dev->stats.rx_dropped++;
		printk(KERN_NOTICE "%s: Memory squeeze, dropping packet.\n", dev->name);
		goto out;
	}

	skb_reserve(skb, NET_IP_ALIGN);

	/* Read the frame data */
	skb_put(skb, len);
	hdoip_ether_read_frame((u32 *) skb->data, rx_buf + 1, len);

	/* Hardware already verified the checksum */
	skb->ip_summed = CHECKSUM_NONE;
	skb->protocol = eth_type_trans(skb, dev);

	netif_rx(skb);

	dev->last_rx = jiffies;
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += len;

out:
	/* Update the descriptors (with wrap around) */
	rx_desc.read += len + FRAME_SIZE_LENGTH + RX_TIMESTAMP_LENGTH;
	if (rx_desc.read > rx_desc.stop)
		rx_desc.read = rx_desc.start;

	/* Set read descriptor */
    eti_set_cpu_read_desc(hde->eti.ptr, rx_desc.read); 
    /* Read write descripto agan (XXX: this might be a bit of a hack) */
    eti_get_cpu_desc(hde->eti.ptr, &rx_desc);

	if (rx_desc.read != rx_desc.write) {
        eti_clr_irq1(hde->eti.ptr);
		goto again;
    }

	spin_unlock_irqrestore(&hde->rx_lock, flags);
}

/*
 * MAC interrupt handler.
 */
static irqreturn_t hdoip_ether_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct hdoip_ether *hde = netdev_priv(dev);

	/* Check the status register for the IRQ */
	if ((eti_get_irq1(hde->eti.ptr)) == 0)
		return IRQ_HANDLED;

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
	hdoip_mac_write(hde, TSE_MAC_MTU, dev->mtu);

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
	regval = regval & (~TSE_CMD_CRC_FWD)		    /* don't forward CRC */
	                & (~TSE_CMD_PROMIS_EN)		    /* promiscous disable */
	                & (~TSE_CMD_LOOP_EN)		    /* no loopback */
			        & (~TSE_CMD_MHASH_SEL)          /* use 48bit address for multicast hash */
	                & (~TSE_CMD_TX_ADDR_INS)	    /* don't set MAC address on TX */
	                & (~TSE_CMD_RX_ERR_DISC)	    /* don't discard erroneous frames */
	                & (~TSE_CMD_NO_LENGTH_CHECK);	/* check payload length against length/type field on RX */
	regval = regval | TSE_CMD_PAD_EN;               /* enable padding removal */
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

	/* Stop the state machines */
	eto_drv_stop(&(hde->eto));
	eti_drv_stop(&(hde->eti));

	/* Setup CPU descriptors */
    eto_drv_set_cpu_buf(&(hde->eto), (void*) hde->tx_buf_dma, 
                          (TX_RING_ENTRIES * MAX_PKTBUF_SIZE) - MAX_PKT_SIZE);

    eti_drv_set_cpu_buf(&(hde->eti), (void*) hde->rx_buf_dma, 
                          (RX_RING_ENTRIES * MAX_PKTBUF_SIZE) - MAX_PKT_SIZE);

#if 0
	/* Setup video/audio descriptors */
	hdoip_descriptor_init(&desc, 0, 0x4000);
	hdoip_etho_desc_set_vid(hde, &desc);

	hdoip_descriptor_init(&desc, 0, 0x4000);
	hdoip_ethi_desc_set_vid(hde, &desc);

	hdoip_descriptor_init(&desc, 0, 0x4000);
	hdoip_etho_desc_set_aud(hde, &desc);

	hdoip_descriptor_init(&desc, 0, 0x4000);
	hdoip_ethi_desc_set_aud(hde, &desc);
#endif

	
	/* Enable TX/RX on MAC */
	regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG,
	                regval | TSE_CMD_TX_EN | TSE_CMD_RX_EN);

	phy_start(hde->phydev);
	
    netif_start_queue(dev);

	return 0;
}

static int hdoip_ether_close(struct net_device *dev)
{
	struct hdoip_ether *hde = netdev_priv(dev);
	u32 regval;

	netif_stop_queue(dev);

	/* Stop DMA controller state machines */
	eti_drv_stop(&(hde->eti));
	eto_drv_stop(&(hde->eto));

	phy_stop(hde->phydev);

	/* Disable TX/RX on MAC */
	regval = hdoip_mac_read(hde, TSE_MAC_COMMAND_CONFIG);
	hdoip_mac_write(hde, TSE_MAC_COMMAND_CONFIG,
	                regval & ~(TSE_CMD_TX_EN | TSE_CMD_RX_EN));

	/* Free ringbuffer memory */
	dma_free_coherent(&dev->dev, TX_RING_ENTRIES * MAX_PKTBUF_SIZE,
	                  hde->tx_buf, hde->tx_buf_dma);
	dma_free_coherent(&dev->dev, RX_RING_ENTRIES * MAX_PKTBUF_SIZE,
	                  hde->rx_buf, hde->rx_buf_dma);

	return 0;
}

static const struct net_device_ops hdoip_ether_netdev_ops = {
	.ndo_open               = hdoip_ether_open,
	.ndo_stop               = hdoip_ether_close,
	.ndo_start_xmit         = hdoip_ether_start_xmit,
	.ndo_set_multicast_list = hdoip_ether_set_multicast_list,
	.ndo_set_mac_address    = hdoip_ether_set_mac_address,
	.ndo_validate_addr      = eth_validate_addr,
	.ndo_change_mtu         = hdoip_ether_change_mtu,
	.ndo_get_stats          = hdoip_ether_get_stats,
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

/*	switch (event) {
        case NETDEV_UP:
            eti_drv_start(&(hde->eti));
            eto_drv_start(&(hde->eto));
    		break;
    	case NETDEV_DOWN:
            eti_drv_stop(&(hde->eti));
            eto_drv_stop(&(hde->eto));
    		break;
	}
*/
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
	                     PHY_INTERFACE_MODE_RGMII);
	if (IS_ERR(phydev)) {
		printk(KERN_ERR "%s: Failed to attach to PHY at %s\n",
		       DRV_NAME, phy_id);
		return PTR_ERR(phydev);
	}

	phydev->autoneg = AUTONEG_ENABLE;
	/* 10, 100 and Gbit supported */
	phydev->supported = SUPPORTED_1000baseT_Full | SUPPORTED_1000baseT_Half | SUPPORTED_100baseT_Full | SUPPORTED_100baseT_Half | SUPPORTED_10baseT_Full | SUPPORTED_10baseT_Half |SUPPORTED_Autoneg;
	phydev->advertising = ADVERTISED_1000baseT_Full | ADVERTISED_1000baseT_Half | ADVERTISED_100baseT_Full | ADVERTISED_100baseT_Half | ADVERTISED_10baseT_Full | ADVERTISED_10baseT_Half | ADVERTISED_Autoneg;

	hde->phydev = phydev;

	return 0;
}

static int hdoip_mdio_irqs[] = {
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
	PHY_POLL,
};

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
	mdio_bus->irq = hdoip_mdio_irqs;

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
    void *p_eti;
    void *p_eto;
    void *p_esc;
	int ret;

	dev = alloc_etherdev(sizeof(struct hdoip_ether));
	if (!dev) {
		printk(KERN_ERR "%s: Failed to alloc etherdev\n", DRV_NAME);
		return -ENOMEM;
	}

	hde = netdev_priv(dev);

	/* Remap the register/memory areas */
	ret = -ENOMEM;
	hde->tse_mac_base = ioremap_nocache(BASE_TSE, TSE_MAC_SIZE);
	if (!hde->tse_mac_base)
		goto out_free;
	p_eti = ioremap_nocache(BASE_ETI, ETI_ADDR_SPACE);
	if (!p_eti)
		goto out_unmap_tse_mac;
	p_eto = ioremap_nocache(BASE_ETO, ETO_ADDR_SPACE);
	if (!p_eto)
		goto out_unmap_ethi;
	p_esc = ioremap_nocache(BASE_ESC, ESC_ADDR_SPACE);
	if (!p_esc)
		goto out_unmap_esc;

    /* Initialize ethernet in/out blocks */
	eto_drv_init(&(hde->eto), p_eto);
	eti_drv_init(&(hde->eti), p_eti);

    /* Initialize ethernet speed change */
    esc_init(&(hde->esc), p_esc, hde->tse_mac_base);

	/* Probe the MII bus */
	ret = hdoip_mii_probe(dev);
	if (ret)
		goto out_unmap_etho;

	/* Initialize the PHY */
	ret = hdoip_ether_phy_init(dev);
	if (ret)
		goto out_mii_remove;

	/* Register interrupt handler */
	if (request_irq(BASE_ETI_IRQ, hdoip_ether_interrupt,
	                IRQF_TRIGGER_FALLING|IRQF_DISABLED|IRQF_SAMPLE_RANDOM|IRQF_NOBALANCING,
	                dev->name, dev)) {
		ret = -EBUSY;
		goto out_phy_disconnect;
	}

	dev->irq = BASE_ETI_IRQ;
	dev->base_addr = BASE_TSE;

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

	dev->mtu = HDOIP_DEFAULT_MTU;

	ret = register_netdev(dev);
	if (ret)
		goto out_irq_free;

	SET_NETDEV_DEV(dev, &pdev->dev);
	platform_set_drvdata(pdev, dev);

	register_inetaddr_notifier(&hdoip_ether_notifier);

	printk(KERN_INFO "%s: HD over IP Ethernet at 0x%08x (0x%08x/0x%08x), irq=%d\n",
	       dev->name, (unsigned int) dev->base_addr, (unsigned int) hde->eti.ptr,
	       (unsigned int) hde->eto.ptr, dev->irq);

	return 0;

out_irq_free:
	free_irq(BASE_ETI_IRQ, dev);
out_phy_disconnect:
	phy_disconnect(hde->phydev);
out_mii_remove:
	mdiobus_unregister(hde->mdio_bus);
	mdiobus_free(hde->mdio_bus);
out_unmap_etho:
	iounmap(hde->eto.ptr);
out_unmap_ethi:
	iounmap(hde->eti.ptr);
out_unmap_tse_mac:
	iounmap(hde->tse_mac_base);
out_unmap_esc:
    iounmap(hde->esc.ptr);
out_free:
	free_netdev(dev);
	return ret;
}

static int __devexit hdoip_ether_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct hdoip_ether *hde = netdev_priv(dev);

	unregister_inetaddr_notifier(&hdoip_ether_notifier);

	unregister_netdev(dev);
	free_irq(dev->irq, dev);

	/* Disconnect from PHY and MDIO bus */
	phy_disconnect(hde->phydev);
	mdiobus_unregister(hde->mdio_bus);
	mdiobus_free(hde->mdio_bus);

	iounmap(hde->eto.ptr);
	iounmap(hde->eti.ptr);
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
	return platform_driver_probe(&hdoip_ether_driver, hdoip_ether_probe);
}

static void __exit hdoip_ether_exit(void)
{
	platform_driver_unregister(&hdoip_ether_driver);
}

module_init(hdoip_ether_init);
module_exit(hdoip_ether_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HD over IP Ethernet driver");
MODULE_AUTHOR("Tobias Klauser <klto@zhaw.ch>");
MODULE_ALIAS("platfrom:" DRV_NAME);
