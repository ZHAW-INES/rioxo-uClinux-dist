/*####ECOSGPLCOPYRIGHTBEGIN####
** -------------------------------------------
** This file is part of eCos, the Embedded Configurable Operating System.
**
** eCos is free software; you can redistribute it and/or modify it under
** the terms of the GNU General Public License as published by the Free
** Software Foundation; either version 2 or (at your option) any later version.
**
** eCos is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
** for more details.
**
** You should have received a copy of the GNU General Public License along
** with eCos; if not, write to the Free Software Foundation, Inc.,
** 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
**
** As a special exception, if other files instantiate templates or use macros
** or inline functions from this file, or you compile this file and link it
** with other works to produce a work based on this file, this file does not
** by itself cause the resulting work to be covered by the GNU General Public
** License. However the source code for this file must still be made available
** in accordance with section (3) of the GNU General Public License.
**
** This exception does not invalidate any other reasons why a work based on
** this file might be covered by the GNU General Public License.
**
** -------------------------------------------
**####ECOSGPLCOPYRIGHTEND####
*/

/*
 *  Modified for used in uClinux
 *  15.01.2013, buan
 */

#ifndef __ALT_AVALON_RTP_TX_REGS_H__
#define __ALT_AVALON_RTP_TX_REGS_H__

#include "stdhal.h"

/* RTP Transmitter enable register */
#define ALTERA_AVALON_RTP_TX_ENABLE_REG                             (0*4)
#define IORD_ALTERA_AVALON_RTP_TX_ENABLE(base)                      HOI_RD32((base), ALTERA_AVALON_RTP_TX_ENABLE_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_ENABLE(base, data)                HOI_WR32((base), ALTERA_AVALON_RTP_TX_ENABLE_REG, (data))

#define ALTERA_AVALON_RTP_TX_ENABLE_ENABLE_MSK                      (0x00000001)
#define ALTERA_AVALON_RTP_TX_ENABLE_ENABLE_OFST                     (0)


/* FEC control register */
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_REG                        (1*4)
#define IORD_ALTERA_AVALON_RTP_TX_FEC_CONTROL(base)                 HOI_RD32((base), ALTERA_AVALON_RTP_TX_FEC_CONTROL_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_FEC_CONTROL(base, data)           HOI_WR32((base), ALTERA_AVALON_RTP_TX_FEC_CONTROL_REG, (data))

#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_L_MSK                      (0x0000001F)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_L_OFST                     (0)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_D_MSK                      (0x00001F00)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_D_OFST                     (8)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_COL_ONLY_MSK               (0x00010000)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_COL_ONLY_OFST              (16)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_ROW_FEC_NO_DELAY_MSK       (0x00100000)         /* What is this? */
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_ROW_FEC_NO_DELAY_OFST      (20)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_COL_FEC_NO_DELAY_MSK       (0x00200000)         /* What is this? */
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_COL_FEC_NO_DELAY_OFST      (21)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_ANNEX_A_EN_MSK             (0x01000000)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_ANNEX_A_EN_OFST            (24)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_INTERLEAVE_OFF_MSK         (0x02000000)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_INTERLEAVE_OFF_OFST        (25)
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_UPDATING_MSK               (0x80000000)         /* READ ONLY */
#define ALTERA_AVALON_RTP_TX_FEC_CONTROL_UPDATING_OFST              (31)


/* Good packets between bursts */
#define ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS_REG                (2*4)
#define IORD_ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS(base)         HOI_RD32((base), ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS(base, data)   HOI_WR32((base), ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS_REG, (data))

#define ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS_COUNT_MSK          (0x0000FFFF)
#define ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS_COUNT_OFST         (0)


/* Burst error control */
#define ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_REG                  (3*4)
#define IORD_ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL(base)           HOI_RD32((base), ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL(base, data)     HOI_WR32((base), ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_REG, (data))

#define ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_NUM_PACKETS_MSK      (0x0000FFFF)
#define ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_NUM_PACKETS_OFST     (0)
#define ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_SINGLE_ERROR_MSK     (0x80000000)
#define ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_SINGLE_ERROR_OFST    (31)


/* Burst error channel select */
#define ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_REG                  (4*4)
#define IORD_ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL(base)           HOI_RD32((base), ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL(base, data)     HOI_WR32((base), ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_REG, (data))

#define ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_SEL_MSK              (0x0000FFFF)
#define ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_SEL_OFST             (0)
#define ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_IMPLEMENTED_MSK      (0x80000000)   /* READ ONLY */
#define ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_IMPLEMENTED_OFST     (31)



/* FEC mode and TS packing channel select */
#define ALTERA_AVALON_RTP_TX_FEC_CHANNEL_REG                        (5*4)
#define IORD_ALTERA_AVALON_RTP_TX_FEC_CHANNEL(base)                 HOI_RD32((base), ALTERA_AVALON_RTP_TX_FEC_CHANNEL_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_FEC_CHANNEL(base, data)           HOI_WR32((base), ALTERA_AVALON_RTP_TX_FEC_CHANNEL_REG, (data))

#define ALTERA_AVALON_RTP_TX_FEC_CHANNEL_SEL_MSK                    (0x0000FFFF)
#define ALTERA_AVALON_RTP_TX_FEC_CHANNEL_SEL_OFST                   (0)



/* Debug register */
#define ALTERA_AVALON_RTP_TX_DEBUG_REG                              (6*4)
#define IORD_ALTERA_AVALON_RTP_TX_DEBUG(base)                       HOI_RD32((base), ALTERA_AVALON_RTP_TX_DEBUG_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_DEBUG(base, data)                 HOI_WR32((base), ALTERA_AVALON_RTP_TX_DEBUG_REG, (data))

#define ALTERA_AVALON_RTP_TX_DEBUG_MSK                              (0x0000000F)
#define ALTERA_AVALON_RTP_TX_DEBUG_OFST                             (0)



/* FEC freelist minimum level status register */
#define ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS_REG                (7*4)
#define IORD_ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS(base)         HOI_RD32((base), ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS(base, data)   (1=0)

#define ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS_MINIMUM_MSK        (0x0000FFFF)   /* READ ONLY */
#define ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS_MINIMUM_OFST       (0)
#define ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS_TOTAL_MSK          (0xFFFF0000)   /* READ ONLY */
#define ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS_TOTAL_OFST         (16)



/* RTP encapsulation register */
#define ALTERA_AVALON_RTP_TX_RTP_ENCAP_REG                          (8*4)
#define IORD_ALTERA_AVALON_RTP_TX_RTP_ENCAP(base)                   HOI_RD32((base), ALTERA_AVALON_RTP_TX_RTP_ENCAP_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_RTP_ENCAP(base, data)             HOI_WR32((base), ALTERA_AVALON_RTP_TX_RTP_ENCAP_REG, (data))

#define ALTERA_AVALON_RTP_TX_RTP_ENCAP_NUM_TS_PACKETS_MSK           (0x00000007)
#define ALTERA_AVALON_RTP_TX_RTP_ENCAP_NUM_TS_PACKETS_OFST          (0)
#define ALTERA_AVALON_RTP_TX_RTP_ENCAP_ENABLE_MSK                   (0x00000100)
#define ALTERA_AVALON_RTP_TX_RTP_ENCAP_ENABLE_OFST                  (8)


/* Transport stream packet size register */
#define ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE_REG                     (9*4)
#define IORD_ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE(base)              HOI_RD32((base), ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE(base, data)        HOI_WR32((base), ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE_REG, (data))

#define ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE_SIZE_MSK                (0x000000FF)
#define ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE_SIZE_OFST               (0)


/* Queue select register */
#define ALTERA_AVALON_RTP_TX_QUEUE_SELECT_REG                       (10*4)
#define IORD_ALTERA_AVALON_RTP_TX_QUEUE_SELECT(base)                HOI_RD32((base), ALTERA_AVALON_RTP_TX_QUEUE_SELECT_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_QUEUE_SELECT(base, data)          HOI_WR32((base), ALTERA_AVALON_RTP_TX_QUEUE_SELECT_REG, (data))

#define ALTERA_AVALON_RTP_TX_QUEUE_SELECT_SEL_MSK                   (0x7FFFFFFF)
#define ALTERA_AVALON_RTP_TX_QUEUE_SELECT_SEL_OFST                  (0)
#define ALTERA_AVALON_RTP_TX_QUEUE_SELECT_PAUSE_MSK                 (0x80000000)
#define ALTERA_AVALON_RTP_TX_QUEUE_SELECT_PAUSE_OFST                (31)


/* Queue level register - READ ONLY */
#define ALTERA_AVALON_RTP_TX_QUEUE_LEVEL_REG                        (11*4)
#define IORD_ALTERA_AVALON_RTP_TX_QUEUE_LEVEL(base)                 HOI_RD32((base), ALTERA_AVALON_RTP_TX_QUEUE_LEVEL_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_TX_QUEUE_LEVEL(base, data)           (1=0)

#define ALTERA_AVALON_RTP_TX_QUEUE_LEVEL_LEVEL_MSK                  (0xFFFFFFFF)
#define ALTERA_AVALON_RTP_TX_QUEUE_LEVEL_LEVEL_OFST                 (0)


/* Version register - READ ONLY */
#define ALTERA_AVALON_RTP_TX_VERSION_REG                            (12*4)
#define IORD_ALTERA_AVALON_RTP_TX_VERSION(base)                     HOI_RD32((base), ALTERA_AVALON_RTP_TX_VERSION_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_TX_VERSION(base, data)               (1=0)

#define ALTERA_AVALON_RTP_TX_VERSION_VERSION_MSK                    (0xFFFFFFFF)
#define ALTERA_AVALON_RTP_TX_VERSION_VERSION_OFST                   (0)


/* Design parameter register - READ ONLY*/
#define ALTERA_AVALON_RTP_TX_PARAMS_REG                             (13*4)
#define IORD_ALTERA_AVALON_RTP_TX_PARAMS(base)                      HOI_RD32((base), ALTERA_AVALON_RTP_TX_PARAMS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_TX_PARAMS(base, data)                (1=0)

#define ALTERA_AVALON_RTP_TX_PARAMS_NUM_CHANNELS_MSK                (0x00001FFF)
#define ALTERA_AVALON_RTP_TX_PARAMS_NUM_CHANNELS_OFST               (0)
#define ALTERA_AVALON_RTP_TX_PARAMS_BYPASS_ENABLED_MSK              (0x00002000)
#define ALTERA_AVALON_RTP_TX_PARAMS_BYPASS_ENABLED_OFST             (13)
#define ALTERA_AVALON_RTP_TX_PARAMS_ADD_RTP_MSK                     (0x00004000)
#define ALTERA_AVALON_RTP_TX_PARAMS_ADD_RTP_OFST                    (14)
#define ALTERA_AVALON_RTP_TX_PARAMS_HAS_FEC_MSK                     (0x00008000)
#define ALTERA_AVALON_RTP_TX_PARAMS_HAS_FEC_OFST                    (15)
#define ALTERA_AVALON_RTP_TX_PARAMS_NUM_POINTERS_MSK                (0xFFFF0000)
#define ALTERA_AVALON_RTP_TX_PARAMS_NUM_POINTERS_OFST               (16)


/* FEC parameter register - READ ONLY*/
#define ALTERA_AVALON_RTP_TX_FEC_PARAMS_REG                         (14*4)
#define IORD_ALTERA_AVALON_RTP_TX_FEC_PARAMS(base)                  HOI_RD32((base), ALTERA_AVALON_RTP_TX_FEC_PARAMS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_TX_FEC_PARAMS(base, data)            (1=0)

#define ALTERA_AVALON_RTP_TX_FEC_PARAMS_MAX_L_MSK                   (0x000000FF)
#define ALTERA_AVALON_RTP_TX_FEC_PARAMS_MAX_L_OFST                  (0)
#define ALTERA_AVALON_RTP_TX_FEC_PARAMS_MAX_D_MSK                   (0x0000FF00)
#define ALTERA_AVALON_RTP_TX_FEC_PARAMS_MAX_D_OFST                  (8)


/* Max output queue depth register */
#define ALTERA_AVALON_RTP_TX_MAX_OUTPUT_DEPTH_REG                   (15*4)
#define IORD_ALTERA_AVALON_RTP_TX_MAX_OUTPUT_DEPTH(base)            HOI_RD32((base), ALTERA_AVALON_RTP_TX_MAX_OUTPUT_DEPTH_REG)
#define IOWR_ALTERA_AVALON_RTP_TX_MAX_OUTPUT_DEPTH(base, data)      HOI_WR32((base), ALTERA_AVALON_RTP_TX_MAX_OUTPUT_DEPTH_REG, (data))

#define ALTERA_AVALON_RTP_TX_MAX_OUTPUT_DEPTH_MSK                   (0xFFFFFFFF)
#define ALTERA_AVALON_RTP_TX_MAX_OUTPUT_DEPTH_OFST                  (0)

#endif /* __ALT_AVALON_RTP_TX_REGS_H__ */
