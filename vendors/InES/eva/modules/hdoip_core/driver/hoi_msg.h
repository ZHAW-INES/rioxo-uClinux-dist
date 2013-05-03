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
#include "../hal/aud/stdaud.h"
#include "../stdinc/event.h"
#include "../driver/fec_tx/fec_tx_struct.h"

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
#define HOI_MSG_ASOREG		        (0x7000002e)
#define HOI_MSG_HDCP_INIT	        (0x7000002f)
#define HOI_MSG_HDCPSTAT            (0x70000030)
#define HOI_MSG_HDCP_ENVIDEO_ETI    (0x70000031)
#define HOI_MSG_HDCP_ENVIDEO_ETO    (0x70000032)
#define HOI_MSG_HDCP_ENAUDIO_ETI    (0x70000033)
#define HOI_MSG_HDCP_ENAUDIO_ETO    (0x70000034)
#define HOI_MSG_HDCP_DISVIDEO_ETI   (0x70000035)
#define HOI_MSG_HDCP_DISVIDEO_ETO   (0x70000036)
#define HOI_MSG_HDCP_DISAUDIO_ETI   (0x70000037)
#define HOI_MSG_HDCP_DISAUDIO_ETO   (0x70000038)
#define HOI_MSG_HDCP_ENAD9889	    (0x70000039)
#define HOI_MSG_HDCP_DISAD9889	    (0x7000003a)
#define HOI_MSG_HPDRESET            (0x7000003b)
#define HOI_MSG_SET_FPS_REDUCTION   (0x7000003c)
#define HOI_MSG_FECSTAT             (0x7000003d)
#define HOI_MSG_AIC23B_ADC          (0x7000003e)
#define HOI_MSG_AIC23B_DAC          (0x7000003f)

#define HOI_MSG_POLL                (0x700000ff)
#define HOI_MSG_HDCP_GET_KEY        (0x70000100)
#define HOI_MSG_HDCP_TIMER_ENABLE   (0x70000101)
#define HOI_MSG_HDCP_TIMER_DISABLE  (0x70000102)
#define HOI_MSG_HDCP_TIMER_LOAD     (0x70000103)
#define HOI_MSG_HDCP_TIMER_GET      (0x70000104)
#define HOI_MSG_HDCP_TIMER_SET      (0x70000105)
#define HOI_MSG_WDG_INIT            (0x70000106)
#define HOI_MSG_WDG_ENABLE          (0x70000107)
#define HOI_MSG_WDG_DISABLE         (0x70000108)
#define HOI_MSG_WDG_SERVICE         (0x70000109)
#define HOI_MSG_SET_TIMING          (0x7000010A)
#define HOI_MSG_LED                 (0x7000010B)
#define HOI_MSG_NEW_AUDIO           (0x7000010C)
#define HOI_MSG_GET_FS              (0x7000010D)
#define HOI_MSG_GET_ANALOG_TIMING   (0x7000010E)
#define HOI_MSG_GET_VID_DEV_ID      (0x7000010F)
#define HOI_MSG_GET_AUD_DEV_ID      (0x70000110)
#define HOI_MSG_GET_RESET_TO_DEFAULT (0x70000111)
#define HOI_MSG_GET_ENCRYPTED_STATUS (0x70000112)
#define HOI_MSG_DEBUG_READ_RAM      (0x70000113)
#define HOI_MSG_CLR_OSD             (0x70000114)
#define HOI_MSG_GET_ACTIVE_RESOLUTION (0x70000115)
#define HOI_MSG_HDCP_BLACK_OUTPUT   (0x70000116)
#define HOI_MSG_OSD_CLR_BORDER      (0x70000117)
#define HOI_MSG_FEC_TX              (0x70000118)

// Driver Bit Mask
#define DRV_NONE                    (0x00000000)
#define DRV_ALL                     (0x0000003F)
#define DRV_ADV9889                 (1<<0)
#define DRV_ADV7441                 (1<<1)
#define DRV_GS2971                  (1<<2)
#define DRV_GS2972                  (1<<3)
#define DRV_AIC23B_DAC              (1<<4)
#define DRV_AIC23B_ADC              (1<<5)

//Card List
#define BDT_ID_HDMI_BOARD           (0x00)
#define BDT_ID_SDI8_BOARD           (0x01)
#define BDT_ID_ANAUDIO_BOARD        (0x01)

// Reset
#define DRV_RST                     (0x1ff)
#define DRV_RST_TMR                 (1<<0)
#define DRV_RST_STSYNC              (1<<1)
#define DRV_RST_VID_OUT             (1<<2)
#define DRV_RST_VID_IN              (1<<3)
#define DRV_RST_AUD_BOARD_OUT       (1<<4)
#define DRV_RST_AUD_EMB_OUT         (1<<5)
#define DRV_RST_AUD_BOARD_IN        (1<<6)
#define DRV_RST_AUD_EMB_IN          (1<<7)
#define DRV_RST_VRP                 (1<<8)

// Timer
#define DRV_TMR_CFG_T1_MASTER       (0x01)
#define DRV_TMR_CFG_T1_SLAVE_0      (0x02)
#define DRV_TMR_CFG_T1_SLAVE_1      (0x03)
#define DRV_TMR_CFG_T2_MASTER       (0x04)
#define DRV_TMR_CFG_T2_SLAVE_0      (0x08)
#define DRV_TMR_CFG_T2_SLAVE_1      (0x0C)
#define DRV_TMR_CFG_T3_MASTER       (0x10)
#define DRV_TMR_CFG_T3_SLAVE_0      (0x20)
#define DRV_TMR_CFG_T3_SLAVE_1      (0x30)
#define DRV_TMR_CFG_T4_MASTER       (0x40)
#define DRV_TMR_CFG_T4_SLAVE_0      (0x80)
#define DRV_TMR_CFG_T4_SLAVE_1      (0xC0)

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
} t_hoi_msg;

#define hoi_msg_switch_init(p, id) hoi_msg_init(p, id, t_hoi_msg)
#define hoi_msg_loop_init(p) hoi_msg_init(p, HOI_MSG_LOOP, t_hoi_msg)
#define hoi_msg_osdon_init(p) hoi_msg_init(p, HOI_MSG_OSDON, t_hoi_msg)
#define hoi_msg_osdoff_init(p) hoi_msg_init(p, HOI_MSG_OSDOFF, t_hoi_msg)
#define hoi_msg_osd_clr_border_init(p) hoi_msg_init(p, HOI_MSG_OSD_CLR_BORDER, t_hoi_msg)
#define hoi_msg_poll_init(p) hoi_msg_init(p, HOI_MSG_POLL, t_hoi_msg)
#define hoi_msg_repair_init(p) hoi_msg_init(p, HOI_MSG_REPAIR, t_hoi_msg)
#define hoi_msg_hpdon_init(p) hoi_msg_init(p, HOI_MSG_HPDON, t_hoi_msg)
#define hoi_msg_hpdoff_init(p) hoi_msg_init(p, HOI_MSG_HPDOFF, t_hoi_msg)

// HDCP messages
#define hoi_msg_hdcp_viden_eti_init(p) hoi_msg_init(p, HOI_MSG_HDCP_ENVIDEO_ETI, t_hoi_msg)
#define hoi_msg_hdcp_viden_eto_init(p) hoi_msg_init(p, HOI_MSG_HDCP_ENVIDEO_ETO, t_hoi_msg)
#define hoi_msg_hdcp_auden_eti_init(p) hoi_msg_init(p, HOI_MSG_HDCP_ENAUDIO_ETI, t_hoi_msg)
#define hoi_msg_hdcp_auden_eto_init(p) hoi_msg_init(p, HOI_MSG_HDCP_ENAUDIO_ETO, t_hoi_msg)
#define hoi_msg_hdcp_viddis_eti_init(p) hoi_msg_init(p, HOI_MSG_HDCP_DISVIDEO_ETI, t_hoi_msg)
#define hoi_msg_hdcp_viddis_eto_init(p) hoi_msg_init(p, HOI_MSG_HDCP_DISVIDEO_ETO, t_hoi_msg)
#define hoi_msg_hdcp_auddis_eti_init(p) hoi_msg_init(p, HOI_MSG_HDCP_DISAUDIO_ETI, t_hoi_msg)
#define hoi_msg_hdcp_auddis_eto_init(p) hoi_msg_init(p, HOI_MSG_HDCP_DISAUDIO_ETO, t_hoi_msg)

#define hoi_msg_hdcp_adv9889dis_init(p) hoi_msg_init(p, HOI_MSG_HDCP_DISAD9889, t_hoi_msg)
#define hoi_msg_hdcp_adv9889en_init(p) hoi_msg_init(p, HOI_MSG_HDCP_ENAD9889, t_hoi_msg)

#define hoi_msg_hpdreset_init(p) hoi_msg_init(p, HOI_MSG_HPDRESET, t_hoi_msg)
#define hoi_msg_hdcp_timer_enable_init(p) hoi_msg_init(p, HOI_MSG_HDCP_TIMER_ENABLE, t_hoi_msg)
#define hoi_msg_hdcp_timer_disable_init(p) hoi_msg_init(p, HOI_MSG_HDCP_TIMER_DISABLE, t_hoi_msg)
#define hoi_msg_hdcp_timer_load_init(p) hoi_msg_init(p, HOI_MSG_HDCP_TIMER_LOAD, t_hoi_msg)

#define hoi_msg_hdcp_black_output_init(p) hoi_msg_init(p, HOI_MSG_HDCP_BLACK_OUTPUT, t_hoi_msg)

#define hoi_msg_wdg_enable_init(p) hoi_msg_init(p, HOI_MSG_WDG_ENABLE, t_hoi_msg)
#define hoi_msg_wdg_disable_init(p) hoi_msg_init(p, HOI_MSG_WDG_DISABLE, t_hoi_msg)
#define hoi_msg_wdg_service_init(p) hoi_msg_init(p, HOI_MSG_WDG_SERVICE, t_hoi_msg)
#define hoi_msg_clr_osd_init(p) hoi_msg_init(p, HOI_MSG_CLR_OSD, t_hoi_msg)

//------------------------------------------------------------------------------
// additional driver load/unload command

typedef struct {
    hoi_msg_extends;
    uint32_t            drivers;        //!< (wr) Driver Bitmap for active drivers
} t_hoi_msg_ldrv;

#define hoi_msg_ldrv_init(p) hoi_msg_init(p, HOI_MSG_LDRV, t_hoi_msg_ldrv)


//------------------------------------------------------------------------------
// configuration

typedef struct {
    hoi_msg_extends;
    void*               vid_tx_buf;     //! < physical address
    size_t              vid_tx_len;     //! < buffer size
    void*               aud_tx_buf;     //! < physical address
    size_t              aud_tx_len;     //! < buffer size
} t_hoi_msg_buf;

#define hoi_msg_buf_init(p) hoi_msg_init(p, HOI_MSG_BUF, t_hoi_msg_buf)

typedef struct {
    hoi_msg_extends;
    uint32_t            udp_port_vid;
    uint32_t            udp_port_aud_emb;
    uint32_t            udp_port_aud_board;
    uint32_t            ip_address_dst;
    uint32_t            ip_address_src_aud;
    uint32_t            ip_address_src_vid;
} t_hoi_msg_eti;

#define hoi_msg_eti_init(p) hoi_msg_init(p, HOI_MSG_ETI, t_hoi_msg_eti)

typedef struct {
    hoi_msg_extends;
    uint32_t            tx_cpu_cnt;
    uint32_t            tx_vid_cnt;
    uint32_t            tx_aud_emb_cnt;
    uint32_t            tx_aud_board_cnt;
    uint32_t            tx_inv_cnt;
    uint32_t            rx_cpu_cnt;
    uint32_t            rx_vid_cnt;
    uint32_t            rx_aud_emb_cnt;
    uint32_t            rx_aud_board_cnt;
    uint32_t            rx_inv_cnt;
    uint32_t            debug;
} t_hoi_msg_ethstat;

#define hoi_msg_ethstat_init(p) hoi_msg_init(p, HOI_MSG_ETHSTAT, t_hoi_msg_ethstat)

typedef struct {
    hoi_msg_extends;
    uint32_t            vid_pkg_cnt;
    uint32_t            vid_mis_cnt;
    uint32_t            vid_fix_cnt;
    uint32_t            vid_fec_en;
    uint32_t            vid_buf;
    uint32_t            aud_pkg_cnt;
    uint32_t            aud_mis_cnt;
    uint32_t            aud_fix_cnt;
    uint32_t            aud_fec_en;
    uint32_t            aud_buf;
    uint32_t            buf_size;
} t_hoi_msg_fecstat;

#define hoi_msg_fecstat_init(p) hoi_msg_init(p, HOI_MSG_FECSTAT, t_hoi_msg_fecstat)

typedef struct {
    hoi_msg_extends;
    uint32_t            vframe_cnt;
    uint32_t            vframe_skip;
    uint32_t            packet_cnt;
    uint32_t            packet_lost;
    uint32_t            packet_in_cnt;
    uint32_t            status;
} t_hoi_msg_vsostat;

#define hoi_msg_vsostat_init(p) hoi_msg_init(p, HOI_MSG_VSOSTAT, t_hoi_msg_vsostat)

typedef struct {
    hoi_msg_extends;
    uint32_t            config[AUD_STREAM_CNT];
    uint32_t            status[AUD_STREAM_CNT];
} t_hoi_msg_asoreg;

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
} t_hoi_msg_viostat;

#define hoi_msg_viostat_init(p) hoi_msg_init(p, HOI_MSG_VIOSTAT, t_hoi_msg_viostat)


//------------------------------------------------------------------------------
// basic commands

typedef struct {
    hoi_msg_extends;
    hdoip_eth_params    eth;        //!< (wr) ethernet parameter
    t_fec_setting       fec;        //!< (wr) fec parameters
    uint32_t            compress;   //!< (wr) use jpeg2000 compressor
    uint32_t            chroma;     //!< (wr) percent of chroma bandwidth
    uint32_t            encrypt;    //!< (wr) encryption key or '0'
    uint32_t            bandwidth;  //!< (wr) bandwidth
    t_video_timing      timing;     //!< (rd) timing of video
    uint32_t            advcnt;     //!< (rd) adv count when compression = true
    uint32_t            enable_traffic_shaping;
} t_hoi_msg_vsi;

#define hoi_msg_vsi_init(p) hoi_msg_init(p, HOI_MSG_VSI, t_hoi_msg_vsi)


typedef struct {
    hoi_msg_extends;
    uint32_t            compress;   //!< [15:0] use jpeg2000 compressor, [31:16]: cfg
    uint32_t            encrypt;    //!< (wr) encryption key or '0'
    t_video_timing      timing;     //!< timing of video
    uint32_t            advcnt;     //!< adv count when compression = true
    uint32_t            delay_ms;
} t_hoi_msg_vso;

#define hoi_msg_vso_init(p) hoi_msg_init(p, HOI_MSG_VSO, t_hoi_msg_vso)


typedef struct {
    hoi_msg_extends;
    t_fec_setting              fec;            //!< (wr) fec parameters
    int unsigned               stream_nr;
    struct hdoip_eth_params    eth;            //!< (wr) ethernet parameter
    struct hdoip_aud_params    aud;            //!< (wr) audio parameter
} t_hoi_msg_asi;

#define hoi_msg_asi_init(p) hoi_msg_init(p, HOI_MSG_ASI, t_hoi_msg_asi)


typedef struct {
    hoi_msg_extends;
    int unsigned                stream_nr;
    uint32_t                    cfg;
    uint16_t                    config;         // config for i2s control: control activ if vtb
    uint32_t                    delay_ms;       //!< (wr) audio delay
    uint32_t                    av_delay;       //!< (wr) audio-video delay
    struct hdoip_aud_params     aud;            //!< (wr) audio parameter
} t_hoi_msg_aso;

#define hoi_msg_aso_init(p) hoi_msg_init(p, HOI_MSG_ASO, t_hoi_msg_aso)

#define AIC23B_DISABLE                  (0)
#define AIC23B_ENABLE                   (1)
#define AIC23B_ADC_SRC_MIC              (2)
#define AIC23B_ADC_SRC_LINE             (3)
#define AIC23B_ADC_MICBOOST_0DB         (0)
#define AIC23B_ADC_MICBOOST_20DB        (20)
typedef struct {
    hoi_msg_extends;
    uint32_t                    enable;         //!< (wr) true to enable codec
    uint32_t                    source;         //!< (wr) input is mic or line in
    int                         line_gain;      //!< (wr) line input gain
    uint32_t                    mic_boost;      //!< (wr) true for mic +20dB gain
    struct hdoip_aud_params     aud;            //!< (wr) audio parameter
} t_hoi_msg_aic23b_adc;

#define hoi_msg_aic23b_adc_init(p) hoi_msg_init(p, HOI_MSG_AIC23B_ADC, t_hoi_msg_aic23b_adc)

typedef struct {
    hoi_msg_extends;
    uint32_t                    enable;         //!< (wr) true to enable codec
    int                         hp_gain;        //!< (wr) headphone gain
    struct hdoip_aud_params     aud;            //!< (wr) audio parameter
} t_hoi_msg_aic23b_dac;

#define hoi_msg_aic23b_dac_init(p) hoi_msg_init(p, HOI_MSG_AIC23B_DAC, t_hoi_msg_aic23b_dac)

typedef struct {
    hoi_msg_extends;
    int unsigned                slave_nr;
    uint32_t                    stime;
} t_hoi_msg_stime;

#define hoi_msg_get_stime_init(p) hoi_msg_init(p, HOI_MSG_GETSTIME, t_hoi_msg_stime)
#define hoi_msg_set_stime_init(p) hoi_msg_init(p, HOI_MSG_SETSTIME, t_hoi_msg_stime)

//------------------------------------------------------------------------------
// read/write single uin32_t command

typedef struct {
    hoi_msg_extends;
    uint32_t            value;
} t_hoi_msg_param;

#define hoi_msg_read_init(p, a) hoi_msg_init(p, a, t_hoi_msg_param)
#define hoi_msg_write_init(p, a) hoi_msg_init(p, a, t_hoi_msg_param)

//------------------------------------------------------------------------------
// read/write command for bandwidth

typedef struct {
    hoi_msg_extends;
    uint32_t            bw;
    uint32_t            chroma;
} t_hoi_msg_bandwidth;

#define hoi_msg_bandwidth_init(p) hoi_msg_init(p, HOI_MSG_BW, t_hoi_msg_bandwidth)

//------------------------------------------------------------------------------
// write command for fec tx parameters

typedef struct {
    hoi_msg_extends;
    t_fec_setting       fec;
} t_hoi_msg_fec_tx;

#define hoi_msg_fec_tx_init(p) hoi_msg_init(p, HOI_MSG_FEC_TX, t_hoi_msg_fec_tx)

//------------------------------------------------------------------------------
// capture/show image command

typedef struct {
    hoi_msg_extends;
    uint32_t            compress;   //!< use jpeg2000 compressor
    void*               buffer;     //!< userspace buffer
    size_t              size;       //!< size of buffer
    t_video_timing      timing;     //!< timing of video
    bool				vtb;		//!< true if transmitter
    uint32_t            advcnt;     //!< adv count when compression = true
} t_hoi_msg_image;

#define hoi_msg_capture_init(p) hoi_msg_init(p, HOI_MSG_CAPTURE, t_hoi_msg_image)
#define hoi_msg_show_init(p) hoi_msg_init(p, HOI_MSG_SHOW, t_hoi_msg_image)


//------------------------------------------------------------------------------
// Get version of the firmware 

typedef struct {
    hoi_msg_extends;
    uint32_t            fpga_date;
    uint32_t            fpga_svn;
    uint32_t            sysid_date;
    uint32_t            sysid_id;
} t_hoi_msg_version;

#define hoi_msg_getversion_init(p) hoi_msg_init(p, HOI_MSG_GETVERSION, t_hoi_msg_version)


//------------------------------------------------------------------------------
// EDID I/O

typedef struct {
    hoi_msg_extends;
    uint32_t            segment;
    uint8_t             edid[256];
} t_hoi_msg_edid;

#define hoi_msg_rdedid_init(p) hoi_msg_init(p, HOI_MSG_RDEDID, t_hoi_msg_edid)
#define hoi_msg_wredid_init(p) hoi_msg_init(p, HOI_MSG_WREDID, t_hoi_msg_edid)


//------------------------------------------------------------------------------
// TAG I/O

typedef struct {
    hoi_msg_extends;
    uint32_t            available;
    uint8_t             tag[256];
} t_hoi_msg_tag;

#define hoi_msg_rdvidtag_init(p) hoi_msg_init(p, HOI_MSG_RDVIDTAG, t_hoi_msg_tag)
#define hoi_msg_wrvidtag_init(p) hoi_msg_init(p, HOI_MSG_WRVIDTAG, t_hoi_msg_tag)
#define hoi_msg_rdaudtag_init(p) hoi_msg_init(p, HOI_MSG_RDAUDTAG, t_hoi_msg_tag)
#define hoi_msg_wraudtag_init(p) hoi_msg_init(p, HOI_MSG_WRAUDTAG, t_hoi_msg_tag)


//------------------------------------------------------------------------------
// video info

typedef struct {
    hoi_msg_extends;
    t_video_timing              timing;         //!< timing of video
    uint32_t                    advcnt;
    uint32_t                    advcnt_old;
    struct hdoip_aud_params     aud_params[AUD_STREAM_CNT];
} t_hoi_msg_info;

#define hoi_msg_info_init(p) hoi_msg_init(p, HOI_MSG_INFO, t_hoi_msg_info)

//------------------------------------------------------------------------------
// hdcp init
typedef struct {
    hoi_msg_extends;
    uint32_t          	key0;
    uint32_t          	key1;
    uint32_t          	key2;
    uint32_t          	key3;
    uint32_t          	riv0;
    uint32_t          	riv1;
} __attribute__ ((__packed__)) t_hoi_msg_hdcp_init;

#define hoi_msg_hdcp_init(p) hoi_msg_init(p, HOI_MSG_HDCP_INIT, t_hoi_msg_hdcp_init)
//------------------------------------------------------------------------------
// Get hdcp status (enabled/disabled)
typedef struct {
    hoi_msg_extends;
    uint32_t            status_eto_video;
    uint32_t            status_eto_audio;
    uint32_t            status_eti_video;
    uint32_t            status_eti_audio;
} __attribute__ ((__packed__)) t_hoi_msg_hdcpstat;

#define hoi_msg_hdcpstat_init(p) hoi_msg_init(p, HOI_MSG_HDCPSTAT, t_hoi_msg_hdcpstat)

// HDCP
typedef struct {
    hoi_msg_extends;
    uint32_t    key[4];
} t_hoi_msg_hdcp_key;

typedef struct {
    hoi_msg_extends;
    uint32_t    config;
    uint32_t    status;
    uint32_t    start_time;
} t_hoi_msg_hdcp_timer;

#define hoi_msg_hdcp_get_key_init(p) hoi_msg_init(p, HOI_MSG_HDCP_GET_KEY, t_hoi_msg_hdcp_key)
#define hoi_msg_hdcp_get_timer_init(p) hoi_msg_init(p, HOI_MSG_HDCP_TIMER_GET, t_hoi_msg_hdcp_timer)
#define hoi_msg_hdcp_set_timer_init(p) hoi_msg_init(p, HOI_MSG_HDCP_TIMER_SET, t_hoi_msg_hdcp_timer)

//------------------------------------------------------------------------------
// Watch dog
typedef struct {
    hoi_msg_extends;
    uint32_t    service_time;
} t_hoi_msg_wdg;

#define hoi_msg_wdg_init_init(p) hoi_msg_init(p, HOI_MSG_WDG_INIT, t_hoi_msg_wdg)

//------------------------------------------------------------------------------
// debug only
typedef struct {
    hoi_msg_extends;
} t_hoi_msg_debug;


#define hoi_msg_debug_init(p) hoi_msg_init(p, HOI_MSG_DEBUG, t_hoi_msg_debug)

//------------------------------------------------------------------------------
// set timing for osd

#define hoi_msg_set_timing_init(p) hoi_msg_init(p, HOI_MSG_SET_TIMING, t_hoi_msg_image)

//------------------------------------------------------------------------------
//

#define hoi_msg_init(p, id, t) _hoi_msg_init((void*)(p), id, sizeof(t))
static inline void _hoi_msg_init(t_hoi_msg* msg, uint32_t id, uint32_t size)
{
    msg->id = id;
    msg->size = size;
}




#endif /* HOI_MSG_H_ */
