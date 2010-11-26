/*****************************************************************
**  _____       ______  _____                                    *
** |_   _|     |  ____|/ ____|                                   *
**   | |  _ __ | |__  | (___    Institute of Embedded Systems    *
**   | | | '_ \|  __|  \___ \   Zuercher Hochschule Winterthur   *
**  _| |_| | | | |____ ____) |  (University of Applied Sciences) *
** |_____|_| |_|______|_____/   8401 Winterthur, Switzerland     *
******************************************************************
**
** Project     : SInet
** Description : Constants for SInet-FPGA
**
** $LastChangedDate$
** $Rev$
** $Author$
*****************************************************************
**
** Change History
** Date     | Name     | Modification
************|**********|*****************************************
** 02.06.08 | ffar     | file created
************|**********|*****************************************/

#ifndef _SINET_FPGA_H_
#define _SINET_FPGA_H_

#include <asm/nios2.h>

/* Register addresses / Avalon Bus settings */
#define AVALON_WIDTH		2	/* in bytes */
#define C_AV_ADR_BASE_WIDTH	15	/* byte aligned */
#define C_REGIONS_BASE		na_nios_16bit_interface_wbd_1  /* 0x04800000 */

/* Memory Regions */
#define C_REGION_GLOBAL		0
#define C_REGION_SCU_CORE	1
#define C_REGION_HDLC		2

/* Global Region Registers */
#define GLOBAL_ADDR_BASE	((C_REGION_GLOBAL << C_AV_ADR_BASE_WIDTH) + C_REGIONS_BASE)
/* no cache version of pointer (TODO, should be done with remap but nios kernel does not provide correct remap...) */
#define GLOBAL_ADDR_BASE_NC	(GLOBAL_ADDR_BASE | 0x80000000)
/* size of the register region */
#define GLOBAL_ADDR_SIZE	0x14

/* SCU core registers */
#define SCU_CORE_ADDR_BASE	((C_REGION_SCU_CORE << C_AV_ADR_BASE_WIDTH) + C_REGIONS_BASE)

/* HDLC registers */
#define HDLC_ADDR_BASE		((C_REGION_HDLC << C_AV_ADR_BASE_WIDTH) + C_REGIONS_BASE)
/* no cache version of pointer (TODO, should be done with remap but nios kernel does not provide correct remap...) */
#define HDLC_ADDR_BASE_NC	(HDLC_ADDR_BASE | 0x80000000)

#define HDLC_RX_IRQ		na_nios_interrupt_quirk_hdlc_rx_irq
#define HDLC_TX_IRQ		na_nios_interrupt_quirk_hdlc_tx_irq

/* Version register read functions */
extern u16 scu_get_svn_version(void);
extern u32 scu_get_fpga_synthesis_time(void);

#endif /* _SINET_FPGA_H_ */
