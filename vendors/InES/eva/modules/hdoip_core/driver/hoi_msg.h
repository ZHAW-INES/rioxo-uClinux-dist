/*
 * hoi_msg.h
 *
 * used both in driver and user space
 *
 *  Created on: 08.10.2010
 *      Author: alda
 */

#ifndef HOI_MSG_H_
#define HOI_MSG_H_

#include <linux/types.h>
#include "../hal/vid/stdvid.h"
#include "../hal/eth/stdeth.h"
#include "../stdinc/event.h"

// linux message
#define HDOIP_IOCTL_MAGIC           ('h')
#define HDOIP_IOCPARAM              (_IOWR(HDOIP_IOCTL_MAGIC, 1, t_hoi_msg))

// Message ID
#define HOI_MSG_DRIVER              (0x70000000)
#define HOI_MSG_LDRV                (0x70000001)
#define HOI_MSG_CAPTURE             (0x70000002)
#define HOI_MSG_SHOW                (0x70000003)
#define HOI_MSG_IFMT                (0x70000004)
#define HOI_MSG_OFMT                (0x70000005)
#define HOI_MSG_PFMT                (0x70000006)
#define HOI_MSG_RDVIDTAG            (0x70000009)
#define HOI_MSG_RDAUDTAG            (0x7000000a)
#define HOI_MSG_WRVIDTAG            (0x7000000b)
#define HOI_MSG_WRAUDTAG            (0x7000000c)
#define HOI_MSG_CFG                 (0x7000000f)
#define HOI_MSG_RDCFG               (0x70000010)
#define HOI_MSG_INFO                (0x70000011)
#define HOI_MSG_RDEDID              (0x70000012)
#define HOI_MSG_WREDID              (0x70000013)
#define HOI_MSG_LOOP                (0x70000014)
#define HOI_MSG_OSDON               (0x70000015)
#define HOI_MSG_OSDOFF              (0x70000016)
#define HOI_MSG_BUF                 (0x70000017)
#define HOI_MSG_VSI                 (0x70000018)
#define HOI_MSG_VSO                 (0x70000019)
#define HOI_MSG_ETI                 (0x7000001a)
#define HOI_MSG_SETMTIME            (0x7000001b)
#define HOI_MSG_GETMTIME            (0x7000001c)
#define HOI_MSG_OFF                 (0x7000001d)
#define HOI_MSG_SETSTIME            (0x7000001e)
#define HOI_MSG_GETSTIME            (0x7000001f)
#define HOI_MSG_TIMER               (0x70000020)
#define HOI_MSG_ETHSTAT             (0x70000021)
#define HOI_MSG_VSOSTAT             (0x70000022)
#define HOI_MSG_BW                  (0x70000023)
#define HOI_MSG_DEBUG               (0x70000024)
#define HOI_MSG_ASI                 (0x70000025)
#define HOI_MSG_ASO                 (0x70000026)
#define HOI_MSG_STSYNC              (0x70000027)
#define HOI_MSG_SYNCDELAY           (0x70000028)
#define HOI_MSG_VIOSTAT             (0x70000029)
#define HOI_MSG_GETVERSION          (0x7000002a)
#define HOI_MSG_REPAIR              (0x7000002b)
#define HOI_MSG_HPDON               (0x7000002c)
#define HOI_MSG_HPDOFF              (0x7000002d)
#define HOI_MSG_ASOREG				(0x7000002e)
#define HOI_MSG_POLL                (0x700000ff)

// Driver Bit Mask
#define DRV_NONE                    (0x00000000)
#define DRV_ALL                     (0x00000003)
#define DRV_ADV9889                 (1<<0)
#define DRV_ADV7441                 (1<<1)

// Reset
#define DRV_RST                     (0x7f)
#define DRV_RST_TMR                 (1<<0)
#define DRV_RST_STSYNC              (1<<1)
#define DRV_RST_VID_OUT             (1<<2)
#define DRV_RST_VID_IN              (1<<3)
#define DRV_RST_AUD_OUT             (1<<4)
#define DRV_RST_AUD_IN              (1<<5)
#define DRV_RST_VRP                 (1<<6)

// Timer
#define DRV_TMR_IN                  (0x00000001)
#define DRV_TMR_OUT                 (0x00000002)
#define DRV_TMR_ETH                 (0x00000004)

// Config
#define DRV_CODEC                   (0x000000ff)
#define DRV_CODEC_NONE              (0x00000000)
#define DRV_CODEC_JP2               (0x00000001)
#define DRV_STREAM_SYNC             (0x00010000)

// Streams
#define DRV_ST_VIDEO                (1<<0)
#define DRV_ST_AUDIO                (1<<1)


//------------------------------------------------------------------------------
// basic message

// Data structure
#define hoi_msg_extends t_hoi_msg _super

// base message struct
typedef struct {
    uint32_t            id;
    uint32_t            size;
} __attribute__ ((__packed__)) t_hoi_msg;

#define hoi_msg_switch_init(p, id) hoi_msg_init(p, id, t_hoi_msg)
#define hoi_msg_loop_init(p) hoi_msg_init(p, HOI_MSG_LOOP, t_hoi_msg)
#define hoi_msg_osdon_init(p) hoi_msg_init(p, HOI_MSG_OSDON, t_hoi_msg)
#define hoi_msg_osdoff_init(p) hoi_msg_init(p, HOI_MSG_OSDOFF, t_hoi_msg)
#define hoi_msg_poll_init(p) hoi_msg_init(p, HOI_MSG_POLL, t_hoi_msg)
#define hoi_msg_repair_init(p) hoi_msg_init(p, HOI_MSG_REPAIR, t_hoi_msg)
#define hoi_msg_hpdon_init(p) hoi_msg_init(p, HOI_MSG_HPDON, t_hoi_msg)
#define hoi_msg_hpdoff_init(p) hoi_msg_init(p, HOI_MSG_HPDOFF, t_hoi_msg)


//------------------------------------------------------------------------------
// additional driver load/unload command

typedef struct {
    hoi_msg_extends;
    uint32_t            drivers;        //!< (wr) Driver Bitmap for active drivers
} __attribute__ ((__packed__)) t_hoi_msg_ldrv;

#define hoi_msg_ldrv_init(p) hoi_msg_init(p, HOI_MSG_LDRV, t_hoi_msg_ldrv)


//------------------------------------------------------------------------------
// configuration

typedef struct {
    hoi_msg_extends;
    void*               vid_tx_buf;     //! < physical address
    size_t              vid_tx_len;     //! < buffer size
    void*               vid_rx_buf;     //! < physical address
    size_t              vid_rx_len;     //! < buffer size
    void*               aud_tx_buf;     //! < physical address
    size_t              aud_tx_len;     //! < buffer size
    void*               aud_rx_buf;     //! < physical address
    size_t              aud_rx_len;     //! < buffer size
} __attribute__ ((__packed__)) t_hoi_msg_buf;

#define hoi_msg_buf_init(p) hoi_msg_init(p, HOI_MSG_BUF, t_hoi_msg_buf)

typedef struct {
    hoi_msg_extends;
    uint32_t            udp_port_vid;
    uint32_t            udp_port_aud;
    uint32_t            ip_address_dst;
    uint32_t            ip_address_src;
} __attribute__ ((__packed__)) t_hoi_msg_eti;

#define hoi_msg_eti_init(p) hoi_msg_init(p, HOI_MSG_ETI, t_hoi_msg_eti)

typedef struct {
    hoi_msg_extends;
    uint32_t            tx_cpu_cnt;
    uint32_t            tx_vid_cnt;
    uint32_t            tx_aud_cnt;
    uint32_t            tx_inv_cnt;
    uint32_t            rx_cpu_cnt;
    uint32_t            rx_vid_cnt;
    uint32_t            rx_aud_cnt;
    uint32_t            rx_inv_cnt;
    uint32_t            debug;
} __attribute__ ((__packed__)) t_hoi_msg_ethstat;

#define hoi_msg_ethstat_init(p) hoi_msg_init(p, HOI_MSG_ETHSTAT, t_hoi_msg_ethstat)

typedef struct {
    hoi_msg_extends;
    uint32_t            vframe_cnt;
    uint32_t            vframe_skip;
    uint32_t            packet_cnt;
    uint32_t            packet_lost;
    uint32_t            packet_in_cnt;
    uint32_t            status;
} __attribute__ ((__packed__)) t_hoi_msg_vsostat;

#define hoi_msg_vsostat_init(p) hoi_msg_init(p, HOI_MSG_VSOSTAT, t_hoi_msg_vsostat)

typedef struct {
    hoi_msg_extends;
    uint32_t			config;
    uint32_t            status;
    uint32_t			start;
    uint32_t			stop;
    uint32_t			read;
    uint32_t			write;
} __attribute__ ((__packed__)) t_hoi_msg_asoreg;

#define hoi_msg_asoreg_init(p) hoi_msg_init(p, HOI_MSG_ASOREG, t_hoi_msg_asoreg)

typedef struct {
    hoi_msg_extends;
    uint32_t            fin;
    uint32_t            fout;
    int32_t             tgerr;
    int32_t             pllerr;
    uint32_t            vid_in;
    uint32_t            vid_out;
    uint32_t            st_in;
    uint32_t            st_out;
    uint32_t            fvsync;
} __attribute__ ((__packed__)) t_hoi_msg_viostat;

#define hoi_msg_viostat_init(p) hoi_msg_init(p, HOI_MSG_VIOSTAT, t_hoi_msg_viostat)


//------------------------------------------------------------------------------
// basic commands

typedef struct {
    hoi_msg_extends;
    hdoip_eth_params    eth;        //!< (wr) ethernet parameter
    uint32_t            compress;   //!< (wr) use jpeg2000 compressor
    uint32_t            encrypt;    //!< (wr) encryption key or '0'
    uint32_t            bandwidth;  //!< (wr) bandwidth
    t_video_timing      timing;     //!< (rd) timing of video
    uint32_t            advcnt;     //!< (rd) adv count when compression = true
} __attribute__ ((__packed__)) t_hoi_msg_vsi;

#define hoi_msg_vsi_init(p) hoi_msg_init(p, HOI_MSG_VSI, t_hoi_msg_vsi)


typedef struct {
    hoi_msg_extends;
    uint32_t            compress;   //!< [15:0] use jpeg2000 compressor, [31:16]: cfg
    uint32_t            encrypt;    //!< (wr) encryption key or '0'
    t_video_timing      timing;     //!< timing of video
    uint32_t            advcnt;     //!< adv count when compression = true
    uint32_t            delay_ms;
} __attribute__ ((__packed__)) t_hoi_msg_vso;

#define hoi_msg_vso_init(p) hoi_msg_init(p, HOI_MSG_VSO, t_hoi_msg_vso)


typedef struct {
    hoi_msg_extends;
    uint32_t            cfg;
    hdoip_eth_params    eth;            //!< (wr) ethernet parameter
    uint32_t            fs;             //!< (wr) sampel frequency in Hz
    uint32_t            width;          //!< (wr) sampel bit width
    uint32_t            channel_cnt;    //!< (wr) number of used audio channels
    uint8_t             channel[16];    //!< (wr) mapping of input/transport channel
} __attribute__ ((__packed__)) t_hoi_msg_asi;

#define hoi_msg_asi_init(p) hoi_msg_init(p, HOI_MSG_ASI, t_hoi_msg_asi)


typedef struct {
    hoi_msg_extends;
    uint32_t            cfg;
    uint32_t            delay_ms;       //!< (wr) audio delay
    uint32_t            fs;             //!< (wr) sampel frequency in Hz
    uint32_t            fs_tol;         //!< (wr) frequency tolerance (fs +/- fs_tol)
    uint32_t            width;          //!< (wr) sampel bit width
    uint32_t            channel_cnt;    //!< (wr) number of used audio channels
    uint8_t             channel[16];    //!< (wr) mapping of transport/output channel
} __attribute__ ((__packed__)) t_hoi_msg_aso;

#define hoi_msg_aso_init(p) hoi_msg_init(p, HOI_MSG_ASO, t_hoi_msg_aso)


//------------------------------------------------------------------------------
// read/write single uin32_t command

typedef struct {
    hoi_msg_extends;
    uint32_t            value;
} __attribute__ ((__packed__)) t_hoi_msg_param;

#define hoi_msg_read_init(p, a) hoi_msg_init(p, a, t_hoi_msg_param)
#define hoi_msg_write_init(p, a) hoi_msg_init(p, a, t_hoi_msg_param)


//------------------------------------------------------------------------------
// capture/show image command

typedef struct {
    hoi_msg_extends;
    uint32_t            compress;   //!< use jpeg2000 compressor
    void*               buffer;     //!< userspace buffer
    size_t              size;       //!< size of buffer
    t_video_timing      timing;     //!< timing of video
    uint32_t            advcnt;     //!< adv count when compression = true
} __attribute__ ((__packed__)) t_hoi_msg_image;

#define hoi_msg_capture_init(p) hoi_msg_init(p, HOI_MSG_CAPTURE, t_hoi_msg_image)
#define hoi_msg_show_init(p) hoi_msg_init(p, HOI_MSG_SHOW, t_hoi_msg_image)
#define hoi_msg_debug_init(p) hoi_msg_init(p, HOI_MSG_DEBUG, t_hoi_msg_image)


//------------------------------------------------------------------------------
// Get version of the firmware 

typedef struct {
    hoi_msg_extends;
    uint32_t            fpga_date;
    uint32_t            fpga_svn;
    uint32_t            sysid_date;
    uint32_t            sysid_id;
} __attribute__ ((__packed__)) t_hoi_msg_version;

#define hoi_msg_getversion_init(p) hoi_msg_init(p, HOI_MSG_GETVERSION, t_hoi_msg_version)


//------------------------------------------------------------------------------
// EDID I/O

typedef struct {
    hoi_msg_extends;
    uint32_t            segment;
    uint8_t             edid[256];
} __attribute__ ((__packed__)) t_hoi_msg_edid;

#define hoi_msg_rdedid_init(p) hoi_msg_init(p, HOI_MSG_RDEDID, t_hoi_msg_edid)
#define hoi_msg_wredid_init(p) hoi_msg_init(p, HOI_MSG_WREDID, t_hoi_msg_edid)


//------------------------------------------------------------------------------
// TAG I/O

typedef struct {
    hoi_msg_extends;
    uint32_t            available;
    uint8_t             tag[256];
} __attribute__ ((__packed__)) t_hoi_msg_tag;

#define hoi_msg_rdvidtag_init(p) hoi_msg_init(p, HOI_MSG_RDVIDTAG, t_hoi_msg_tag)
#define hoi_msg_wrvidtag_init(p) hoi_msg_init(p, HOI_MSG_WRVIDTAG, t_hoi_msg_tag)
#define hoi_msg_rdaudtag_init(p) hoi_msg_init(p, HOI_MSG_RDAUDTAG, t_hoi_msg_tag)
#define hoi_msg_wraudtag_init(p) hoi_msg_init(p, HOI_MSG_WRAUDTAG, t_hoi_msg_tag)


//------------------------------------------------------------------------------
// video info

typedef struct {
    hoi_msg_extends;
    t_video_timing      timing;         //!< timing of video
    uint32_t            advcnt;
    uint32_t            audio_fs[2];    //!< audio sampling frequency [HZ] (0=off)
    uint32_t            audio_width[2]; //!< audio sample width [bit]
    uint32_t            audio_cnt[2];   //!< number of active channels
    uint32_t            audio_map;      //!< bitmap for active audio channels
} __attribute__ ((__packed__)) t_hoi_msg_info;

#define hoi_msg_info_init(p) hoi_msg_init(p, HOI_MSG_INFO, t_hoi_msg_info)


//------------------------------------------------------------------------------
//

#define hoi_msg_init(p, id, t) _hoi_msg_init((void*)(p), id, sizeof(t))
static inline void _hoi_msg_init(t_hoi_msg* msg, uint32_t id, uint32_t size)
{
    msg->id = id;
    msg->size = size;
}


#endif /* HOI_MSG_H_ */
