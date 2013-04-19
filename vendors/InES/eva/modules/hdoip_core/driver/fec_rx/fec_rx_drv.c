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

#include <fec_rx_hal.h>
#include <fec_rx_drv.h>

#ifndef ALTERA_AVALON_RTP_RX_SMALL_MSG_STRING_CONVERSION
#endif

// Switch these macros between AUK_RTP_RX_DIAG and AUK_RTP_RX_NO_DIAG
// to turn diagnostic messages on and off
#define AUK_RTP_RX_API_DIAG(...)  printk(__VA_ARGS__)



// Private function for determining the number of bits required to store a given number of items
uint32_t alt_avalon_rtp_rx_clogb2(uint32_t Value)
{
   uint32_t i;
   uint32_t Result = 1;

   for(i=31; i>0; i--)
   {
      if((1<<i) >= Value)
      {
         Result = i;
      }
   }

   return Result;
}


// Get the parameters of a design
void alt_avalon_rtp_rx_get_parameters(alt_avalon_rtp_rx_dev* pDev, alt_avalon_rtp_rx_design_parameters* pParams)
{
   uint32_t data;


   // Get the version number
   data = IORD_ALTERA_AVALON_RTP_RX_VERSION(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_VERSION_VERSION_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_VERSION_VERSION_OFST;
   pParams->Version = data;


   // Get the channel count
   data = IORD_ALTERA_AVALON_RTP_RX_NUM_CHANNELS(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_NUM_CHANNELS_COUNT_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_NUM_CHANNELS_COUNT_OFST;
   pParams->NumChannels = data;


   // Get the number of FEC pointers
   data = IORD_ALTERA_AVALON_RTP_RX_FEC_POINTERS(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_FEC_POINTERS_COUNT_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_FEC_POINTERS_COUNT_OFST;
   pParams->NumFECPointers = data;


   // Get the number of frame buffer entries for each channel
   data = IORD_ALTERA_AVALON_RTP_RX_FB_ENTRIES(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_FB_ENTRIES_COUNT_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_FB_ENTRIES_COUNT_OFST;
   pParams->FrameBufferEntriesPerChannel = data;


   // TBD - Remainder are not done in hardware yet
   pParams->MsgFIFOTotalDepth = 512;   // This is given as a number of alt_avalon_msg_fifo_entry objects
   pParams->HasMsgFIFO = 1;
   pParams->HasStatsRegisters = 1;
}


// Setup receive parameters for a single channel
void alt_avalon_rtp_rx_set_channel_config(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel, const alt_avalon_rtp_rx_channel_settings* pSettings)
{
   uint32_t data;

   // Set the channel
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, (Channel << ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK);

   // Change the settings
   data = 0;

   if(pSettings->Enable)
   {
      data |= ALTERA_AVALON_RTP_RX_CONFIG_CHANNEL_ENABLE_MSK;
   }

   if(pSettings->RTPBypass)
   {
      data |= ALTERA_AVALON_RTP_RX_CONFIG_RTP_BYPASS_MSK;
   }

   IOWR_ALTERA_AVALON_RTP_RX_CONFIG(pDev->base, data);
}


// Get the receive parameters for a single channel
void alt_avalon_rtp_rx_get_channel_config(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel, alt_avalon_rtp_rx_channel_settings* pSettings)
{
   uint32_t data;

   // Set the channel
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, (Channel << ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK);

   // Read the settings
   data = IORD_ALTERA_AVALON_RTP_RX_CONFIG(pDev->base);

   if(0 != (data & ALTERA_AVALON_RTP_RX_CONFIG_CHANNEL_ENABLE_MSK))
   {
      pSettings->Enable = 1;
   }
   else
   {
      pSettings->Enable = 0;
   }

   if(0 != (data & ALTERA_AVALON_RTP_RX_CONFIG_RTP_BYPASS_MSK))
   {
      pSettings->RTPBypass = 1;
   }
   else
   {
      pSettings->RTPBypass = 0;
   }
}


// Determine the length of the last packet received on a given channel
uint32_t alt_avalon_rtp_rx_get_last_payload_length(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel)
{
   uint32_t data;

   // Set the channel
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, (Channel << ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK);

   // Read the length value
   data = IORD_ALTERA_AVALON_RTP_RX_CONFIG(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_CONFIG_LAST_PAYLOAD_LENGTH_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_CONFIG_LAST_PAYLOAD_LENGTH_OFST;

   return data;
}


// Determine if the given channnel has been flushed
bool alt_avalon_rtp_rx_get_flushed(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel)
{
   uint32_t data;

   // Set the channel
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, (Channel << ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK);

   // Read the flush value
   data = IORD_ALTERA_AVALON_RTP_RX_CONFIG(pDev->base);

   if(0 != (data & ALTERA_AVALON_RTP_RX_CONFIG_HAS_FLUSHED_MSK))
   {
      return 1;
   }

   return 0;
}


// Clear the flushed status on a given channel
void alt_avalon_rtp_rx_clear_flushed(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel)
{
   uint32_t data;

   // Set the channel
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, (Channel << ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK);

   // Get the current value of the config register
   data = IORD_ALTERA_AVALON_RTP_RX_CONFIG(pDev->base);

   // Set the flush bit
   data |= ALTERA_AVALON_RTP_RX_CONFIG_HAS_FLUSHED_MSK;

   // Write the data back to clear the flushed status
   IOWR_ALTERA_AVALON_RTP_RX_CONFIG(pDev->base, data);
}


// Get the changes in the statistics values since the last call to this function.
// Note that if the channel has flushed, this will be indicated in the valid
// packet count. The other counters will be cleared by the hardware at the time
// of the count and then start running again. For this reason, this driver will
// read from all of the counters when a flush is detected to ensure they are all
// cleared at the same time.
void alt_avalon_rtp_rx_get_statistics(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel, alt_avalon_rtp_rx_stats* pStats)
{
   uint32_t data;

   // Set the channel
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, (Channel << ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK);

   // Get the valid packet count register value
   data = IORD_ALTERA_AVALON_RTP_RX_VALID_PACKETS(pDev->base);

   // Determine if the channel has been flushed
   if(ALTERA_AVALON_RTP_RX_VALID_PACKETS_CHANNEL_FLUSHED == data)
   {
      // Set the return values
      pStats->Flushed = 1;
      pStats->ValidPacketCount      = 0;
      pStats->MissingPacketCount    = 0;
      pStats->FixedPacketCount      = 0;
      pStats->DuplicatePacketCount  = 0;
      pStats->ReorderedPacketCount  = 0;
      pStats->OutOfRangePacketCount = 0;

      // Read from each of the statistics counters to clear their values
      IORD_ALTERA_AVALON_RTP_RX_MISSING_PACKETS(pDev->base);
      IORD_ALTERA_AVALON_RTP_RX_FIXED_PACKETS(pDev->base);
      IORD_ALTERA_AVALON_RTP_RX_DUPLICATE_PACKETS(pDev->base);
      IORD_ALTERA_AVALON_RTP_RX_REORDERED_PACKETS(pDev->base);
      IORD_ALTERA_AVALON_RTP_RX_OUT_OF_RANGE_PACKETS(pDev->base);
   }
   else
   {
      pStats->Flushed = 0;

      // Read out the stats
      data &= ALTERA_AVALON_RTP_RX_VALID_PACKETS_COUNT_MSK;
      data = data >> ALTERA_AVALON_RTP_RX_VALID_PACKETS_COUNT_OFST;
      pStats->ValidPacketCount = data;

      data = IORD_ALTERA_AVALON_RTP_RX_MISSING_PACKETS(pDev->base);
      data &= ALTERA_AVALON_RTP_RX_MISSING_PACKETS_COUNT_MSK;
      data = data >> ALTERA_AVALON_RTP_RX_MISSING_PACKETS_COUNT_OFST;
      pStats->MissingPacketCount = data;

      data = IORD_ALTERA_AVALON_RTP_RX_FIXED_PACKETS(pDev->base);
      data &= ALTERA_AVALON_RTP_RX_FIXED_PACKETS_COUNT_MSK;
      data = data >> ALTERA_AVALON_RTP_RX_FIXED_PACKETS_COUNT_OFST;
      pStats->FixedPacketCount = data;

      data = IORD_ALTERA_AVALON_RTP_RX_DUPLICATE_PACKETS(pDev->base);
      data &= ALTERA_AVALON_RTP_RX_DUPLICATE_PACKETS_COUNT_MSK;
      data = data >> ALTERA_AVALON_RTP_RX_DUPLICATE_PACKETS_COUNT_OFST;
      pStats->DuplicatePacketCount = data;

      data = IORD_ALTERA_AVALON_RTP_RX_REORDERED_PACKETS(pDev->base);
      data &= ALTERA_AVALON_RTP_RX_REORDERED_PACKETS_COUNT_MSK;
      data = data >> ALTERA_AVALON_RTP_RX_REORDERED_PACKETS_COUNT_OFST;
      pStats->ReorderedPacketCount = data;

      data = IORD_ALTERA_AVALON_RTP_RX_OUT_OF_RANGE_PACKETS(pDev->base);
      data &= ALTERA_AVALON_RTP_RX_OUT_OF_RANGE_PACKETS_COUNT_MSK;
      data = data >> ALTERA_AVALON_RTP_RX_OUT_OF_RANGE_PACKETS_COUNT_OFST;
      pStats->OutOfRangePacketCount = data;
   }
}


// Gets the last seen row and column FEC parameters on a given channel. Returns zero
// if no FEC of any kind has been seen on the channel. Otherwise returns one.
bool alt_avalon_rtp_rx_get_fec_status(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel, alt_avalon_rtp_rx_fec_observations* pLatestFEC)
{
   uint32_t data;

   // Set the channel
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, (Channel << ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK);

   // Read the FEC status register
   data = IORD_ALTERA_AVALON_RTP_RX_FEC_MODE(pDev->base);

   // Disect the register contents into the FEC structure
   if(0 != (data & ALTERA_AVALON_RTP_RX_FEC_MODE_COL_SEEN_MSK))
   {
      pLatestFEC->ColFECSeen = 1;
   }
   else
   {
      pLatestFEC->ColFECSeen = 0;
   }

   if(0 != (data & ALTERA_AVALON_RTP_RX_FEC_MODE_ROW_SEEN_MSK))
   {
      pLatestFEC->RowFECSeen = 1;
   }
   else
   {
      pLatestFEC->RowFECSeen = 0;
   }

   pLatestFEC->FECL          = (uint8_t)((data & ALTERA_AVALON_RTP_RX_FEC_MODE_L_MSK) >> ALTERA_AVALON_RTP_RX_FEC_MODE_L_OFST);

   pLatestFEC->FECD          =  (uint8_t)((data & ALTERA_AVALON_RTP_RX_FEC_MODE_D_MSK) >> ALTERA_AVALON_RTP_RX_FEC_MODE_D_OFST);

   // Calculate the return value
   if(pLatestFEC->ColFECSeen || pLatestFEC->RowFECSeen)
   {
      return 1;
   }

   return 0;
}


// Clears FEC status
void alt_avalon_rtp_rx_clear_fec_status(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel)
{
   // TBD - Not yet implemented in hardware
}


// Get the current buffer depth count for the given channel
uint32_t alt_avalon_rtp_rx_get_buffer_depth(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel)
{
   uint32_t data;

   // Set the channel
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, (Channel << ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK);

   // Get the depth
   data = IORD_ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM_COUNT_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM_COUNT_OFST;

   return data;
}


// Get the duration (in milliseconds) associated with the current buffer
// depth for the given channel
uint32_t alt_avalon_rtp_rx_get_buffer_duration(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel)
{
   uint32_t data = 0;

   // Set the channel
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, (Channel << ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK);

   // Read depth first as this latches all the status values
   data = IORD_ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM(pDev->base);

   // Get the depth based on the timesatmp
   data = IORD_ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_TSTAMP(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_TSTAMP_COUNT_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_TSTAMP_COUNT_OFST;

   // We convert the timestamp based duration into milliseconds by dividing
   // by the RTP timestamp period in kHz
   return data / 90;
}

// Get the base timestamp for the given channel
uint32_t alt_avalon_rtp_rx_get_base_timestamp(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel)
{
   uint32_t data;

   // Set the channel
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, (Channel << ALTERA_AVALON_RTP_RX_CHANNEL_SEL_OFST) & ALTERA_AVALON_RTP_RX_CHANNEL_SEL_MSK);

   // Read depth first as this latches all the status values
   data = IORD_ALTERA_AVALON_RTP_RX_BUFFER_DEPTH_SEQ_NUM(pDev->base);

   // Get the timestamp
   data = IORD_ALTERA_AVALON_RTP_RX_BUFFER_TIMESTAMP(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_BUFFER_TIMESTAMP_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_BUFFER_TIMESTAMP_OFST;

   return data;
}


// Configures and enables the message FIFO
void alt_avalon_rtp_rx_enable_msg_fifo(alt_avalon_rtp_rx_dev* pDev, const alt_avalon_rtp_rx_msg_fifo_settings* pSettings)
{
   uint32_t data;

   // We need the message FIFO to be disabled when we read it, so this call requires a lock
   // to ensure that we don't enable the FIFO midway through reading it.

   // Print a debug message
   AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_enable_msg_fifo] Ch %d, St %d,%d, Mk 0x%08x\n", pSettings->Channel, pSettings->StopOnAbnormalEvent, pSettings->StopOnError, pSettings->Mask);

   // Set the message mask register
   IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK(pDev->base, pSettings->Mask);

   // Apply the settings and enable the FIFO
   data = ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_ENABLE_MSK;

   if(pSettings->StopOnAbnormalEvent)
   {
      data |= ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_STOP_ON_ABNORMAL_MSK;
   }

   if(pSettings->StopOnError)
   {
      data |= ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_STOP_ON_ERROR_MSK;
   }

   if(ALTERA_AVALON_RTP_RX_ALL_CHANNELS != pSettings->Channel)
   {
      data |= ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_SINGLE_CHANNEL_MSK;

      data |= ((pSettings->Channel << ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_CHANNEL_SELECT_OFST) & ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_CHANNEL_SELECT_MSK);
   }

   IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL(pDev->base, data);

   // When the FIFO is first enabled, the depth is stored in hardware.
   // See note about this in the alt_avalon_rtp_rx_get_msg_fifo_count
   // function for more information.
   pDev->UseSoftwareMsgFIFODepth = 0;
}


// Determines if the message FIFO is enabled
bool alt_avalon_rtp_rx_msg_fifo_is_enabled(alt_avalon_rtp_rx_dev* pDev)
{
   uint32_t data;

   // Finding if the FIFO is enabled is atomic, so no lock is required

   data = IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL(pDev->base);

   if(0 != (data & ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_ENABLE_MSK))
   {
      return 1;
   }

   return 0;
}


// Disables the message FIFO
void alt_avalon_rtp_rx_disable_msg_fifo(alt_avalon_rtp_rx_dev* pDev)
{
   // It doesn't matter if someone disables the FIFO while it is being read, so no lock is required.
   IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL(pDev->base, 0);
}


// Gets the fill level of the message FIFO
uint32_t alt_avalon_rtp_rx_get_msg_fifo_count(alt_avalon_rtp_rx_dev* pDev)
{
   uint32_t data;

   // Note that the hardware message fifo depth counter only increments.
   // If data is read out of the hardware, this is not indicated in the hardware value.
   // Also, when the FIFO is enabled, it is cleared, so extra data can never
   // appear in the FIFO after it has been disabled.
   //
   // We keep a software counter to allow a decrementing count value to be presented
   // to the software. The driver switches to this representation when data is first
   // read from the hardware. We use a flag in the device handle to determine which
   // value to return here.

   // Use the flag in the handle to determine which depth representation to use.
   if(pDev->UseSoftwareMsgFIFODepth)
   {
      data = pDev->MsgFIFODepth;
   }
   else
   {
      data = IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES(pDev->base);
      data &= ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES_COUNT_MSK;
      data = data >> ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES_COUNT_OFST;
   }

   return data;
}

// Reads upto MaxCount messages from the message FIFO. pMsgs must contain enough storage for
// MaxCount elements. Returns the number of messages actually retreived. This call automatically
// disables the message FIFO.
uint32_t alt_avalon_rtp_rx_get_msg_fifo_entries(alt_avalon_rtp_rx_dev* pDev, alt_avalon_msg_fifo_entry* pMsgs, uint32_t MaxCount)
{
   uint32_t data;
   uint32_t count;
   uint32_t msg_base;
   uint32_t total_size;
   uint32_t idx;
   uint32_t ls_address;
   uint32_t channel_width;


   // Disable the message FIFO
   alt_avalon_rtp_rx_disable_msg_fifo(pDev);


   // Get the total number of messages in the FIFO, irrespective of what may have been
   // read before.
   data = IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES_COUNT_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_MSG_FIFO_ENTRIES_COUNT_OFST;
   total_size = data;


   // Get the quantity of data remaining in the FIFO. If this is not already stored in
   // software, then do so now. See note in the alt_avalon_rtp_rx_get_msg_fifo_count
   // function for more information.
   if(!(pDev->UseSoftwareMsgFIFODepth))
   {
      pDev->UseSoftwareMsgFIFODepth = 1;
      pDev->MsgFIFODepth = total_size;
   }

   AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_get_msg_fifo_entries] MaxCount=%d, MsgFIFODepth=%d\n", MaxCount, pDev->MsgFIFODepth);


   // Exit if there is no data to read
   if(0 == (pDev->MsgFIFODepth))
   {
      return 0;
   }


   // Determine the number of bits required to store the channel ID.
   // This is used to disect the messages
   data = IORD_ALTERA_AVALON_RTP_RX_NUM_CHANNELS(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_NUM_CHANNELS_COUNT_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_NUM_CHANNELS_COUNT_OFST;
   channel_width = alt_avalon_rtp_rx_clogb2(data);


   // Get the address of the base FIFO entry
   data = IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_BASE(pDev->base);
   data &= ALTERA_AVALON_RTP_RX_MSG_FIFO_BASE_ADDRESS_MSK;
   data = data >> ALTERA_AVALON_RTP_RX_MSG_FIFO_BASE_ADDRESS_OFST;
   msg_base = data;

   AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_get_msg_fifo_entries] Reading from base 0x%08x\n", msg_base);

   // Loop to extract data from the FIFO
   count = 0;
   while((count < MaxCount) && (pDev->MsgFIFODepth > 0))
   {
      // Determine the address of the head FIFO entry (least significant word)
      idx = total_size - (pDev->MsgFIFODepth);
      ls_address = (msg_base + idx)*2;

      // Set the FIFO read address to the head (most significant word)
      data = ls_address + 1;
      data = data << ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS_ADDRESS_OFST;
      data &= ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS_ADDRESS_MSK;
      IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS(pDev->base, data);

      // Read back the FIFO head (most significant word)
      data = IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA(pDev->base);
      data &= ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA_VALUE_MSK;
      data = data >> ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA_VALUE_OFST;

      AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_get_msg_fifo_entries] Read 0x%08x from address 0x%08x\n", data, ls_address+1);


      // Disect the head value into fields - it would be nice to put this code
      // in macros within the _regs.h file, but the variability of the format
      // with channel count made the macros very difficult to understand,
      // so I opted for the much simpler 'magic numbers' implementation.
      pMsgs[count].Code = (alt_avalon_rtp_rx_msg)(data & 0xf);
      AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_get_msg_fifo_entries] Code: %d\n", pMsgs[count].Code);
      data = data >> 4;

      pMsgs[count].Channel = data & ((1<<channel_width)-1);
      AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_get_msg_fifo_entries] Channel: %d\n", pMsgs[count].Channel);
      data = data >> channel_width;

      pMsgs[count].Time = data;
      AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_get_msg_fifo_entries] Time: 0x%08x\n", pMsgs[count].Time);

      // Set the FIFO read address to the head (least significant word)
      data = ls_address;
      data = data << ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS_ADDRESS_OFST;
      data &= ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS_ADDRESS_MSK;
      IOWR_ALTERA_AVALON_RTP_RX_MSG_FIFO_ACCESS(pDev->base, data);

      // Read back the FIFO head (least significant word)
      data = IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA(pDev->base);
      data &= ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA_VALUE_MSK;
      data = data >> ALTERA_AVALON_RTP_RX_MSG_FIFO_DATA_VALUE_OFST;

      AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_get_msg_fifo_entries] Read 0x%08x from address 0x%08x\n", data, ls_address);


      // Extract the head value into the output structure
      pMsgs[count].Data = data;
      AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_get_msg_fifo_entries] Data: 0x%08x\n", pMsgs[count].Data);


      // Update the FIFO depth and count ready for the next iteration
      (pDev->MsgFIFODepth)--;
      count++;
   } // while(count < MaxCount)

   return count;
}

// Get the current message fifo configuration
void alt_avalon_rtp_rx_get_msg_fifo_settings(alt_avalon_rtp_rx_dev* pDev, alt_avalon_rtp_rx_msg_fifo_settings* pSettings)
{
   uint32_t data;

   // Get the message mask
   pSettings->Mask = IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK(pDev->base);

   // Get the rest of the settings
   data = IORD_ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL(pDev->base);

   if(data & ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_STOP_ON_ABNORMAL_MSK)
   {
      pSettings->StopOnAbnormalEvent = 1;
   }
   else
   {
      pSettings->StopOnAbnormalEvent = 0;
   }

   if(data & ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_STOP_ON_ERROR_MSK)
   {
      pSettings->StopOnError = 1;
   }
   else
   {
      pSettings->StopOnError = 0;
   }

   if(data & ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_SINGLE_CHANNEL_MSK)
   {
      data &= ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_CHANNEL_SELECT_MSK;
      data = data >> ALTERA_AVALON_RTP_RX_MSG_FIFO_CTRL_CHANNEL_SELECT_OFST;
      pSettings->Channel = data;
   }
   else
   {
      pSettings->Channel = ALTERA_AVALON_RTP_RX_ALL_CHANNELS;
   }
}


// Get a string representation of a message FIFO entry. The returned string will not
// be longer than ALTERA_AVALON_RTP_RX_MAX_MSG_STRING_LENGTH. If a small implementation of
// this function is required, define ALTERA_AVALON_RTP_RX_SMALL_MSG_STRING_CONVERSION
void alt_avalon_rtp_rx_get_msg_string(const alt_avalon_msg_fifo_entry* pMsg, char* String)
{
   #ifdef ALTERA_AVALON_RTP_RX_SMALL_MSG_STRING_CONVERSION
      sprintf( String,
               "Ch %3lu - Time 0x%08lx - Msg %2d - Data 0x%08lx",
               pMsg->Channel,
               pMsg->Time,
               pMsg->Code,
               pMsg->Data);
   #else
      sprintf(String, "Ch %3lu - Time 0x%08lx - ", pMsg->Channel, pMsg->Time);
      String += strlen(String);

      switch(pMsg->Code)
      {
         case alt_avalon_rtp_rx_msg_input_req_ok:
            sprintf( String,
                     "Input req OK,           SeqNum 0x%04lx, Base 0x%04lx",
                     pMsg->Data&0xFFFF,
                     (pMsg->Data)>>16);
         break;

         case alt_avalon_rtp_rx_msg_input_req_out_of_range:
            sprintf( String,
                     "Input req OUT OF RANGE, SeqNum 0x%04lx, Base 0x%04lx",
                     pMsg->Data&0xFFFF,
                     (pMsg->Data)>>16);
         break;

         case alt_avalon_rtp_rx_msg_input_req_flush:
            sprintf( String,
                     "Input req FLUSH,        SeqNum 0x%04lx, Base 0x%04lx",
                     pMsg->Data&0xFFFF,
                     (pMsg->Data)>>16);
         break;

         case alt_avalon_rtp_rx_msg_input_req_duplicate:
            sprintf( String,
                     "Input req DUPLICATE,    SeqNum 0x%04lx, Base 0x%04lx",
                     pMsg->Data&0xFFFF,
                     (pMsg->Data)>>16);
         break;

         case alt_avalon_rtp_rx_msg_input_req_fec:
            sprintf( String,
                     "%s FEC - SNBase 0x%04lx, NA 0x%02lx, Offset 0x%02lx",
                     ((pMsg->Data >> 26) & 0x1) ? "Row" : "Col",
                     pMsg->Data&0xFFFF,
                     ((pMsg->Data)>>16)&0x1F,
                     ((pMsg->Data)>>21)&0x1F);
         break;

         case alt_avalon_rtp_rx_msg_input_req_fec_discard:
            sprintf( String,
                     "FEC discarded");
         break;

         case alt_avalon_rtp_rx_msg_input_update:
            sprintf( String,
                     "Input update,  Depth %3lu,              Base 0x%04lx",
                     pMsg->Data&0xFFFF,
                     (pMsg->Data)>>16);
         break;

         case alt_avalon_rtp_rx_msg_input_update_reorder:
            sprintf( String,
                     "Input update, SeqNum reordered,        Base 0x%04lx",
                     (pMsg->Data)>>16);
         break;

         case alt_avalon_rtp_rx_msg_output_req_empty:
            sprintf( String,
                     "Output request from empty buffer");
         break;

         case alt_avalon_rtp_rx_msg_input_length:
            sprintf( String,
                     "New input length %lu",
                     (pMsg->Data) & 0xFFFF);
         break;

         case alt_avalon_rtp_rx_msg_output_req_missing:
            sprintf( String,
                     "Missing                 SeqNum 0x%04lx",
                     (pMsg->Data) & 0xFFFF);
         break;

         case alt_avalon_rtp_rx_msg_output_req_ok:
            sprintf( String,
                     "Output req OK,          SeqNum 0x%04lx",
                     pMsg->Data&0xFFFF);
         break;

         case alt_avalon_rtp_rx_msg_output_update:
            sprintf( String,
                     "Output update, Depth %3lu,              Base 0x%04lx",
                     pMsg->Data&0xFFFF,
                     (pMsg->Data)>>16);
         break;

         case alt_avalon_rtp_rx_msg_fec_update:
            sprintf( String,
                     "FEC fix for missing SeqNum 0x%04lx",
                     pMsg->Data&0xFFFF);
         break;

         case alt_avalon_rtp_rx_msg_fec_update_problem:
            sprintf( String,
                     "FEC problem (code %lu), SeqNum 0x%04lx",
                     ((pMsg->Data)>>16) & 0x3,
                     pMsg->Data&0xFFFF);
         break;

         default:
            sprintf( String,
                     "Message code %d, Data 0x%08lx",
                     pMsg->Code,
                     pMsg->Data);
         break;
      }
   #endif
}

// Get flush status - returns 0 if no flush pending
uint32_t alt_avalon_rtp_rx_get_flush_status(alt_avalon_rtp_rx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_RX_FLUSH_STATUS_REG(pDev->base);
   return data;
}

// Get ready status - returns 1 when ready for data
uint32_t alt_avalon_rtp_rx_get_ready_status(alt_avalon_rtp_rx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_RX_READY_STATUS_REG(pDev->base);
   return data;
}

// Get debug values
void alt_avalon_rtp_rx_get_debug_values(alt_avalon_rtp_rx_dev* pDev, alt_avalon_rtp_rx_debug_values* pDebug)
{
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 0);
   pDebug->RamWriteCount = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 1);
   pDebug->RamReadCount = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 2);
   pDebug->RamAccessCount = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 3);
   pDebug->RamBusyCount = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 4);
   pDebug->PayloadInReadyCount = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 5);
   pDebug->PayloadInValidCount = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 6);
   pDebug->PayloadOutReadyCount = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 7);
   pDebug->PayloadOutValidCount = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 8);
   pDebug->InputDmaDebugValue = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 9);
   pDebug->OutputDmaDebugValue = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base, 10);
   pDebug->StatusDebugValue = IORD_ALTERA_AVALON_RTP_RX_DEBUG(pDev->base);

   IOWR_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base, 0);
   pDebug->FecDiscardCount = IORD_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base, 1);
   pDebug->FecFreelistEntries = IORD_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base, 2);
   pDebug->FecFreelistEntriesMinimum = IORD_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base, 3);
   pDebug->FecFixClocksAverage = IORD_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base, 4);
   pDebug->FecFixClocksMaximum = IORD_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base, 5);
   pDebug->FecStatusEntries = IORD_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base);
   IOWR_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base, 6);
   pDebug->FecStackEntries = IORD_ALTERA_AVALON_RTP_RX_FEC_STATS(pDev->base);
}

// Set FEC debug pause
void alt_avalon_rtp_rx_set_fec_debug_pause(alt_avalon_rtp_rx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base);
   data |= ALTERA_AVALON_RTP_RX_EXTENDED_MODE_FEC_PAUSE_MSK;
   IOWR_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base, data);
}

// Clear FEC debug pause
void alt_avalon_rtp_rx_clear_fec_debug_pause(alt_avalon_rtp_rx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base);
   data &= ~ALTERA_AVALON_RTP_RX_EXTENDED_MODE_FEC_PAUSE_MSK;
   IOWR_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base, data);
}

// Get FEC pointer debug
void alt_avalon_rtp_rx_get_fec_pointer_debug(alt_avalon_rtp_rx_dev* pDev, uint32_t Pointer, alt_avalon_rtp_rx_fec_pointer_debug_values* pDebug)
{
   uint32_t data;
   IOWR_ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG(pDev->base, Pointer);
   data = IORD_ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG(pDev->base);

   pDebug->LinkedPointer = (data & ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG_MSK) >> ALTERA_AVALON_RTP_RX_FEC_POINTER_DEBUG_OFST;
}

// Get FEC channel debug
void alt_avalon_rtp_rx_get_fec_channel_debug(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel, alt_avalon_rtp_rx_fec_channel_debug_values* pDebug)
{
   uint32_t data;
   IOWR_ALTERA_AVALON_RTP_RX_CHANNEL(pDev->base, Channel);
   IOWR_ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG(pDev->base, 0);
   data = IORD_ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG(pDev->base);
   pDebug->BasePointer = (data & ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_BASE_MSK) >> ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_BASE_OFST;
   pDebug->Entries = (data & ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_ENTRIES_MSK) >> ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_ENTRIES_OFST;
}

// Get FEC freelist debug
void alt_avalon_rtp_rx_get_fec_freelist_debug(alt_avalon_rtp_rx_dev* pDev, alt_avalon_rtp_rx_fec_channel_debug_values* pDebug)
{
   uint32_t data;
   IOWR_ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG(pDev->base, 1);
   data = IORD_ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG(pDev->base);
   AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_get_fec_freelist_debug] Read 0x%08x as debug value\n", data);
   pDebug->BasePointer = (data & ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_BASE_MSK) >> ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_BASE_OFST;
   pDebug->Entries = (data & ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_ENTRIES_MSK) >> ALTERA_AVALON_RTP_RX_FEC_CHANNEL_DEBUG_ENTRIES_OFST;
   AUK_RTP_RX_API_DIAG("[alt_avalon_rtp_rx_get_fec_freelist_debug] #entries is %d, base pointer is %d.\n", pDebug->Entries, pDebug->BasePointer);
}

// Disable data reception for test purposes
void alt_avalon_rtp_rx_disable_data_rx(alt_avalon_rtp_rx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base);
   data |= ALTERA_AVALON_RTP_RX_EXTENDED_MODE_DISCARD_DATA_MSK;
   IOWR_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base, data);
}
// Enable data reception
void alt_avalon_rtp_rx_enable_data_rx(alt_avalon_rtp_rx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base);
   data &= ~ALTERA_AVALON_RTP_RX_EXTENDED_MODE_DISCARD_DATA_MSK;
   IOWR_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base, data);
}
// Disable FEC reception for test purposes
void alt_avalon_rtp_rx_disable_fec_rx(alt_avalon_rtp_rx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base);
   data |= ALTERA_AVALON_RTP_RX_EXTENDED_MODE_DISCARD_FEC_MSK;
   IOWR_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base, data);
}
// Enable FEC reception
void alt_avalon_rtp_rx_enable_fec_rx(alt_avalon_rtp_rx_dev* pDev)
{
   uint32_t data;
   data = IORD_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base);
   data &= ~ALTERA_AVALON_RTP_RX_EXTENDED_MODE_DISCARD_FEC_MSK;
   IOWR_ALTERA_AVALON_RTP_RX_EXTENDED_MODE(pDev->base, data);
}

void init_fec_rx_ip_video(void *p, int enable, t_fec_rx *counter_values)
{
    alt_avalon_rtp_rx_dev dev;
    alt_avalon_rtp_rx_channel_settings settings;
    alt_avalon_rtp_rx_stats stats;

    dev.base = p;
    dev.MsgFIFODepth = 0;
    dev.UseSoftwareMsgFIFODepth = 0;

    settings.Enable = enable;
    settings.RTPBypass = 0;

    // video
    alt_avalon_rtp_rx_set_channel_config(&dev, 0, &settings);

    if (enable == 1) {
        // reset statistic counters
        counter_values->valid_packets_vid = 0;
        counter_values->missing_packets_vid = 0;
        counter_values->fixed_packets_vid = 0;
        // statistic counter start on first call of this routine
        alt_avalon_rtp_rx_get_statistics(&dev, 0, &stats);
    }
}

void init_fec_rx_ip_audio_emb(void *p, int enable, t_fec_rx *counter_values)
{
    alt_avalon_rtp_rx_dev dev;
    alt_avalon_rtp_rx_channel_settings settings;
    alt_avalon_rtp_rx_stats stats;

    dev.base = p;
    dev.MsgFIFODepth = 0;
    dev.UseSoftwareMsgFIFODepth = 0;

    settings.Enable = enable;
    settings.RTPBypass = 0;

    alt_avalon_rtp_rx_set_channel_config(&dev, 1, &settings);

    if (enable == 1) {
        // reset statistic counters
        counter_values->valid_packets_aud = 0;
        counter_values->missing_packets_aud = 0;
        counter_values->fixed_packets_aud = 0;
        // statistic counter start on first call of this routine
        alt_avalon_rtp_rx_get_statistics(&dev, 1, &stats);
    }
}

void init_fec_rx_ip_audio_board(void *p, int enable)
{
    alt_avalon_rtp_rx_dev dev;
    alt_avalon_rtp_rx_channel_settings settings;

    dev.base = p;
    dev.MsgFIFODepth = 0;
    dev.UseSoftwareMsgFIFODepth = 0;

    settings.Enable = enable;
    settings.RTPBypass = 0;

    alt_avalon_rtp_rx_set_channel_config(&dev, 2, &settings);
}

void fec_rx_statistics(void *p, t_fec_rx *counter_values, t_hoi_msg_fecstat* msg)
{
    alt_avalon_rtp_rx_dev               dev;
    alt_avalon_rtp_rx_stats             stats_vid;
    alt_avalon_rtp_rx_stats             stats_aud;
    alt_avalon_rtp_rx_design_parameters parameters;
    alt_avalon_rtp_rx_channel_settings  settings_vid;
    alt_avalon_rtp_rx_channel_settings  settings_aud;

    dev.base = p;
    dev.MsgFIFODepth = 0;
    dev.UseSoftwareMsgFIFODepth = 0;

    // get values
    alt_avalon_rtp_rx_get_parameters(&dev, &parameters);
    alt_avalon_rtp_rx_get_statistics(&dev, 0, &stats_vid);
    alt_avalon_rtp_rx_get_statistics(&dev, 1, &stats_aud);
    alt_avalon_rtp_rx_get_channel_config(&dev, 0, &settings_vid);
    alt_avalon_rtp_rx_get_channel_config(&dev, 1, &settings_aud);

    // statistic registers are set to zero after read -> build sum of all values read
    counter_values->valid_packets_vid += stats_vid.ValidPacketCount;
    counter_values->missing_packets_vid += stats_vid.MissingPacketCount;
    counter_values->fixed_packets_vid += stats_vid.FixedPacketCount;
    counter_values->valid_packets_aud += stats_aud.ValidPacketCount;
    counter_values->missing_packets_aud += stats_aud.MissingPacketCount;
    counter_values->fixed_packets_aud += stats_aud.FixedPacketCount;

    msg->vid_pkg_cnt    = counter_values->valid_packets_vid;
    msg->vid_mis_cnt    = counter_values->missing_packets_vid;
    msg->vid_fix_cnt    = counter_values->fixed_packets_vid;
    msg->vid_fec_en     = settings_vid.Enable;
    msg->vid_buf        = alt_avalon_rtp_rx_get_buffer_depth(&dev, 0);
    msg->aud_pkg_cnt    = counter_values->valid_packets_aud;
    msg->aud_mis_cnt    = counter_values->missing_packets_aud;
    msg->aud_fix_cnt    = counter_values->fixed_packets_aud;
    msg->aud_fec_en     = settings_aud.Enable;
    msg->aud_buf        = alt_avalon_rtp_rx_get_buffer_depth(&dev, 1);
    msg->buf_size       = parameters.FrameBufferEntriesPerChannel;
}
