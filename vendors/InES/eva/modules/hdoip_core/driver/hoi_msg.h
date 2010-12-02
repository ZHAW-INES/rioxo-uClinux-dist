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

// linux message
#define HDOIP_IOCTL_MAGIC           ('h')
#define HDOIP_IOCPARAM              (_IOWR(HDOIP_IOCTL_MAGIC, 1, t_hoi_msg))

// Message ID
#define HOI_MSG_GET                 (0x08000000)
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
#define HOI_MSG_GETEVENT            (0x7000000d)
#define HOI_MSG_GETSTATE            (0x7000000e)
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

// Driver Bit Mask
#define DRV_NONE                    (0x00000000)
#define DRV_ALL                     (0x00000003)
#define DRV_ADV9889                 (1<<0)
#define DRV_ADV7441                 (1<<1)


//------------------------------------------------------------------------------
// basic message

// Data structure
#define hoi_msg_extends t_hoi_msg _super

// base message struct
typedef struct {
    uint32_t            id;
    uint32_t            size;
} __attribute__ ((__packed__)) t_hoi_msg;

#define hoi_msg_loop_init(p) hoi_msg_init(p, HOI_MSG_LOOP, t_hoi_msg)
#define hoi_msg_osdon_init(p) hoi_msg_init(p, HOI_MSG_OSDON, t_hoi_msg)
#define hoi_msg_osdoff_init(p) hoi_msg_init(p, HOI_MSG_OSDOFF, t_hoi_msg)

//------------------------------------------------------------------------------
// additional driver load/unload command

typedef struct {
    hoi_msg_extends;
    uint32_t            drivers;        //!< (wr) Driver Bitmap for active drivers
    int                 pid;            //!< (wr) PID of calling process
} __attribute__ ((__packed__)) t_hoi_msg_ldrv;

#define hoi_msg_ldrv_init(p) hoi_msg_init(p, HOI_MSG_LDRV, t_hoi_msg_ldrv)


//------------------------------------------------------------------------------
// general configuration

typedef struct {
    hoi_msg_extends;
    uint32_t            set_config;     //!< (wr) Config Bitmap
    uint32_t            clr_config;     //!< (wr) Config Bitmap
    uint32_t            new_config;     //!< (rd) Config Bitmap
} __attribute__ ((__packed__)) t_hoi_msg_cfg;

#define hoi_msg_cfg_init(p) hoi_msg_init(p, HOI_MSG_CFG, t_hoi_msg_cfg)


//------------------------------------------------------------------------------
// setup buffer

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

#define hoi_msg_eti_init(p) hoi_msg_init(p, HOI_MSG_ETI, t_hoi_msg_buf)


//------------------------------------------------------------------------------
// setup buffer

typedef struct {
    hoi_msg_extends;
    hdoip_eth_params    eth;        //!< (wr) ethernet parameter
    uint32_t            compress;   //!< (wr) use jpeg2000 compressor
    uint32_t            bandwidth;  //!< (wr) bandwidth
    t_video_timing      timing;     //!< (rd) timing of video
    uint32_t            advcnt;     //!< (rd) adv count when compression = true
} __attribute__ ((__packed__)) t_hoi_msg_vsi;

#define hoi_msg_vsi_init(p) hoi_msg_init(p, HOI_MSG_VSI, t_hoi_msg_vsi)


typedef struct {
    hoi_msg_extends;
    uint32_t            compress;   //!< use jpeg2000 compressor
    t_video_timing      timing;     //!< timing of video
    uint32_t            advcnt;     //!< adv count when compression = true
    uint32_t            port;
} __attribute__ ((__packed__)) t_hoi_msg_vso;

#define hoi_msg_vso_init(p) hoi_msg_init(p, HOI_MSG_VSO, t_hoi_msg_vso)

//------------------------------------------------------------------------------
// setup/read video format for input/output

typedef struct {
    hoi_msg_extends;
    t_video_format      value;
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


//------------------------------------------------------------------------------
// EDID I/O

typedef struct {
    hoi_msg_extends;
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
    t_video_timing      timing;     //!< timing of video
} __attribute__ ((__packed__)) t_hoi_msg_info;

#define hoi_msg_info(p) hoi_msg_init(p, HOI_MSG_INFO, t_hoi_msg_info)


//------------------------------------------------------------------------------
//

#define hoi_msg_init(p, id, t) _hoi_msg_init((void*)(p), id, sizeof(t))
static inline void _hoi_msg_init(t_hoi_msg* msg, uint32_t id, uint32_t size)
{
    msg->id = id;
    msg->size = size;
}


#endif /* HOI_MSG_H_ */
