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

#ifndef __ALT_AVALON_RTP_RX_API_H__
#define __ALT_AVALON_RTP_RX_API_H__

#include "fec_rx_hal.h"
#include "fec_rx_struct.h"
#include "hoi_msg.h"

/* Used to set message FIFO to all channel mode */
#define ALTERA_AVALON_RTP_RX_ALL_CHANNELS  (0x00010000)

/* Required size for a string buffer that will accept FIFO messages */
#define ALTERA_AVALON_RTP_RX_MAX_MSG_STRING_LENGTH 100

typedef struct
{
   void *base;                             /* Base address of the device */
   uint32_t MsgFIFODepth;                  /* Software message FIFO depth counter */
   uint8_t  UseSoftwareMsgFIFODepth;       /* Flag indicating whether to use the hardware or software depth counter */
} alt_avalon_rtp_rx_dev;

/* Structure containing design parameters for the receiver */
typedef struct
{
   uint32_t     Version;
   uint32_t     NumChannels;
   uint32_t     NumFECPointers;
   uint32_t     FrameBufferEntriesPerChannel;
   uint32_t     MsgFIFOTotalDepth;   /* This is given as a number of alt_avalon_msg_fifo_entry objects */
   bool         HasMsgFIFO;
   bool         HasStatsRegisters;
} alt_avalon_rtp_rx_design_parameters;

/* Structure containing receive settings for a single channel */
typedef struct
{
   bool         Enable;
   bool         RTPBypass;
} alt_avalon_rtp_rx_channel_settings;

/* Structure containing receiver statistic counter values */
typedef struct
{
   bool         Flushed;
   uint32_t     ValidPacketCount;
   uint32_t     MissingPacketCount;
   uint32_t     FixedPacketCount;
   uint32_t     DuplicatePacketCount;
   uint32_t     ReorderedPacketCount;
   uint32_t     OutOfRangePacketCount;
} alt_avalon_rtp_rx_stats;

/* Structure containing latest observations of FEC for a given channel */
typedef struct
{
   bool         ColFECSeen;
   bool         RowFECSeen;
   uint8_t      FECL;
   uint8_t      FECD;
} alt_avalon_rtp_rx_fec_observations;

/* Structure containing message FIFO configuration */
typedef struct
{
   /* Set to ALTERA_AVALON_RTP_RX_ALL_CHANNELS to capture messages from all channels */
   uint32_t     Channel;

   /* Flags indicating when to stop the FIFO */
   bool         StopOnAbnormalEvent;
   bool         StopOnError;

   /* The following defines, from altera_avalon_rtp_rx_regs.h define the masks to be used in this call...
   **
   ** ALTERA_AVALON_RTP_RX_MSG_FIFO_MASK...
   **                      _IN_OK_MSK              - Packet input  - OK
   **                      _IN_RANGE_MSK           - Packet input  - Out of range
   **                      _IN_FLUSH_MSK           - Packet input  - Flush
   **                      _IN_DUPLICATE_MSK       - Packet input  - Duplicate
   **                      _IN_FEC_MSK             - Packet input  - FEC
   **                      _IN_FEC_DISCARD_MSK     - Packet input  - FEC discard
   **                      _IN_DEPTH_UPDATE_MSK    - Packet input  - Depth update
   **                      _IN_REORDER_MSK         - Packet input  - Packet reorder
   **                      _OUT_EMPTY_MSK          - Packet output - Empty
   **                      _IN_NEW_LENGTH_MSK      - Packet input  - New length detected
   **                      _OUT_MISSING_MSK        - Packet output - Missing
   **                      _OUT_OK_MSK             - Packet output - OK
   **                      _OUT_DEPTH_UPDATE_MSK   - Packet output - Depth update
   **                      _FEC_FIX_MSK            - FEC           - Fix
   **                      _FEC_PROBLEM_MSK        - FEC           - Problem
   */
   uint32_t     Mask;
} alt_avalon_rtp_rx_msg_fifo_settings;

/* Enumeration of message FIFO codes */
typedef enum
{
   alt_avalon_rtp_rx_msg_input_req_ok           = 0,
   alt_avalon_rtp_rx_msg_input_req_out_of_range = 1,
   alt_avalon_rtp_rx_msg_input_req_flush        = 2,
   alt_avalon_rtp_rx_msg_input_req_duplicate    = 3,
   alt_avalon_rtp_rx_msg_input_req_fec          = 4,
   alt_avalon_rtp_rx_msg_input_req_fec_discard  = 5,
   alt_avalon_rtp_rx_msg_input_update           = 6,
   alt_avalon_rtp_rx_msg_input_update_reorder   = 7,
   alt_avalon_rtp_rx_msg_output_req_empty       = 8,
   alt_avalon_rtp_rx_msg_input_length           = 9,
   alt_avalon_rtp_rx_msg_output_req_missing     = 10,
   alt_avalon_rtp_rx_msg_output_req_ok          = 11,
   alt_avalon_rtp_rx_msg_output_update          = 13,
   alt_avalon_rtp_rx_msg_fec_update             = 14,
   alt_avalon_rtp_rx_msg_fec_update_problem     = 15
} alt_avalon_rtp_rx_msg;

/* Structure containing a single message FIFO entry */
typedef struct
{
   alt_avalon_rtp_rx_msg   Code;
   uint32_t                 Channel;
   uint32_t                 Time;
   uint32_t                 Data;
} alt_avalon_msg_fifo_entry;

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
   uint32_t InputDmaDebugValue;
   uint32_t OutputDmaDebugValue;
   uint32_t StatusDebugValue;
   uint32_t FecDiscardCount;
   uint32_t FecFreelistEntries;
   uint32_t FecStatusEntries;
   uint32_t FecStackEntries;
   uint32_t FecFreelistEntriesMinimum;
   uint32_t FecFixClocksAverage;
   uint32_t FecFixClocksMaximum;
} alt_avalon_rtp_rx_debug_values;

/* Structure for FEC pointer debug values */
typedef struct
{
   uint32_t LinkedPointer;
} alt_avalon_rtp_rx_fec_pointer_debug_values;

/* Structure for FEC channel debug values */
typedef struct
{
   uint32_t BasePointer;
   uint32_t Entries;
} alt_avalon_rtp_rx_fec_channel_debug_values;

/* statistic for FEC packets*/
void fec_rx_statistics(void *p, t_fec_rx *counter_values, t_hoi_msg_fecstat* msg);

/* Initialize FEC block for video*/
void init_fec_rx_ip_video(void *p, int enable, t_fec_rx *counter_values);

/* Initialize FEC block for audio*/
void init_fec_rx_ip_audio(void *p, int enable, t_fec_rx *counter_values);

/* Get the parameters of a design */
void alt_avalon_rtp_rx_get_parameters(alt_avalon_rtp_rx_dev* pDev, alt_avalon_rtp_rx_design_parameters* pParams);

/* Setup receive parameters for a single channel */
void alt_avalon_rtp_rx_set_channel_config(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel, const alt_avalon_rtp_rx_channel_settings* pSettings);

/* Get the receive parameters for a single channel */
void alt_avalon_rtp_rx_get_channel_config(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel, alt_avalon_rtp_rx_channel_settings* pSettings);

/* Determine the length of the last packet received on a given channel */
uint32_t alt_avalon_rtp_rx_get_last_payload_length(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel);

/* Determine if the given channnel has been flushed */
bool alt_avalon_rtp_rx_get_flushed(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel);

/* Clear the flushed status on a given channel */
void alt_avalon_rtp_rx_clear_flushed(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel);

/* Get the changes in the statistics values since the last call to this function */
void alt_avalon_rtp_rx_get_statistics(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel, alt_avalon_rtp_rx_stats* pStats);

/* Gets the last seen row and column FEC parameters on a given channel. Returns zero
** if no FEC of any kind has been seen on the channel. Otherwise returns one. */
bool alt_avalon_rtp_rx_get_fec_status(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel, alt_avalon_rtp_rx_fec_observations* pLatestFEC);

/* Clears FEC status */
void alt_avalon_rtp_rx_clear_fec_status(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel);

/* Get the current buffer depth count for the given channel */
uint32_t alt_avalon_rtp_rx_get_buffer_depth(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel);

/* Get the duration (in milliseconds) associated with the current buffer
** depth for the given channel */
uint32_t alt_avalon_rtp_rx_get_buffer_duration(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel);

/* Get the base timestamp for the given channel */
uint32_t alt_avalon_rtp_rx_get_base_timestamp(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel);


/* Configures and enables the message FIFO */
void alt_avalon_rtp_rx_enable_msg_fifo(alt_avalon_rtp_rx_dev* pDev, const alt_avalon_rtp_rx_msg_fifo_settings* pSettings);

/* Determines if the message FIFO is enabled */
bool alt_avalon_rtp_rx_msg_fifo_is_enabled(alt_avalon_rtp_rx_dev* pDev);

/* Disables the message FIFO */
void alt_avalon_rtp_rx_disable_msg_fifo(alt_avalon_rtp_rx_dev* pDev);

/* Gets the fill level of the message FIFO  */
uint32_t alt_avalon_rtp_rx_get_msg_fifo_count(alt_avalon_rtp_rx_dev* pDev);

/* Reads upto MaxCount messages from the message FIFO. pMsgs must contain enough storage for
** MaxCount elements. Returns the number of messages actually retreived. This call automatically
** disables the message FIFO. */
uint32_t alt_avalon_rtp_rx_get_msg_fifo_entries(alt_avalon_rtp_rx_dev* pDev, alt_avalon_msg_fifo_entry* pMsgs, uint32_t MaxCount);

/* Get the current message fifo configuration */
void alt_avalon_rtp_rx_get_msg_fifo_settings(alt_avalon_rtp_rx_dev* pDev, alt_avalon_rtp_rx_msg_fifo_settings* pSettings);

/* Get a string representation of a message FIFO entry. The returned string will not
** be longer than ALTERA_AVALON_RTP_RX_MAX_MSG_STRING_LENGTH. If a small implementation of
** this function is required, define ALTERA_AVALON_RTP_RX_SMALL_MSG_STRING_CONVERSION */
void alt_avalon_rtp_rx_get_msg_string(const alt_avalon_msg_fifo_entry* pMsg, char* String);

/* Get flush status - returns 0 if no flush pending */
uint32_t alt_avalon_rtp_rx_get_flush_status(alt_avalon_rtp_rx_dev* pDev);

/* Get ready status - returns 1 when ready for data */
uint32_t alt_avalon_rtp_rx_get_ready_status(alt_avalon_rtp_rx_dev* pDev);

/* Get debug values */
void alt_avalon_rtp_rx_get_debug_values(alt_avalon_rtp_rx_dev* pDev, alt_avalon_rtp_rx_debug_values* pDebug);

/* Set FEC debug pause */
void alt_avalon_rtp_rx_set_fec_debug_pause(alt_avalon_rtp_rx_dev* pDev);

/* Clear FEC debug pause */
void alt_avalon_rtp_rx_clear_fec_debug_pause(alt_avalon_rtp_rx_dev* pDev);

/* Get FEC pointer debug */
void alt_avalon_rtp_rx_get_fec_pointer_debug(alt_avalon_rtp_rx_dev* pDev, uint32_t Pointer, alt_avalon_rtp_rx_fec_pointer_debug_values* pDebug);

/* Get FEC channel debug */
void alt_avalon_rtp_rx_get_fec_channel_debug(alt_avalon_rtp_rx_dev* pDev, uint32_t Channel, alt_avalon_rtp_rx_fec_channel_debug_values* pDebug);

/* Get FEC channel debug */
void alt_avalon_rtp_rx_get_fec_freelist_debug(alt_avalon_rtp_rx_dev* pDev, alt_avalon_rtp_rx_fec_channel_debug_values* pDebug);

/* Disable data reception for test purposes */
void alt_avalon_rtp_rx_disable_data_rx(alt_avalon_rtp_rx_dev* pDev);

/* Enable data reception */
void alt_avalon_rtp_rx_enable_data_rx(alt_avalon_rtp_rx_dev* pDev);

/* Disable FEC reception for test purposes */
void alt_avalon_rtp_rx_disable_fec_rx(alt_avalon_rtp_rx_dev* pDev);

/* Enable FEC reception */
void alt_avalon_rtp_rx_enable_fec_rx(alt_avalon_rtp_rx_dev* pDev);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __ALT_AVALON_RTP_RX_API_H__ */
