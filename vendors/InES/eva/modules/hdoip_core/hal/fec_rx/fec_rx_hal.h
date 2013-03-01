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

#ifndef __ALT_AVALON_RTP_RX_REGS_H__
#define __ALT_AVALON_RTP_RX_REGS_H__

#include "stdhal.h"

/* Registers of Interface Out Block */
#define fec_rx_enable_video_out(base)                                       HOI_WR32((base), 0, 1);
#define fec_rx_enable_audio_emb_out(base)                                   HOI_WR32((base), 0, 2);
#define fec_rx_enable_audio_int_out(base)                                   HOI_WR32((base), 0, 4);

#define fec_rx_disable_video_out(base)                                      HOI_REG_CLR((base), 0, 1);
#define fec_rx_disable_audio_emb_out(base)                                  HOI_REG_CLR((base), 0, 2);
#define fec_rx_disable_audio_int_out(base)                                  HOI_REG_CLR((base), 0, 4);

/* Registers of FEC RX IP */
#define ALTERA_AVALON_RTP_RX_CHANNEL_REG                                    0
#define ALTERA_AVALON_RTP_RX_VALID_PACKETS_REG                              4
#define ALTERA_AVALON_RTP_RX_MISSING_PACKETS_REG                            8
#define ALTERA_AVALON_RTP_RX_FIXED_PACKETS_REG                              12
#define ALTERA_AVALON_RTP_RX_DUPLICATE_PACKETS_REG                          16
#define ALTERA_AVALON_RTP_RX_REORDERED_PACKETS_REG                          20
#define ALTERA_AVALON_RTP_RX_OUT_OF_RANGE_PACKETS_REG                       24
#define ALTERA_AVALON_RTP_RX_FEC_MODE_REG                                   28
#define ALTERA_AVALON_RTP_RX_FEC_STATS_REG                                  32
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_REG                              36
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES_REG                           40
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_BASE_REG                              44
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS_REG                            48
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA_REG                              52
#define ALTERA_AVALON_RTP_RX_DEBUG_REG                                      56
#define ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG_REG                          60
#define ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_REG                          64
#define ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM_REG                       68
#define ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_TSTAMP_REG                        72
#define ALTERA_AVALON_RTP_RX_FB_ENTRIES_REG                                 76
#define ALTERA_AVALON_RTP_RX_FEC_POINTERS_REG                               80
#define ALTERA_AVALON_RTP_RX_CONFIG_REG                                     84
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_REG                              88
#define ALTERA_AVALON_RTP_RX_NUM_CHANNELS_REG                               92
#define ALTERA_AVALON_RTP_RX_VERSION_REG                                    96
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_REG                              100
#define ALTERA_AVALON_RTP_RX_EVENT_FIFO_DEPTH_REG                           104
#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_REG                               112
#define ALTERA_AVALON_RTP_RX_READY_STATUS_REG                               116
#define ALTERA_AVALON_RTP_RX_BUFFER_TIMESTAMP_REG                           120


/*
** Address     R/W     Use
** =======     ===     ===
** 0           R/W     Channel select
*/
#define IORD_ALTERA_AVALON_RTP_RX_CHANNEL(base)                             HOI_RD32(base, ALTERA_AVALON_RTP_RX_CHANNEL_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(base, data)                       HOI_WR32(base, ALTERA_AVALON_RTP_RX_CHANNEL_REG, data)

#define ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK                                (0x0000FFFF)
#define ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST                               (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 21          R/W     [0] Enable for selected channel (enabled (1) by default)
**                     [1] RTP bypass for selected channel (disabled (0) by default)
**                     [26:16] Last payload length received for the selected channel. Clears on any write
**                     [31] Reads as 1 if the selected channel has been flushed. Clears on write of 1.
*/
#define IORD_ALTERA_AVALON_RTP_RX_CONFIG(base)                              HOI_RD32(base, ALTERA_AVALON_RTP_RX_CONFIG_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_CONFIG(base, data)                        HOI_WR32(base, ALTERA_AVALON_RTP_RX_CONFIG_REG, data)

#define ALTERA_AVALON_RTP_RX_CONFIG_CHANNEL_ENABLE_MSK                      (0x00000001)
#define ALTERA_AVALON_RTP_RX_CONFIG_CHANNEL_ENABLE_OFST                     (0)
#define ALTERA_AVALON_RTP_RX_CONFIG_RTP_BYPASS_MSK                          (0x00000002)
#define ALTERA_AVALON_RTP_RX_CONFIG_RTP_BYPASS_OFST                         (1)
#define ALTERA_AVALON_RTP_RX_CONFIG_LAST_PAYLOAD_LENGTH_MSK                 (0x07FF0000)   /* Read only - clears on any write */
#define ALTERA_AVALON_RTP_RX_CONFIG_LAST_PAYLOAD_LENGTH_OFST                (16)
#define ALTERA_AVALON_RTP_RX_CONFIG_HAS_FLUSHED_MSK                         (0x80000000)   /* Read only - clears on write of 1 */
#define ALTERA_AVALON_RTP_RX_CONFIG_HAS_FLUSHED_OFST                        (31)




/*
** Address     R/W     Use
** =======     ===     ===
** 1           R       Valid packets count for selected channel (clears on read)
*/
#define IORD_ALTERA_AVALON_RTP_RX_VALID_PACKETS(base)                       HOI_RD32(base, ALTERA_AVALON_RTP_RX_VALID_PACKETS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_VALID_PACKETS(base, data)                 (1=0)

#define ALTERA_AVALON_RTP_RX_VALID_PACKETS_COUNT_MSK                        (0xFFFFFFFF)   /* Read only - clears on read */
#define ALTERA_AVALON_RTP_RX_VALID_PACKETS_COUNT_OFST                       (0)

/* The channel has been flushed if this register contains -1 */
#define ALTERA_AVALON_RTP_RX_VALID_PACKETS_CHANNEL_FLUSHED                  (0xFFFFFFFF)



/*
** Address     R/W     Use
** =======     ===     ===
** 2           R       Missing packets count for selected channel (clears on read)
*/
#define IORD_ALTERA_AVALON_RTP_RX_MISSING_PACKETS(base)                     HOI_RD32(base, ALTERA_AVALON_RTP_RX_MISSING_PACKETS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_MISSING_PACKETS(base, data)               (1=0)

#define ALTERA_AVALON_RTP_RX_MISSING_PACKETS_COUNT_MSK                      (0xFFFFFFFF)   /* Read only - clears on read */
#define ALTERA_AVALON_RTP_RX_MISSING_PACKETS_COUNT_OFST                     (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 3           R       Fixed packets count for selected channel (clears on read)
*/
#define IORD_ALTERA_AVALON_RTP_RX_FIXED_PACKETS(base)                       HOI_RD32(base, ALTERA_AVALON_RTP_RX_FIXED_PACKETS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_FIXED_PACKETS(base, data)                 (1=0)

#define ALTERA_AVALON_RTP_RX_FIXED_PACKETS_COUNT_MSK                        (0xFFFFFFFF)   /* Read only - clears on read */
#define ALTERA_AVALON_RTP_RX_FIXED_PACKETS_COUNT_OFST                       (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 4           R       Duplicate packet count for selected channel (clears on read)
*/
#define IORD_ALTERA_AVALON_RTP_RX_DUPLICATE_PACKETS(base)                   HOI_RD32(base, ALTERA_AVALON_RTP_RX_DUPLICATE_PACKETS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_DUPLICATE_PACKETS(base, data)             (1=0)

#define ALTERA_AVALON_RTP_RX_DUPLICATE_PACKETS_COUNT_MSK                    (0xFFFFFFFF)   /* Read only - clears on read */
#define ALTERA_AVALON_RTP_RX_DUPLICATE_PACKETS_COUNT_OFST                   (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 5           R       Reordered packet count for selected channel (clears on read)
*/
#define IORD_ALTERA_AVALON_RTP_RX_REORDERED_PACKETS(base)                   HOI_RD32(base, ALTERA_AVALON_RTP_RX_REORDERED_PACKETS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_REORDERED_PACKETS(base, data)             (1=0)

#define ALTERA_AVALON_RTP_RX_REORDERED_PACKETS_COUNT_MSK                    (0xFFFFFFFF)   /* Read only - clears on read */
#define ALTERA_AVALON_RTP_RX_REORDERED_PACKETS_COUNT_OFST                   (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 6           R       Out of range packet count for selected channel (clears on read)
*/
#define IORD_ALTERA_AVALON_RTP_RX_OUT_OF_RANGE_PACKETS(base)                HOI_RD32(base, ALTERA_AVALON_RTP_RX_OUT_OF_RANGE_PACKETS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_OUT_OF_RANGE_PACKETS(base, data)          (1=0)

#define ALTERA_AVALON_RTP_RX_OUT_OF_RANGE_PACKETS_COUNT_MSK                 (0xFFFFFFFF)   /* Read only - clears on read */
#define ALTERA_AVALON_RTP_RX_OUT_OF_RANGE_PACKETS_COUNT_OFST                (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 7           R       FEC mode detected for selected channel. [Redefined v1.3.0]
**                        [17]    1 if row FEC has been seen
**                        [16]    1 if column FEC has been seen
**                        [15:8]  Last detected D
**                        [7:0]   Last detected L
*/
#define IORD_ALTERA_AVALON_RTP_RX_FEC_MODE(base)                            HOI_RD32(base, ALTERA_AVALON_RTP_RX_FEC_MODE_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_FEC_MODE(base, data)                      (1=0)

#define ALTERA_AVALON_RTP_RX_FEC_MODE_L_MSK                                 (0x000000FF)   /* Read only */
#define ALTERA_AVALON_RTP_RX_FEC_MODE_L_OFST                                (0)
#define ALTERA_AVALON_RTP_RX_FEC_MODE_D_MSK                                 (0x0000FF00)   /* Read only */
#define ALTERA_AVALON_RTP_RX_FEC_MODE_D_OFST                                (8)
#define ALTERA_AVALON_RTP_RX_FEC_MODE_ROW_SEEN_MSK                          (0x00020000)   /* Read only */
#define ALTERA_AVALON_RTP_RX_FEC_MODE_ROW_SEEN_OFST                         (17)
#define ALTERA_AVALON_RTP_RX_FEC_MODE_COL_SEEN_MSK                          (0x00010000)   /* Read only */
#define ALTERA_AVALON_RTP_RX_FEC_MODE_COL_SEEN_OFST                         (16)




/*
** Address     R/W     Use
** =======     ===     ===
** 17          R       Buffer depth (based on sequence number) for selected channel
*/
#define IORD_ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM(base)                HOI_RD32(base, ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM(base, data)          (1=0)

#define ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM_COUNT_MSK                 (0xFFFFFFFF)   /* Read only */
#define ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM_COUNT_OFST                (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 18          R       Buffer depth (based on timestamp) for selected channel
*/
#define IORD_ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_TSTAMP(base)                 HOI_RD32(base, ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_TSTAMP_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_TSTAMP(base, data)           (1=0)

#define ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_TSTAMP_COUNT_MSK                  (0xFFFFFFFF)   /* Read only */
#define ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_TSTAMP_COUNT_OFST                 (0)


/*
** Address     R/W     Use
** =======     ===     ===
** 30          R       Base timestamp for selected channel
*/
#define IORD_ALTERA_AVALON_RTP_RX_BUFFER_TIMESTAMP(base)                    HOI_RD32(base, ALTERA_AVALON_RTP_RX_BUFFER_TIMESTAMP_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_BUFFER_TIMESTAMP(base, data)              (1=0)

#define ALTERA_AVALON_RTP_RX_BUFFER_TIMESTAMP_MSK                           (0xFFFFFFFF)   /* Read only */
#define ALTERA_AVALON_RTP_RX_BUFFER_TIMESTAMP_OFST                          (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 19          R       Value of P_ENTRIES_PER_CHANNEL
*/
#define IORD_ALTERA_AVALON_RTP_RX_FB_ENTRIES(base)                          HOI_RD32(base, ALTERA_AVALON_RTP_RX_FB_ENTRIES_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_FB_ENTRIES(base, data)                    (1=0)

#define ALTERA_AVALON_RTP_RX_FB_ENTRIES_COUNT_MSK                           (0xFFFFFFFF)   /* Read only */
#define ALTERA_AVALON_RTP_RX_FB_ENTRIES_COUNT_OFST                          (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 20          R       Value of P_FEC_POINTERS
*/
#define IORD_ALTERA_AVALON_RTP_RX_FEC_POINTERS(base)                        HOI_RD32(base, ALTERA_AVALON_RTP_RX_FEC_POINTERS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_FEC_POINTERS(base, data)                  (1=0)

#define ALTERA_AVALON_RTP_RX_FEC_POINTERS_COUNT_MSK                         (0xFFFFFFFF)   /* Read only */
#define ALTERA_AVALON_RTP_RX_FEC_POINTERS_COUNT_OFST                        (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 23          R       Value of P_CHANNELS
*/
#define IORD_ALTERA_AVALON_RTP_RX_NUM_CHANNELS(base)                        HOI_RD32(base, ALTERA_AVALON_RTP_RX_NUM_CHANNELS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_NUM_CHANNELS(base, data)                  (1=0)

#define ALTERA_AVALON_RTP_RX_NUM_CHANNELS_COUNT_MSK                         (0xFFFFFFFF)   /* Read only */
#define ALTERA_AVALON_RTP_RX_NUM_CHANNELS_COUNT_OFST                        (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 24          R       Version number
*/
#define IORD_ALTERA_AVALON_RTP_RX_VERSION(base)                             HOI_RD32(base, ALTERA_AVALON_RTP_RX_VERSION_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_VERSION(base, data)                       (1=0)

#define ALTERA_AVALON_RTP_RX_VERSION_VERSION_MSK                            (0xFFFFFFFF)   /* Read only */
#define ALTERA_AVALON_RTP_RX_VERSION_VERSION_OFST                           (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 25          R/W     [0] Enable/disable for P, X, CC and M recovery. Enabled by default
**                     [1] Write 1 to disable receive FEC fix operation (enabled by default)
**                     [2] Write 1 to disable receive FEC load operation (enabled by default)
**                     [3] Write 1 to disable checking of RTP PT field (enabled by default)
**                     [4] Write 1 to pause FEC decoder for debug purposes (enabled by default). P_DEBUG must be 1
**                     [5] Write 1 to force data packets to be discarded for test purposes. P_DEBUG must be 1
**                     [6] Write 1 to force FEC packets to be discarded for test purposes. P_DEBUG must be 1
*/
#define IORD_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(base)                       HOI_RD32(base, ALTERA_AVALON_RTP_RX_EXTENDED_MODE_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(base, data)                 HOI_WR32(base, ALTERA_AVALON_RTP_RX_EXTENDED_MODE_REG, data)

#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_HEADER_EXT_MSK                   (0x00000001)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_HEADER_EXT_OFST                  (0)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_FEC_FIX_DISABLE_MSK              (0x00000002)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_FEC_FIX_DISABLE_OFST             (1)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_FEC_LOAD_DISABLE_MSK             (0x00000004)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_FEC_LOAD_DISABLE_OFST            (2)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_PT_CHECK_DISABLE_MSK             (0x00000008)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_PT_CHECK_DISABLE_OFST            (3)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_FEC_PAUSE_MSK                    (0x00000010)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_FEC_PAUSE_OFST                   (4)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_DISCARD_DATA_MSK                 (0x00000020)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_DISCARD_DATA_OFST                (5)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_DISCARD_FEC_MSK                  (0x00000040)
#define ALTERA_AVALON_RTP_RX_EXTENDED_MODE_DISCARD_FEC_OFST                 (6)




/*
** Address     R/W     Use
** =======     ===     ===
** 9           R/W     Message FIFO control
**                     [31:16] Channel select for single channel mode
**                     [3] 1=single channel mode
**                     [2] 1=stop on abnormal event
**                     [1] 1=stop on error (missing packet)
**                     [0] 1=enable
*/
#define IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL(base)                       HOI_RD32(base, ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL(base, data)                 HOI_WR32(base, ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_REG, data)

#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_ENABLE_MSK                       (0x00000001)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_ENABLE_OFST                      (0)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_STOP_ON_ERROR_MSK                (0x00000002)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_STOP_ON_ERROR_OFST               (1)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_STOP_ON_ABNORMAL_MSK             (0x00000004)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_STOP_ON_ABNORMAL_OFST            (2)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_SINGLE_CHANNEL_MSK               (0x00000008)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_SINGLE_CHANNEL_OFST              (3)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_CHANNEL_SELECT_MSK               (0xFFFF0000)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_CHANNEL_SELECT_OFST              (16)




/*
** Address     R/W     Use
** =======     ===     ===
** 10          R       Message FIFO entries
**                     [30:0] Number of messages in the FIFO
**                     [31] 1 if FIFO is full
*/
#define IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES(base)                    HOI_RD32(base, ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES(base, data)              (1=0)

#define ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES_COUNT_MSK                     (0x7FFFFFFF)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES_COUNT_OFST                    (0)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES_FULL_MSK                      (0x80000000)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES_FULL_OFST                     (31)




/*
** Address     R/W     Use
** =======     ===     ===
** 11          R       Message FIFO base address
*/
#define IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_BASE(base)                       HOI_RD32(base, ALTERA_AVALON_RTP_RX_MSG_FIFO_BASE_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_BASE(base, data)                 (1=0)

#define ALTERA_AVALON_RTP_RX_MSG_FIFO_BASE_ADDRESS_MSK                      (0xFFFFFFFF)   /* Read only */
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_BASE_ADDRESS_OFST                     (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 12          R/W     Message FIFO access address
*/
#define IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS(base)                     HOI_RD32(base, ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS(base, data)               HOI_WR32(base, ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS_REG, data)

#define ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS_ADDRESS_MSK                    (0xFFFFFFFF)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS_ADDRESS_OFST                   (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 13          R       Message FIFO data
*/
#define IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA(base)                       HOI_RD32(base, ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA(base, data)                 (1=0)

#define ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA_VALUE_MSK                        (0xFFFFFFFF)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA_VALUE_OFST                       (0)




/*
** Address     R/W     Use
** =======     ===     ===
** 22          R/W     Message FIFO mask. Set individual bits to disable related message
**                      0 : Packet input  - OK
**                      1 : Packet input  - Out of range
**                      2 : Packet input  - Flush
**                      3 : Packet input  - Duplicate
**                      4 : Packet input  - FEC
**                      5 : Packet input  - FEC discard
**                      6 : Packet input  - Depth update
**                      7 : Packet input  - Packet reorder
**                      8 : Packet output - Empty
**                      9 : Packet input  - New length detected
**                     10 : Packet output - Missing
**                     11 : Packet output - OK
**                     12 : Not used
**                     13 : Packet output - Depth update
**                     14 : FEC           - Fix
**                     15 : FEC           - Problem
*/
#define IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK(base)                       HOI_RD32(base, ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK(base, data)                 HOI_WR32(base, ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_REG, data)

#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_OK_MSK                        (0x00000001)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_OK_OFST                       (0)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_RANGE_MSK                     (0x00000002)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_RANGE_OFST                    (1)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_FLUSH_MSK                     (0x00000004)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_FLUSH_OFST                    (2)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_DUPLICATE_MSK                 (0x00000008)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_DUPLICATE_OFST                (3)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_FEC_MSK                       (0x00000010)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_FEC_OFST                      (4)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_FEC_DISCARD_MSK               (0x00000020)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_FEC_DISCARD_OFST              (5)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_DEPTH_UPDATE_MSK              (0x00000040)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_DEPTH_UPDATE_OFST             (6)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_REORDER_MSK                   (0x00000080)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_REORDER_OFST                  (7)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_OUT_EMPTY_MSK                    (0x00000100)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_OUT_EMPTY_OFST                   (8)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_NEW_LENGTH_MSK                (0x00000200)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_IN_NEW_LENGTH_OFST               (9)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_OUT_MISSING_MSK                  (0x00000400)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_OUT_MISSING_OFST                 (10)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_OUT_OK_MSK                       (0x00000800)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_OUT_OK_OFST                      (11)
/*                                                     Not used - (0x00001000) */
/*                                                     Not used - (12)         */
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_OUT_DEPTH_UPDATE_MSK             (0x00002000)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_OUT_DEPTH_UPDATE_OFST            (13)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_FEC_FIX_MSK                      (0x00004000)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_FEC_FIX_OFST                     (14)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_FEC_PROBLEM_MSK                  (0x00008000)
#define ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK_FEC_PROBLEM_OFST                 (15)




/*
** Address     R/W     Use
** =======     ===     ===
** 26          R/W     [31] Set to 1 to stall event FIFO if it gets full. Default is 0
**                     [30:0] Maximum event FIFO depth reached. Write the register to reset
*/
#define IORD_ALTERA_AVALON_RTP_RX_EVENT_FIFO_DEPTH(base)                    HOI_RD32(base, ALTERA_AVALON_RTP_RX_EVENT_FIFO_DEPTH_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_EVENT_FIFO_DEPTH(base, data)              HOI_WR32(base, ALTERA_AVALON_RTP_RX_EVENT_FIFO_DEPTH_REG, data)

#define ALTERA_AVALON_RTP_RX_EVENT_FIFO_DEPTH_MAXIMUM_MSK                   (0x7FFFFFFF)  /* Read only - clears on write */
#define ALTERA_AVALON_RTP_RX_EVENT_FIFO_DEPTH_MAXIMUM_OFST                  (0)
#define ALTERA_AVALON_RTP_RX_EVENT_FIFO_DEPTH_STALL_WHEN_FULL_MSK           (0x80000000)
#define ALTERA_AVALON_RTP_RX_EVENT_FIFO_DEPTH_STALL_WHEN_FULL_OFST          (31)


/*
** Address     R/W     Use
** =======     ===     ===
** 28          R       Pending flush status
*/
#define IORD_ALTERA_AVALON_RTP_RX_FLUSH_STATUS_REG(base)                    HOI_RD32(base, ALTERA_AVALON_RTP_RX_FLUSH_STATUS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_FLUSH_STATUS_REG(base, data)              (1=0)

#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_FEC_MSK                           (0x00000001)
#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_FEC_OFST                          (0)

#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_EVENT_MSK                         (0x00000002)
#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_EVENT_OFST                        (1)

#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_OUTPUT_MSK                        (0x00000004)
#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_OUTPUT_OFST                       (2)

#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_INPUT_MSK                         (0x00000008)
#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_INPUT_OFST                        (3)

#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_STATUS_MSK                        (0x00000010)
#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_STATUS_OFST                       (4)

#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_AVALON_MSK                        (0x00000020)
#define ALTERA_AVALON_RTP_RX_FLUSH_STATUS_AVALON_OFST                       (5)

/*
** Address     R/W     Use
** =======     ===     ===
** 29          R       Initialisation status. Reads as 1 once complete and ready for data
*/
#define IORD_ALTERA_AVALON_RTP_RX_READY_STATUS_REG(base)                    HOI_RD32(base, ALTERA_AVALON_RTP_RX_READY_STATUS_REG)
/* READ ONLY - this macro ensures a compilation error if a write is attempted */
#define IOWR_ALTERA_AVALON_RTP_RX_READY_STATUS_REG(base, data)              (1=0)

#define ALTERA_AVALON_RTP_RX_READY_STATUS_MSK                               (0x00000001)
#define ALTERA_AVALON_RTP_RX_READY_STATUS_OFST                              (0)


/* Debug register */
#define IORD_ALTERA_AVALON_RTP_RX_DEBUG(base)                               HOI_RD32(base, ALTERA_AVALON_RTP_RX_DEBUG_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_DEBUG(base, data)                         HOI_WR32(base, ALTERA_AVALON_RTP_RX_DEBUG_REG, data)

#define ALTERA_AVALON_RTP_RX_DEBUG_MSK                                      (0x0000000F)
#define ALTERA_AVALON_RTP_RX_DEBUG_OFST                                     (0)


/* FEC stats register */
#define IORD_ALTERA_AVALON_RTP_RX_FEC_STATS(base)                           HOI_RD32(base, ALTERA_AVALON_RTP_RX_FEC_STATS_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_FEC_STATS(base, data)                     HOI_WR32(base, ALTERA_AVALON_RTP_RX_FEC_STATS_REG, data)

#define ALTERA_AVALON_RTP_RX_FEC_STATS_MSK                                  (0xFFFFFFFF)
#define ALTERA_AVALON_RTP_RX_FEC_STATS_OFST                                 (0)


/* FEC pointer debug register. P_DEBUG must be 1 */
#define IORD_ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG(base)                   HOI_RD32(base, ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG(base, data)             HOI_WR32(base, ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG_REG, data)

#define ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG_MSK                          (0xFFFFFFFF)
#define ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG_OFST                         (0)

/* FEC channel debug register. P_DEBUG must be 1 */
#define IORD_ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG(base)                   HOI_RD32(base, ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_REG)
#define IOWR_ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG(base, data)             HOI_WR32(base, ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_REG, data)

#define ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_BASE_MSK                     (0x0000FFFF)
#define ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_BASE_OFST                    (0)

#define ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_ENTRIES_MSK                  (0xFFFF0000)
#define ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_ENTRIES_OFST                 (16)

#endif /* __ALT_AVALON_RTP_RX_REGS_H__ */
