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

#include "eti_drv.h"
#include "eto_drv.h"
#include "esc_drv.h"
#include "esc_reg.h"

/* MTU */
#define HDOIP_DEFAULT_MTU	    	0xffff
#define HDOIP_MIN_MTU			    64	/* 60? */

/* Default values for MAC registers */
#define HDOIP_MII_ID			    0
#define HDOIP_PHY_ID			    0x12  /*  HW revision 1.4: 0x14      HW revision 1.3: 0x12*/

/* Maximum size of a ringbuffer entry. Also defines the maximum packet size. */
#define MAX_PKTBUF_SIZE				2048
/* Maximum packet size this driver can handle */
#define MAX_PKT_SIZE				(MAX_PKTBUF_SIZE - 4)

/* Size for the TX/RX ringbuffers */
#define TX_RING_ENTRIES				16
#define RX_RING_ENTRIES				16

/* Length of the frame size preceeding transmitted/received frames */
#define FRAME_SIZE_LENGTH			sizeof(u32)
/* Length of the timestamp preceeding received frames */
#define RX_TIMESTAMP_LENGTH			sizeof(u32)



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

	/* Ethernet in/out pointer */
    t_eto   eto;
    t_eti   eti;	
    t_esc   esc;

	/* TX/RX ringbuffers */
	void __iomem *tx_buf;
	void __iomem *rx_buf;
	dma_addr_t tx_buf_dma;
	dma_addr_t rx_buf_dma;
};

#endif /* _HDOIP_ETHER_H_ */
