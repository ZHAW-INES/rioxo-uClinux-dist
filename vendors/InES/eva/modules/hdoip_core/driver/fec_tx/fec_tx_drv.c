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
 *  Modified for use in uClinux
 *  15.01.2013, buan
 */

#include <fec_tx_drv.h>
#include <fec_tx_hal.h>
#include <fec_tx_struct.h>


// turn diagnostic messages on and off
#define AUK_RTP_TX_API_DIAG(...)    printk(__VA_ARGS__)


void fec_drv_set_video_eth_params(void *p, hdoip_eth_params *eth)
{
    //video dst mac
    fec_tx_write_vid_dst_mac_0(p, eth->dst_mac[0]);
    fec_tx_write_vid_dst_mac_1(p, eth->dst_mac[1]);
    fec_tx_write_vid_dst_mac_2(p, eth->dst_mac[2]);
    fec_tx_write_vid_dst_mac_3(p, eth->dst_mac[3]);
    fec_tx_write_vid_dst_mac_4(p, eth->dst_mac[4]);
    fec_tx_write_vid_dst_mac_5(p, eth->dst_mac[5]);

    //video src mac
    fec_tx_write_vid_src_mac_0(p, eth->src_mac[0]);
    fec_tx_write_vid_src_mac_1(p, eth->src_mac[1]);
    fec_tx_write_vid_src_mac_2(p, eth->src_mac[2]);
    fec_tx_write_vid_src_mac_3(p, eth->src_mac[3]);
    fec_tx_write_vid_src_mac_4(p, eth->src_mac[4]);
    fec_tx_write_vid_src_mac_5(p, eth->src_mac[5]);

    //video tll/tos
    fec_tx_write_vid_ttl_tos(p, eth->ipv4_ttl, eth->ipv4_tos);
    //video src ip
    fec_tx_write_vid_src_ip(p, eth->ipv4_src_ip);
    //video dst ip
    fec_tx_write_vid_dst_ip(p, eth->ipv4_dst_ip);
    //video src port
    fec_tx_write_vid_src_port(p, eth->udp_src_port);
    //video dst port
    fec_tx_write_vid_dst_port(p, eth->udp_dst_port);
}

void fec_drv_set_audio_eth_params(void *p, hdoip_eth_params *eth)
{
    //audio dst mac
    fec_tx_write_aud_dst_mac_0(p, eth->dst_mac[0]);
    fec_tx_write_aud_dst_mac_1(p, eth->dst_mac[1]);
    fec_tx_write_aud_dst_mac_2(p, eth->dst_mac[2]);
    fec_tx_write_aud_dst_mac_3(p, eth->dst_mac[3]);
    fec_tx_write_aud_dst_mac_4(p, eth->dst_mac[4]);
    fec_tx_write_aud_dst_mac_5(p, eth->dst_mac[5]);
    //audio src mac
    fec_tx_write_aud_src_mac_0(p, eth->src_mac[0]);
    fec_tx_write_aud_src_mac_1(p, eth->src_mac[1]);
    fec_tx_write_aud_src_mac_2(p, eth->src_mac[2]);
    fec_tx_write_aud_src_mac_3(p, eth->src_mac[3]);
    fec_tx_write_aud_src_mac_4(p, eth->src_mac[4]);
    fec_tx_write_aud_src_mac_5(p, eth->src_mac[5]);
    //audio tll/tos
    fec_tx_write_aud_ttl_tos(p, eth->ipv4_ttl, eth->ipv4_tos);
    //audio src ip
    fec_tx_write_aud_src_ip(p, eth->ipv4_src_ip);
    //audio dst ip
    fec_tx_write_aud_dst_ip(p, eth->ipv4_dst_ip);
    //audio src port
    fec_tx_write_aud_src_port(p, eth->udp_src_port);
    //audio dst port
    fec_tx_write_aud_dst_port(p, eth->udp_dst_port);
}

void fec_drv_set_descriptors(void *p, void *vid_tx_buf, void *aud_tx_buf, size_t vid_tx_len, size_t aud_tx_len, uint32_t burst_size)
{
    t_rbf_dsc dsc;

    // set video descriptors
    rbf_dsc(&dsc, vid_tx_buf, vid_tx_len - MAX_FRAME_LENGTH);
    fec_tx_write_video_descriptors(p, dsc);

    // set audio descriptors
    rbf_dsc(&dsc, aud_tx_buf, aud_tx_len - MAX_FRAME_LENGTH);
    fec_tx_write_audio_descriptors(p, dsc);

    // set burst size
    fec_tx_write_burst_size(p, burst_size);
}

void start_fec_ip_tx(void* p_fec_ip_tx, int enable_vid, int d_vid, int l_vid, int interleave_vid, int single_mode_vid, int enable_aud, int d_aud, int l_aud, int interleave_aud, int single_mode_aud)
{
    alt_avalon_rtp_tx_dev p;

    p.base = p_fec_ip_tx;

    // set up fec tx block
    change_setting_fec_ip_tx(p_fec_ip_tx, enable_vid, d_vid, l_vid, interleave_vid, single_mode_vid, enable_aud, d_aud, l_aud, interleave_aud, single_mode_aud);

    // check if fec block is running already
    if (!alt_avalon_rtp_tx_is_enabled(&p)) {
        alt_avalon_rtp_tx_enable(&p);
    }
}

void change_setting_fec_ip_tx(void* p_fec_ip_tx, int enable_vid, int d_vid, int l_vid, int interleave_vid, int single_mode_vid, int enable_aud, int d_aud, int l_aud, int interleave_aud, int single_mode_aud)
{
    alt_avalon_rtp_tx_dev p;
    alt_avalon_rtp_tx_fec_settings fec_settings_channel_0;
    alt_avalon_rtp_tx_fec_settings fec_settings_channel_1;

    p.base = p_fec_ip_tx;

    // settings for video channel
    fec_settings_channel_0.EnableFEC = enable_vid;
    fec_settings_channel_0.L = l_vid;
    fec_settings_channel_0.D = d_vid;
    fec_settings_channel_0.ColFECOnly = single_mode_vid;

    switch (interleave_vid) {
        case 1:     fec_settings_channel_0.InterleaveMode = alt_avalon_rtp_tx_interleave_mode_annex_b;  break;
        case 2:     fec_settings_channel_0.InterleaveMode = alt_avalon_rtp_tx_interleave_mode_annex_a;  break;
        default:    fec_settings_channel_0.InterleaveMode = alt_avalon_rtp_tx_interleave_mode_off;
    }

    // settings for audio channel
    fec_settings_channel_1.EnableFEC = enable_aud;
    fec_settings_channel_1.L = l_aud;
    fec_settings_channel_1.D = d_aud;
    fec_settings_channel_1.ColFECOnly = single_mode_aud;

    switch (interleave_aud) {
        case 1:     fec_settings_channel_1.InterleaveMode = alt_avalon_rtp_tx_interleave_mode_annex_b;  break;
        case 2:     fec_settings_channel_1.InterleaveMode = alt_avalon_rtp_tx_interleave_mode_annex_a;  break;
        default:    fec_settings_channel_1.InterleaveMode = alt_avalon_rtp_tx_interleave_mode_off;
    }

    // configure channel 0 (video)
    alt_avalon_rtp_tx_set_fec_config(&p, 0x00, &fec_settings_channel_0);

    // configure channel 1 (audio)
    alt_avalon_rtp_tx_set_fec_config(&p, 0x01, &fec_settings_channel_1);
}

/* Get the parameters of a design */
void alt_avalon_rtp_tx_get_parameters(alt_avalon_rtp_tx_dev* pDev, alt_avalon_rtp_tx_design_parameters* pParams)
{
   uint32_t data;

   /* Get data from the version register */
   data = IORD_ALTERA_AVALON_RTP_TX_VERSION(pDev->base);

   pParams->Version = (data & ALTERA_AVALON_RTP_TX_VERSION_VERSION_MSK) >> ALTERA_AVALON_RTP_TX_VERSION_VERSION_OFST;

   /* Get data from the burst corruptor */
   data = IORD_ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL(pDev->base);

   pParams->HasPacketCorrupter = (data & ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_IMPLEMENTED_MSK) >> ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_IMPLEMENTED_OFST;

   /* Get data from the design parameters register */
   data = IORD_ALTERA_AVALON_RTP_TX_PARAMS(pDev->base);

   pParams->NumChannels              = (data & ALTERA_AVALON_RTP_TX_PARAMS_NUM_CHANNELS_MSK) >> ALTERA_AVALON_RTP_TX_PARAMS_NUM_CHANNELS_OFST;
   pParams->HasRTPEncapsulator       = (data & ALTERA_AVALON_RTP_TX_PARAMS_ADD_RTP_MSK) >> ALTERA_AVALON_RTP_TX_PARAMS_ADD_RTP_OFST;
   pParams->HasFECGenerator          = (data & ALTERA_AVALON_RTP_TX_PARAMS_HAS_FEC_MSK) >> ALTERA_AVALON_RTP_TX_PARAMS_HAS_FEC_OFST;
   pParams->NumFECFrameBufferEntries = (data & ALTERA_AVALON_RTP_TX_PARAMS_NUM_POINTERS_MSK) >> ALTERA_AVALON_RTP_TX_PARAMS_NUM_POINTERS_OFST;

   /* Calculate the number of queues from the channel count */
   pParams->NumQueues = 2*(pParams->NumChannels) + 2;

   data = IORD_ALTERA_AVALON_RTP_TX_FEC_PARAMS(pDev->base);
   pParams->MaxFECL = (uint8_t)((data & ALTERA_AVALON_RTP_TX_FEC_PARAMS_MAX_L_MSK) >> ALTERA_AVALON_RTP_TX_FEC_PARAMS_MAX_L_OFST);
   pParams->MaxFECD = (uint8_t)((data & ALTERA_AVALON_RTP_TX_FEC_PARAMS_MAX_D_MSK) >> ALTERA_AVALON_RTP_TX_FEC_PARAMS_MAX_D_OFST);
}


/* Enable the RTP transmitter */
void alt_avalon_rtp_tx_enable(alt_avalon_rtp_tx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_TX_ENABLE(pDev->base);
   data |= ALTERA_AVALON_RTP_TX_ENABLE_ENABLE_MSK;
   IOWR_ALTERA_AVALON_RTP_TX_ENABLE(pDev->base, data);
}


/* Disable the RTP transmitter */
void alt_avalon_rtp_tx_disable(alt_avalon_rtp_tx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_TX_ENABLE(pDev->base);
   data &= (~ALTERA_AVALON_RTP_TX_ENABLE_ENABLE_MSK);
   IOWR_ALTERA_AVALON_RTP_TX_ENABLE(pDev->base, data);
}


/* Determine if the RTP transmitter is enabled */
int alt_avalon_rtp_tx_is_enabled(alt_avalon_rtp_tx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_TX_ENABLE(pDev->base);

   if(data & ALTERA_AVALON_RTP_TX_ENABLE_ENABLE_MSK)
   {
      return 1;
   }

   return 0;
}


/* Setup the FEC parameters for a single channel */
void alt_avalon_rtp_tx_set_fec_config(alt_avalon_rtp_tx_dev* pDev, uint32_t Channel, alt_avalon_rtp_tx_fec_settings* pSettings)
{
   uint32_t data;

   /* Set the index of the channel to be updated. */
   data = Channel;
   data = (data << ALTERA_AVALON_RTP_TX_FEC_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_TX_FEC_CHANNEL_SEL_MSK;

   IOWR_ALTERA_AVALON_RTP_TX_FEC_CHANNEL(pDev->base, data);

   /* Calculate the value that will be written to the FEC control register -
   ** We pre-calculate this so that the update happens as soon as possible
   ** after spinning on the updating flag. This maximimises the time
   ** available for the circuit to complete any previous update, and therefore
   ** minimises the risk of us having to stall while the update completes. */
   if(pSettings->EnableFEC)
   {
      data  = ((pSettings->L << ALTERA_AVALON_RTP_TX_FEC_CONTROL_L_OFST) & ALTERA_AVALON_RTP_TX_FEC_CONTROL_L_MSK);
      data |= ((pSettings->D << ALTERA_AVALON_RTP_TX_FEC_CONTROL_D_OFST) & ALTERA_AVALON_RTP_TX_FEC_CONTROL_D_MSK);

      if(pSettings->ColFECOnly)
      {
         data |= ALTERA_AVALON_RTP_TX_FEC_CONTROL_COL_ONLY_MSK;
      }

      if(alt_avalon_rtp_tx_interleave_mode_off == pSettings->InterleaveMode)
      {
         data |= ALTERA_AVALON_RTP_TX_FEC_CONTROL_INTERLEAVE_OFF_MSK;
      }

      if(alt_avalon_rtp_tx_interleave_mode_annex_a == pSettings->InterleaveMode)
      {
         data |= ALTERA_AVALON_RTP_TX_FEC_CONTROL_ANNEX_A_EN_MSK;
      }
   }
   else
   {
      data = 0;
   }

   /* Spin on the updating flag, this will prevent the bus from being
   ** stalled for a long period if the FEC module is unable to service
   ** the update immediately. */
   while(IORD_ALTERA_AVALON_RTP_TX_FEC_CONTROL(pDev->base) & ALTERA_AVALON_RTP_TX_FEC_CONTROL_UPDATING_MSK);

   /* Now that the FEC engine is not busy, we can write the actual FEC update. */
   IOWR_ALTERA_AVALON_RTP_TX_FEC_CONTROL(pDev->base, data);
}


/* Get the FEC parameters for a single channel */
void alt_avalon_rtp_tx_get_fec_config(alt_avalon_rtp_tx_dev* pDev, uint32_t Channel, alt_avalon_rtp_tx_fec_settings* pSettings)
{
   uint32_t data;

   /* Set the index of the channel to be read. */
   data = Channel;
   data = (data << ALTERA_AVALON_RTP_TX_FEC_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_TX_FEC_CHANNEL_SEL_MSK;

   IOWR_ALTERA_AVALON_RTP_TX_FEC_CHANNEL(pDev->base, data);

   /* Spin on the updating flag - this will ensure that we do not read the
   ** previous settings by waiting for any update to complete in hardware
   ** before we perform the read. */
   while(IORD_ALTERA_AVALON_RTP_TX_FEC_CONTROL(pDev->base) & ALTERA_AVALON_RTP_TX_FEC_CONTROL_UPDATING_MSK);

   /* Now that the FEC engine is not busy, we can read the actual FEC settings. */
   data = IORD_ALTERA_AVALON_RTP_TX_FEC_CONTROL(pDev->base);

   pSettings->L = (data & ALTERA_AVALON_RTP_TX_FEC_CONTROL_L_MSK) >> ALTERA_AVALON_RTP_TX_FEC_CONTROL_L_OFST;

   pSettings->D = (data & ALTERA_AVALON_RTP_TX_FEC_CONTROL_D_MSK) >> ALTERA_AVALON_RTP_TX_FEC_CONTROL_D_OFST;

   if(data & ALTERA_AVALON_RTP_TX_FEC_CONTROL_COL_ONLY_MSK)
   {
      pSettings->ColFECOnly = 1;
   }
   else
   {
      pSettings->ColFECOnly = 0;
   }

   if(data & ALTERA_AVALON_RTP_TX_FEC_CONTROL_INTERLEAVE_OFF_MSK)
   {
      pSettings->InterleaveMode = alt_avalon_rtp_tx_interleave_mode_off;
   }
   else
   {
      if(data & ALTERA_AVALON_RTP_TX_FEC_CONTROL_ANNEX_A_EN_MSK)
      {
         pSettings->InterleaveMode = alt_avalon_rtp_tx_interleave_mode_annex_a;
      }
      else
      {
         pSettings->InterleaveMode = alt_avalon_rtp_tx_interleave_mode_annex_b;
      }
   }

   if((0 == pSettings->L) || (0 == pSettings->D))
   {
      pSettings->EnableFEC = 0;
   }
   else
   {
      pSettings->EnableFEC = 1;
   }
}


/*Setup the encapsulation parameters for a single channel */
void alt_avalon_rtp_tx_set_encap_config(alt_avalon_rtp_tx_dev* pDev, uint32_t Channel, alt_avalon_rtp_tx_encap_settings* pSettings)
{
   uint32_t data;
   uint32_t fec_settings;

   AUK_RTP_TX_API_DIAG("[alt_avalon_rtp_tx_set_encap_config] Chan=%d, En=%d, Count=%d, Size=%d\n", Channel, pSettings->RTPEncapEnable, pSettings->RTPEncapNumTSPackets, pSettings->TSPacketSize);

   /* Set the index of the channel to be updated. */
   data = Channel;
   data = (data << ALTERA_AVALON_RTP_TX_FEC_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_TX_FEC_CHANNEL_SEL_MSK;

   IOWR_ALTERA_AVALON_RTP_TX_FEC_CHANNEL(pDev->base, data);

   /* We need to disable FEC correction while we update the encapsulation settings */
   fec_settings = IORD_ALTERA_AVALON_RTP_TX_FEC_CONTROL(pDev->base);

   /* Spin on the updating flag, this will prevent the bus from being
   ** stalled for a long period if the FEC module is unable to service
   ** the update immediately. */
   while(IORD_ALTERA_AVALON_RTP_TX_FEC_CONTROL(pDev->base) & ALTERA_AVALON_RTP_TX_FEC_CONTROL_UPDATING_MSK);

   /* Disable the FEC */
   IOWR_ALTERA_AVALON_RTP_TX_FEC_CONTROL(pDev->base, 0);


   /* Update the RTP encapsulation register */
   data = ((pSettings->RTPEncapNumTSPackets) << ALTERA_AVALON_RTP_TX_RTP_ENCAP_NUM_TS_PACKETS_OFST) & ALTERA_AVALON_RTP_TX_RTP_ENCAP_NUM_TS_PACKETS_MSK;

   if(pSettings->RTPEncapEnable)
   {
      data |= ALTERA_AVALON_RTP_TX_RTP_ENCAP_ENABLE_MSK;
   }

   IOWR_ALTERA_AVALON_RTP_TX_RTP_ENCAP(pDev->base, data);

   /* Update the transport stream packet size register */
   data = pSettings->TSPacketSize;
   data = data << ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE_SIZE_OFST;
   data &= ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE_SIZE_MSK;
   IOWR_ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE(pDev->base, data);

   /* Spin on the updating flag, this will prevent the bus from being
   ** stalled for a long period if the FEC module is unable to service
   ** the update immediately. */
   while(IORD_ALTERA_AVALON_RTP_TX_FEC_CONTROL(pDev->base) & ALTERA_AVALON_RTP_TX_FEC_CONTROL_UPDATING_MSK);

   /* Re-enable FEC correction */
   IOWR_ALTERA_AVALON_RTP_TX_FEC_CONTROL(pDev->base, fec_settings);
}


/* Get the encapsulation parameters for a single channel */
void alt_avalon_rtp_tx_get_encap_config(alt_avalon_rtp_tx_dev* pDev, uint32_t Channel, alt_avalon_rtp_tx_encap_settings* pSettings)
{
   uint32_t data;

   /* Set the index of the channel to be read. */
   data = Channel;
   data = (data << ALTERA_AVALON_RTP_TX_FEC_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_TX_FEC_CHANNEL_SEL_MSK;

   IOWR_ALTERA_AVALON_RTP_TX_FEC_CHANNEL(pDev->base, data);

   /* Read the RTP encapsulation register */
   data = IORD_ALTERA_AVALON_RTP_TX_RTP_ENCAP(pDev->base);
   pSettings->RTPEncapNumTSPackets = (data & ALTERA_AVALON_RTP_TX_RTP_ENCAP_NUM_TS_PACKETS_MSK) >> ALTERA_AVALON_RTP_TX_RTP_ENCAP_NUM_TS_PACKETS_OFST;

   if(data & ALTERA_AVALON_RTP_TX_RTP_ENCAP_ENABLE_MSK)
   {
      pSettings->RTPEncapEnable = 1;
   }
   else
   {
      pSettings->RTPEncapEnable = 0;
   }


   /* Read the transport stream packet size register */
   data = IORD_ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE(pDev->base);
   pSettings->TSPacketSize = (data & ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE_SIZE_MSK) >> ALTERA_AVALON_RTP_TX_TS_PACKET_SIZE_SIZE_OFST;

   AUK_RTP_TX_API_DIAG("[alt_avalon_rtp_tx_get_encap_config] Chan=%d, En=%d, Count=%d, Size=%d\n", Channel, pSettings->RTPEncapEnable, pSettings->RTPEncapNumTSPackets, pSettings->TSPacketSize);
}


/* Configures burst error generation */
void alt_avalon_rtp_tx_set_burst_error_config(alt_avalon_rtp_tx_dev* pDev, alt_avalon_rtp_tx_burst_error_settings* pSettings)
{
   uint32_t data;

   /* Set the index of the channel to corrupt */
   data = (pSettings->ChannelToCorrupt << ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_SEL_MSK;

   IOWR_ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL(pDev->base, data);

   /* Set the number of good packets between bursts */
   data = (pSettings->GapLength << ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS_COUNT_OFST) & ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS_COUNT_MSK;

   IOWR_ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS(pDev->base, data);

   /* Set the length of the burst, and whether or not to loop the burst loss */
   data = (pSettings->BurstLength << ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_NUM_PACKETS_OFST) & ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_NUM_PACKETS_MSK;

   if(!pSettings->Repeat)
   {
      data |= ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_SINGLE_ERROR_MSK;
   }

   IOWR_ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL(pDev->base, data);
}


/* Gets burst error generation settings */
void alt_avalon_rtp_tx_get_burst_error_config(alt_avalon_rtp_tx_dev* pDev, alt_avalon_rtp_tx_burst_error_settings* pSettings)
{
   uint32_t data;

   /* Get the index of the corrupted channel */
   data = IORD_ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL(pDev->base);
   data &= ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_SEL_MSK;
   data = data >> ALTERA_AVALON_RTP_TX_BURST_ERR_CHANNEL_SEL_OFST;
   pSettings->ChannelToCorrupt = data;

   /* Get the number of good packets between bursts */
   data = IORD_ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS(pDev->base);

   pSettings->GapLength = (data & ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS_COUNT_MSK) >> ALTERA_AVALON_RTP_TX_BURST_ERR_GOOD_PKTS_COUNT_OFST;


   /* Get the length of the burst, and whether or not to loop the burst loss */
   data = IORD_ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL(pDev->base);

   pSettings->BurstLength = (data & ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_NUM_PACKETS_MSK) >> ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_NUM_PACKETS_OFST;

   if(data & ALTERA_AVALON_RTP_TX_BURST_ERR_CONTROL_SINGLE_ERROR_MSK)
   {
      pSettings->Repeat = 0;
   }
   else
   {
      pSettings->Repeat = 1;
   }
}


/* Get the minimum level of the free list */
uint32_t alt_avalon_rtp_tx_get_minimum_freelist_level(alt_avalon_rtp_tx_dev* pDev)
{
   uint32_t data;

   data = IORD_ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS(pDev->base);
   data = (data & ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS_MINIMUM_MSK) >> ALTERA_AVALON_RTP_TX_FEC_FREELIST_STATUS_MINIMUM_OFST;

   return data;
}


/* Get the level of a single queue */
uint32_t alt_avalon_rtp_tx_get_single_queue_level(alt_avalon_rtp_tx_dev* pDev, uint32_t Queue)
{
   uint32_t data;

   /* Set the queue index and pause the queue system */
   data = (Queue << ALTERA_AVALON_RTP_TX_QUEUE_SELECT_SEL_OFST) & ALTERA_AVALON_RTP_TX_QUEUE_SELECT_SEL_MSK;

   data |= ALTERA_AVALON_RTP_TX_QUEUE_SELECT_PAUSE_MSK;

   IOWR_ALTERA_AVALON_RTP_TX_QUEUE_SELECT(pDev->base, data);

   /* Read the queue level */
   data = IORD_ALTERA_AVALON_RTP_TX_QUEUE_LEVEL(pDev->base);
   data = (data & ALTERA_AVALON_RTP_TX_QUEUE_LEVEL_LEVEL_MSK) >> ALTERA_AVALON_RTP_TX_QUEUE_LEVEL_LEVEL_OFST;

   /* Release the pause */
   IOWR_ALTERA_AVALON_RTP_TX_QUEUE_SELECT(pDev->base, 0);

   return data;
}


/* Get the level of all queues - pLevels must point to enough memory to store the levels
** for all channels */
void alt_avalon_rtp_tx_get_all_queue_levels(alt_avalon_rtp_tx_dev* pDev, uint32_t* pLevels)
{
   unsigned int QueueIdx;
   uint32_t NumQueues;
   uint32_t data;

   /* Determine the number of queues in the design */
   data = IORD_ALTERA_AVALON_RTP_TX_PARAMS(pDev->base);

   NumQueues = (data & ALTERA_AVALON_RTP_TX_PARAMS_NUM_CHANNELS_MSK) >> ALTERA_AVALON_RTP_TX_PARAMS_NUM_CHANNELS_OFST;
   NumQueues = 2*NumQueues + 2;

   for(QueueIdx=0; QueueIdx<NumQueues; QueueIdx++)
   {
      /* Set the queue index and pause the queue system */
      data = (QueueIdx << ALTERA_AVALON_RTP_TX_QUEUE_SELECT_SEL_OFST) & ALTERA_AVALON_RTP_TX_QUEUE_SELECT_SEL_MSK;

      data |= ALTERA_AVALON_RTP_TX_QUEUE_SELECT_PAUSE_MSK;

      IOWR_ALTERA_AVALON_RTP_TX_QUEUE_SELECT(pDev->base, data);

      /* Read the queue level */
      data = IORD_ALTERA_AVALON_RTP_TX_QUEUE_LEVEL(pDev->base);
      data = (data & ALTERA_AVALON_RTP_TX_QUEUE_LEVEL_LEVEL_MSK) >> ALTERA_AVALON_RTP_TX_QUEUE_LEVEL_LEVEL_OFST;

      /* Write the queue level to the output array */
      pLevels[QueueIdx] = data;
   }

   /* Release the pause */
   IOWR_ALTERA_AVALON_RTP_TX_QUEUE_SELECT(pDev->base, 0);
}

/* Set maximum output queue depth */
void alt_avalon_rtp_tx_set_max_output_depth(alt_avalon_rtp_tx_dev* pDev, uint32_t Depth)
{
   IOWR_ALTERA_AVALON_RTP_TX_MAX_OUTPUT_DEPTH(pDev->base, Depth);
}

/* Get maximum output queue depth */
uint32_t alt_avalon_rtp_tx_get_max_output_depth(alt_avalon_rtp_tx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_TX_MAX_OUTPUT_DEPTH(pDev->base);
   return data;
}

/* Get debug values */
void alt_avalon_rtp_tx_get_debug_values(alt_avalon_rtp_tx_dev* pDev, alt_avalon_rtp_tx_debug_values* pDebug)
{
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 0);
   pDebug->RamWriteCount = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 1);
   pDebug->RamReadCount = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 2);
   pDebug->RamAccessCount = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 3);
   pDebug->RamBusyCount = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 4);
   pDebug->PayloadInReadyCount = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 5);
   pDebug->PayloadInValidCount = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 6);
   pDebug->PayloadOutReadyCount = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 7);
   pDebug->PayloadOutValidCount = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);

   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 9);
   pDebug->QueueLevelsWrong = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 10);
   pDebug->FreelistLevel = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 11);
   pDebug->TransmitQueueLevel = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 12);
   pDebug->FecQueueLevel = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 13);
   pDebug->GenQueueLevel = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 14);
   pDebug->DmaQueueLevel = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base, 15);
   pDebug->DataQueueLevel = IORD_ALTERA_AVALON_RTP_TX_DEBUG(pDev->base);
}


