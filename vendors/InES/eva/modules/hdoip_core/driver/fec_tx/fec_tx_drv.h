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

#ifndef __ALT_AVALON_RTP_TX_API_H__
#define __ALT_AVALON_RTP_TX_API_H__

#include "std.h"
#include "stdeth.h"

typedef struct
{
   void* base;                              /* Base address of the device */
} alt_avalon_rtp_tx_dev;

/* Structure containing design parameters for the transmitter */
typedef struct
{
   uint32_t Version;
   uint32_t NumChannels;
   uint32_t HasRTPEncapsulator;
   uint32_t HasFECGenerator;
   uint32_t NumFECFrameBufferEntries;
   uint32_t HasPacketCorrupter;
   uint32_t NumQueues;
   uint8_t  MaxFECL;
   uint8_t  MaxFECD;
} alt_avalon_rtp_tx_design_parameters;

/* Enumeration containing FEC interleaver modes */
typedef enum
{
   alt_avalon_rtp_tx_interleave_mode_off,
   alt_avalon_rtp_tx_interleave_mode_annex_a,
   alt_avalon_rtp_tx_interleave_mode_annex_b
} alt_avalon_rtp_tx_interleave_mode;

/* Structure containing FEC settings for a single channel */
typedef struct
{
   uint32_t EnableFEC;
   uint32_t L;
   uint32_t D;
   uint32_t ColFECOnly;
   alt_avalon_rtp_tx_interleave_mode InterleaveMode;
} alt_avalon_rtp_tx_fec_settings;

/* Structure containing encapsulator settings for a single channel */
typedef struct
{
   uint32_t RTPEncapEnable;
   uint32_t RTPEncapNumTSPackets;
   uint32_t TSPacketSize;
} alt_avalon_rtp_tx_encap_settings;

/* Structure containing settings for the burst error generator */
typedef struct
{
   uint32_t ChannelToCorrupt;
   uint32_t GapLength;
   uint32_t BurstLength;
   uint32_t Repeat;
} alt_avalon_rtp_tx_burst_error_settings;

/* Structure for returned debug values */
typedef struct
{
   uint32_t RamWriteCount;
   uint32_t RamReadCount;
   uint32_t RamAccessCount;
   uint32_t RamBusyCount;
   uint32_t PayloadInReadyCount;
   uint32_t PayloadInValidCount;
   uint32_t PayloadOutReadyCount;
   uint32_t PayloadOutValidCount;
   uint32_t QueueLevelsWrong;
   uint32_t FreelistLevel;
   uint32_t TransmitQueueLevel;
   uint32_t FecQueueLevel;
   uint32_t GenQueueLevel;
   uint32_t DmaQueueLevel;
   uint32_t DataQueueLevel;
} alt_avalon_rtp_tx_debug_values;

/* set eth parameters for eth, ip and udp header */
void fec_drv_set_video_eth_params(void *p, hdoip_eth_params *eth);

/* set eth parameters for eth, ip and udp header */
void fec_drv_set_audio_eth_params(void *p, hdoip_eth_params *eth);

/* set ringbuffer descriptors and burst size for dma access */
void fec_drv_set_descriptors(void *p, void *vid_tx_buf, void *aud_tx_buf, size_t vid_tx_len, size_t aud_tx_len, uint32_t burst_size);

/* start the IP */
void start_fec_ip_tx(void* p_fec_ip_tx, int enable_vid, int d_vid, int l_vid, int interleave_vid, int single_mode_vid, int enable_aud, int d_aud, int l_aud, int interleave_aud, int single_mode_aud);

/* change settings */
void change_setting_fec_ip_tx(void* p_fec_ip_tx, int enable_vid, int d_vid, int l_vid, int interleave_vid, int single_mode_vid, int enable_aud, int d_aud, int l_aud, int interleave_aud, int single_mode_aud);

/* Get the parameters of a design */
void alt_avalon_rtp_tx_get_parameters(alt_avalon_rtp_tx_dev* pDev, alt_avalon_rtp_tx_design_parameters* pParams);

/* Enable the RTP transmitter */
void alt_avalon_rtp_tx_enable(alt_avalon_rtp_tx_dev* pDev);

/* Disable the RTP transmitter */
void alt_avalon_rtp_tx_disable(alt_avalon_rtp_tx_dev* pDev);

/* Determine if the RTP transmitter is enabled */
int alt_avalon_rtp_tx_is_enabled(alt_avalon_rtp_tx_dev* pDev);

/* Setup the FEC parameters for a single channel */
void alt_avalon_rtp_tx_set_fec_config(alt_avalon_rtp_tx_dev* pDev, uint32_t Channel, alt_avalon_rtp_tx_fec_settings* pSettings);

/* Get the FEC parameters for a single channel */
void alt_avalon_rtp_tx_get_fec_config(alt_avalon_rtp_tx_dev* pDev, uint32_t Channel, alt_avalon_rtp_tx_fec_settings* pSettings);

/* Setup the encapsulation parameters for a single channel */
void alt_avalon_rtp_tx_set_encap_config(alt_avalon_rtp_tx_dev* pDev, uint32_t Channel, alt_avalon_rtp_tx_encap_settings* pSettings);

/* Get the encapsulation parameters for a single channel */
void alt_avalon_rtp_tx_get_encap_config(alt_avalon_rtp_tx_dev* pDev, uint32_t Channel, alt_avalon_rtp_tx_encap_settings* pSettings);

/* Configures burst error generation */
void alt_avalon_rtp_tx_set_burst_error_config(alt_avalon_rtp_tx_dev* pDev, alt_avalon_rtp_tx_burst_error_settings* pSettings);

/* Gets burst error generation settings*/
void alt_avalon_rtp_tx_get_burst_error_config(alt_avalon_rtp_tx_dev* pDev, alt_avalon_rtp_tx_burst_error_settings* pSettings);

/* Get the minimum level of the free list */
uint32_t alt_avalon_rtp_tx_get_minimum_freelist_level(alt_avalon_rtp_tx_dev* pDev);

/* Get the level of a single queue */
uint32_t alt_avalon_rtp_tx_get_single_queue_level(alt_avalon_rtp_tx_dev* pDev, uint32_t Queue);

/* Get the level of all queues - pLevels must point to enough memory to store the levels
** for all channels */
void alt_avalon_rtp_tx_get_all_queue_levels(alt_avalon_rtp_tx_dev* pDev, uint32_t* pLevels);

/* Set maximum output queue depth */
void alt_avalon_rtp_tx_set_max_output_depth(alt_avalon_rtp_tx_dev* pDev, uint32_t Depth);

/* Get maximum output queue depth */
uint32_t alt_avalon_rtp_tx_get_max_output_depth(alt_avalon_rtp_tx_dev* pDev);

/* Get debug values */
void alt_avalon_rtp_tx_get_debug_values(alt_avalon_rtp_tx_dev* pDev, alt_avalon_rtp_tx_debug_values* pDebug);

#endif /* __ALT_AVALON_RTP_TX_API_H__ */
