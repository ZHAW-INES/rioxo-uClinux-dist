/*
 * hoi_msg.c
 *
 *  Created on: 08.10.2010
 *      Author: alda
 */

#include "std.h"
#include "hoi_drv.h"

#include "adv7441a_drv_edid.h" // TODO no edid for init
#include "adv7441a_drv.h"
#include "gs2971_drv.h"
#include "sdi_edid.h"
#include "fec_tx_drv.h"
#include "fec_rx_drv.h"

// demo workaround:
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/seq_file.h>
#include <asm/system.h>
#include <asm/traps.h>

#include "wdg_hal.h"
#include "hdcp_hal.h"
#include "sdi_edid.h"
#include "../hal/aud/stdaud.h"
//------------------------------------------------------------------------------
// Load driver

int hoi_drv_msg_ldrv(t_hoi* handle, t_hoi_msg_ldrv* msg)
{
    uint32_t rmdrv = ~msg->drivers & handle->drivers;
    uint32_t lddrv = msg->drivers & ~handle->drivers;

    // remove driver...
    if (rmdrv & DRV_ADV9889) {
        // TODO
    }
    if (rmdrv & DRV_ADV7441) {
        // TODO
    }
    if (rmdrv & DRV_GS2971) {
        // TODO
    }
    if (rmdrv & DRV_GS2972) {
        // TODO
    }

    // install driver...
    if (lddrv & DRV_ADV9889) {
        adv9889_drv_init(&handle->adv9889, &handle->i2c_tx, &handle->vio);
    }
    if (lddrv & DRV_ADV7441) {
        adv7441a_drv_init(&handle->adv7441a, &handle->i2c_rx, &handle->vio, (char*)sdi_edid, handle->p_video_mux);
    }
    if (lddrv & DRV_GS2971) {
        gs2971_driver_init(&handle->gs2971, handle->p_spi_rx, &handle->i2c_tag_vid, handle->p_video_mux);
    }
    if (lddrv & DRV_GS2972) {
        gs2972_driver_init(&handle->gs2972, handle->p_spi_tx, &handle->i2c_tag_vid, handle->p_video_mux);
    }

    handle->drivers = msg->drivers & DRV_ALL;
    return SUCCESS;
}


int hoi_drv_msg_buf(t_hoi* handle, t_hoi_msg_buf* msg)
{
    // ringbuffer
    eto_drv_set_vid_buf(&handle->eto, msg->vid_tx_buf, msg->vid_tx_len - MAX_FRAME_LENGTH);
    eto_drv_set_aud_buf(&handle->eto, msg->aud_tx_buf, msg->aud_tx_len - MAX_FRAME_LENGTH);

    // fec memory offset
    fec_rx_set_address_offset(handle->p_fec_memory_interface, msg->vid_tx_buf);

    // set descriptors and burst size in fec block
    fec_drv_set_descriptors(handle->p_fec_tx, msg->vid_tx_buf, msg->aud_tx_buf, msg->vid_tx_len, msg->aud_tx_len, 0x08);

    vso_drv_flush_buf(&handle->vso);
    asi_drv_flush_buf(&handle->asi);
    aso_drv_flush_buf(&handle->aso);

    return SUCCESS;
}
//--------------------------------- HDCP --------------------------------------
// enable hdcp eti video encryption
int hoi_drv_msg_hdcp_video_en_eti(t_hoi* handle){
    eti_drv_aes_vid_en(&handle->eti);
    return SUCCESS;
}
// enable hdcp eto video encryption
int hoi_drv_msg_hdcp_video_en_eto(t_hoi* handle){
    eto_drv_aes_vid_en(&handle->eto);
    return SUCCESS;
}
// enable hdcp eti audio encryption
int hoi_drv_msg_hdcp_audio_en_eti(t_hoi* handle){
    eti_drv_aes_aud_en(&handle->eti);
    return SUCCESS;
}
// enable hdcp eto audio encryption
int hoi_drv_msg_hdcp_audio_en_eto(t_hoi* handle){
    eto_drv_aes_aud_en(&handle->eto);
    return SUCCESS;
}
// disable hdcp eti video encryption
int hoi_drv_msg_hdcp_video_dis_eti(t_hoi* handle){
    eti_set_config_video_enc_dis(&handle->eti);
    return SUCCESS;
}
// disable hdcp eto video encryption
int hoi_drv_msg_hdcp_video_dis_eto(t_hoi* handle){
    eto_set_config_video_enc_dis(&handle->eto);
    return SUCCESS;
}
// disable hdcp eti audio encryption
int hoi_drv_msg_hdcp_audio_dis_eti(t_hoi* handle){
    eti_set_config_audio_enc_dis(&handle->eti);
    return SUCCESS;
}
// disable hdcp eto audio encryption
int hoi_drv_msg_hdcp_audio_dis_eto(t_hoi* handle){
    eto_set_config_audio_enc_dis(&handle->eto);
    return SUCCESS;
}
// enable hdcp encryption in AD9889 (hdmi transmitter IC)
int hoi_drv_msg_hdcp_ADV9889_en(t_hoi* handle){
    if (handle->drivers & DRV_ADV9889) {
        adv9889_drv_hdcp_on(&handle->adv9889);
    }
    return SUCCESS;
}

// disable hdcp encryption in AD9889 (hdmi transmitter IC)
int hoi_drv_msg_hdcp_ADV9889_dis(t_hoi* handle){
    if (handle->drivers & DRV_ADV9889) {
        adv9889_drv_hdcp_off(&handle->adv9889);
    }
    return SUCCESS;
}


// hdcp init (write keys to hardware)
int hoi_drv_msg_hdcp_init(t_hoi* handle, t_hoi_msg_hdcp_init* msg)
{
   /* REPORT(INFO, "hoi_drv_msg_aes_init() -> init aes (key0 : %08x)", msg->key0);
    REPORT(INFO, "hoi_drv_msg_aes_init() -> init aes (key1 : %08x)", msg->key1);
    REPORT(INFO, "hoi_drv_msg_aes_init() -> init aes (key2 : %08x)", msg->key2);
    REPORT(INFO, "hoi_drv_msg_aes_init() -> init aes (key3 : %08x)", msg->key3);
    REPORT(INFO, "hoi_drv_msg_aes_init() -> init aes (riv0 : %08x)", msg->riv0);
    REPORT(INFO, "hoi_drv_msg_aes_init() -> init aes (riv1 : %08x)", msg->riv1);*/
    //set aes keys 
    eti_drv_set_aes(&handle->eti, &(msg->key0), &(msg->riv0)); 
    eto_drv_set_aes(&handle->eto, &(msg->key0), &(msg->riv0));
    return SUCCESS;
}

// return hdcp status (on/off)
int hoi_drv_msg_hdcpstat(t_hoi* handle, t_hoi_msg_hdcpstat* msg)
{
    eti_drv_aes_stat(&handle->eti, &msg->status_eti_audio, &msg->status_eti_video);
    eto_drv_aes_stat(&handle->eto, &msg->status_eto_audio, &msg->status_eto_video);
    return SUCCESS;
}
//-----------------------------------------------------------------------------
int hoi_drv_msg_eti(t_hoi* handle, t_hoi_msg_eti* msg)
{
    // when port = 0 not active!

    // load filter...
    eti_drv_set_filter(&handle->eti,
            msg->ip_address_dst, msg->ip_address_src_aud, msg->ip_address_src_vid,
            msg->udp_port_aud, msg->udp_port_vid);

    // flush buffers
    if (msg->udp_port_vid) vso_drv_flush_buf(&handle->vso);
    if (msg->udp_port_aud) aso_drv_flush_buf(&handle->aso);

    // activate ethernet input
    if (msg->udp_port_vid) eti_drv_start_vid(&handle->eti);
    if (msg->udp_port_aud) eti_drv_start_aud(&handle->eti);

    return SUCCESS;
}


int hoi_drv_msg_stsync(t_hoi* handle, t_hoi_msg_param* msg)
{
    
    if (msg->value & DRV_ST_VIDEO) {
        stream_sync_chg_source(&handle->sync, SYNC_SOURCE_VIDEO);
    } else if (msg->value & DRV_ST_AUDIO) {
        stream_sync_chg_source(&handle->sync, SYNC_SOURCE_AUDIO);
    }
    stream_sync_start(&handle->sync);
    
    return SUCCESS;
}


int hoi_drv_msg_syncdelay(t_hoi* handle, t_hoi_msg_param* msg)
{
    msg->value = stream_sync_get_delay(&handle->sync);

    return SUCCESS;
}


//------------------------------------------------------------------------------
// Status

int hoi_drv_msg_ethstat(t_hoi* handle, t_hoi_msg_ethstat* msg)
{
    msg->rx_cpu_cnt = eti_get_cpu_packet_cnt(handle->p_esi);
    msg->rx_aud_cnt = eti_get_aud_packet_cnt(handle->p_esi);
    msg->rx_vid_cnt = eti_get_vid_packet_cnt(handle->p_esi);
    msg->rx_inv_cnt = eti_get_inv_packet_cnt(handle->p_esi);
    msg->debug = eti_get_debug_reg(handle->p_esi);

    fec_rx_statistics(handle->p_fec_ip_rx, &handle->fec_rx);

    msg->tx_cpu_cnt = eto_get_cpu_packet_cnt(handle->p_eso);
    msg->tx_aud_cnt = eto_get_aud_packet_cnt(handle->p_eso);
    msg->tx_vid_cnt = eto_get_vid_packet_cnt(handle->p_eso);
    msg->tx_inv_cnt = eto_get_inv_packet_cnt(handle->p_eso);

    return SUCCESS;
}

int hoi_drv_msg_vsostat(t_hoi* handle, t_hoi_msg_vsostat* msg)
{
    msg->vframe_skip = vso_get_vframe_lost(handle->p_vso);
    msg->vframe_cnt = vso_get_vframe_cnt(handle->p_vso);
    msg->packet_cnt = vso_get_packet_cnt(handle->p_vso);
    msg->packet_lost = vso_get_packet_lost(handle->p_vso);
    msg->packet_in_cnt = vso_get_packet_in_cnt(handle->p_vso);
    msg->status = vso_get_status(handle->p_vso, VSO_ST_MSK);
    
    return SUCCESS;
}

int hoi_drv_msg_asoreg(t_hoi* handle, t_hoi_msg_asoreg* msg)
{
    msg->config = aso_get_ctrl(handle->p_aso, 0xFFFFFFFF);
    msg->status = aso_get_status(handle->p_aso, 0xFFFFFFFF);

    return SUCCESS;
}

int hoi_drv_msg_viostat(t_hoi* handle, t_hoi_msg_viostat* msg)
{
    msg->fin = vio_get_fin(handle->p_vio);
    msg->fout = vio_get_fout(handle->p_vio);
    msg->tgerr = vio_get_tg_error(handle->p_vio);
    msg->pllerr = vio_get_pll_error(handle->p_vio);
    msg->fvsync = vio_get_statistic_fvsnyc(handle->p_vio);
    msg->vid_in = vio_get_statistic_vid_in(handle->p_vio);
    msg->vid_out = vio_get_statistic_vid_out(handle->p_vio);
    msg->st_in = vio_get_statistic_st_in(handle->p_vio);
    msg->st_out = vio_get_statistic_st_out(handle->p_vio);

    return SUCCESS;
}

int hoi_drv_msg_getversion(t_hoi* handle, t_hoi_msg_version* msg)
{
    msg->fpga_date      = ver_get_fpga_date(handle->p_ver);
    msg->fpga_svn       = ver_get_fpga_svn(handle->p_ver);
    msg->sysid_date     = ver_get_sopc_date(handle->p_sysid);
    msg->sysid_id       = ver_get_sopc_id(handle->p_sysid);
    return SUCCESS;
}

//------------------------------------------------------------------------------
// Stream I/O

int hoi_drv_msg_vsi(t_hoi* handle, t_hoi_msg_vsi* msg)
{
    // start fec tx block;
    start_fec_ip_tx(handle->p_fec_ip_tx, msg->fec.video_enable, msg->fec.video_l, msg->fec.video_d, msg->fec.video_interleaving, msg->fec.video_col_only, 
                                         msg->fec.audio_enable, msg->fec.audio_l, msg->fec.audio_d, msg->fec.audio_interleaving, msg->fec.audio_col_only);

    // activate ethernet output
    eto_drv_start_vid(&handle->eto);

    // set parameters for packet generation in fec block
    fec_drv_set_video_eth_params(handle->p_fec_tx, &msg->eth);

    // setup vsi
    vsi_drv_go(&handle->vsi, &msg->eth);

    // get video timing
    if (handle->drivers & DRV_ADV7441) {
        if (adv7441a_get_video_timing(&handle->adv7441a)) {
            REPORT(INFO, "adv7441a_get_video_timing results not valid");
        }
        vio_copy_adv7441_timing(&msg->timing, &handle->adv7441a);
    }
    if (handle->drivers & DRV_GS2971) {
        if (gs2971_get_interlaced(&handle->gs2971)) {
            vio_get_timing(handle->p_vio, &msg->timing);
        }
        gs2971_get_video_timing(&handle->gs2971, &msg->timing); // interlaced only
        vio_copy_gs2971_timing(&msg->timing, &handle->gs2971);
    }

    vio_get_input_frequency(handle->p_vio, &msg->timing);
    if (!msg->timing.interlaced) {
        vio_get_timing(handle->p_vio, &msg->timing);
    }

    // if sdi, set output data rate
    if (handle->drivers & DRV_GS2972) {
        gs2972_driver_set_data_rate(&handle->gs2972, msg->timing.pfreq);
    }

    memcpy(&handle->vio.timing, &msg->timing, sizeof(t_video_timing));

    eto_drv_set_frame_period(&handle->eto, &handle->vio.timing, &msg->fec, msg->enable_traffic_shaping);

    // setup vio
    if (msg->compress) {
        vio_drv_encodex(&handle->vio, msg->bandwidth, msg->chroma, msg->advcnt, bdt_return_device(&handle->bdt));
    } else {
        vio_drv_plainin(&handle->vio, bdt_return_device(&handle->bdt));
    }

    vio_drv_get_advcnt(&handle->vio, &msg->advcnt);

    return SUCCESS;
}


int hoi_drv_msg_vso(t_hoi* handle, t_hoi_msg_vso* msg)
{
    int n;
    int delay, scomm5, vid, vsd;
    uint32_t ien;

    // activate FEC RX block
    init_fec_rx_ip_video(handle->p_fec_ip_rx, 1);

    // setup vio
    if (msg->compress & DRV_CODEC) {
        n = vio_drv_decodex(&handle->vio, &msg->timing, msg->advcnt, bdt_return_device(&handle->bdt));
    } else {
        n = vio_drv_plainoutx(&handle->vio, &msg->timing, bdt_return_device(&handle->bdt));
    }

    if (n) {
        REPORT(INFO, "vio_drv_(output) failed: %s", vio_str_err(n));
    }

    // sync...
    vso_drv_stop(&handle->vso);
    fec_rx_disable_video_out(handle->p_fec_rx);

    // if sdi, set output data rate
    if (handle->drivers & DRV_GS2972) {
        gs2972_driver_set_data_rate(&handle->gs2972, msg->timing.pfreq);
    }

    // setup vso (20ms delay, 15ms scomm5 delay, 1ms packet timeout)
    vso_set_vsync_blanking(&handle->vso, &msg->timing);
    vid = vid_duration_in_us(&msg->timing);  
    delay = (((67 + msg->delay_ms) * 1000) + vid);
    if (msg->timing.interlaced) {
        delay += (2*vid);
    }

    scomm5 = vid * 1500;
    vsd = vid * 800;
    if ((n = vso_drv_update(&handle->vso, &msg->timing, delay, vsd, scomm5, 1000))) {
        REPORT(INFO, "vso_drv_update failed: %s", vso_str_err(n));
        return n;
    }

    // !!! workaround !!!
    // ADV212 chips are sensitive to timing
    // switch of all nios2-irq
    ien = __builtin_rdctl(3);
    __builtin_wrctl(3, 0);

    // enable fec rx output interface
    fec_rx_enable_video_out(handle->p_fec_rx);

    if ((n = vso_drv_start(&handle->vso))) {
        __builtin_wrctl(3, ien);
        REPORT(INFO, "vso_drv_start failed: %s", vso_str_err(n));
        return n;
    }

    // sync all needed ADV212
    if ((n = vio_drv_decode_sync(&handle->vio))) {
        __builtin_wrctl(3, ien);
        REPORT(INFO, "vio_drv_decode_sync failed: %s", vio_str_err(n));
        return n;
    }

    __builtin_wrctl(3, ien);
    // !!! back to normal !!!

    if (msg->compress & DRV_STREAM_SYNC) {
        stream_sync_chg_source(&handle->sync, SYNC_SOURCE_VIDEO);
        stream_sync_start(&handle->sync);
    }

    return SUCCESS;
}

int hoi_drv_msg_vso_repaire(t_hoi* handle)
{
    int n;
    uint32_t ien;
// TODO: check if needed

    // stop/flush vso
    vso_drv_stop(&handle->vso);
    vso_drv_flush_buf(&handle->vso);

    vio_drv_decode(&handle->vio, bdt_return_device(&handle->bdt));

    vso_drv_stop(&handle->vso);

    // !!! workaround !!!
    // ADV212 chips are sensitive to timing
    // switch of all nios2-irq
    ien = __builtin_rdctl(3);
    __builtin_wrctl(3, 0);

    if ((n = vso_drv_start(&handle->vso))) {
        __builtin_wrctl(3, ien);
        REPORT(INFO, "vso_drv_start failed: %s", vso_str_err(n));
        return n;
    }

    // sync all needed ADV212
    if ((n = vio_drv_decode_sync(&handle->vio))) {
        __builtin_wrctl(3, ien);
        REPORT(INFO, "vio_drv_decode_sync failed: %s", vio_str_err(n));
        return n;
    }

    __builtin_wrctl(3, ien);
    // !!! back to normal !!!

    return SUCCESS;
}

int hoi_drv_msg_asi(t_hoi* handle, t_hoi_msg_asi* msg)
{
    asi_drv_flush_buf(&handle->asi);

    // start fec tx block;
    start_fec_ip_tx(handle->p_fec_ip_tx, msg->fec.video_enable, msg->fec.video_l, msg->fec.video_d, msg->fec.video_interleaving, msg->fec.video_col_only, 
                                         msg->fec.audio_enable, msg->fec.audio_l, msg->fec.audio_d, msg->fec.audio_interleaving, msg->fec.audio_col_only);
    // activate ethernet output
    eto_drv_start_aud(&handle->eto);

    // set parameters for packet generation in fec block
    fec_drv_set_audio_eth_params(handle->p_fec_tx, &msg->eth);

    // setup asi
    asi_drv_update(&handle->asi, msg->stream_nr, &msg->eth, &msg->aud); // this also stops and restarts stream

    return SUCCESS;
}

int hoi_drv_msg_aso(t_hoi* handle, t_hoi_msg_aso* msg)
{
    int delay, vid = 0;

    // activate FEC RX block
    init_fec_rx_ip_audio(handle->p_fec_ip_rx, 1);

    // sync...
    aso_drv_stop(&handle->aso);
    fec_rx_disable_audio_emb_out(handle->p_fec_rx);
    fec_rx_disable_audio_int_out(handle->p_fec_rx); 

    // setup aso ()
    if (handle->vio.active) {
        vid = vid_duration_in_us(&handle->vio.timing);
        delay = (((67 + msg->delay_ms + msg->av_delay - 53) * 1000) + vid);     // audio is 53ms too late
        
        if (handle->vio.timing.interlaced) {
            delay += (2*vid);
        }
    }

    // if av-delay is too small
    if (delay < msg->delay_ms) {
        delay = msg->delay_ms;
    }

    // enable fec rx output interface
    fec_rx_enable_audio_emb_out(handle->p_fec_rx);
    fec_rx_enable_audio_int_out(handle->p_fec_rx); 

    aso_drv_update(&handle->aso, &(msg->aud), delay);
    aso_drv_start(&handle->aso);

    if (handle->drivers & DRV_ADV9889) {
        adv9889_drv_setup_audio(&handle->adv9889, aud_chmap2cnt(msg->aud.ch_map), msg->aud.fs, msg->aud.sample_width);
    }
    
    if (msg->cfg & DRV_STREAM_SYNC) {
        stream_sync_chg_source(&handle->sync, SYNC_SOURCE_AUDIO);
        stream_sync_start(&handle->sync);
    }

    return SUCCESS;
}

//------------------------------------------------------------------------------
// Image capture / show

int hoi_drv_msg_capture(t_hoi* handle, t_hoi_msg_image* msg)
{
    uint32_t ret = SUCCESS;

    // TODO: access userspace buffer properly
    // TODO: proper bandwidth/size

    if (msg->compress) {
        ret = vrp_drv_capture_jpeg2000(&handle->vrp, msg->buffer, msg->size, (msg->size * 3 / 4), bdt_return_device(&handle->bdt));
    } else {
        ret = vrp_drv_capture_image(&handle->vrp, msg->buffer, msg->size, bdt_return_device(&handle->bdt));
    }

    vio_drv_get_timing(&handle->vio, &msg->timing);
    vio_drv_get_advcnt(&handle->vio, &msg->advcnt);

    return ret;
}

int hoi_drv_msg_show(t_hoi* handle, t_hoi_msg_image* msg)
{
    uint32_t ret = SUCCESS;

    // if sdi, set output data rate
    if (handle->drivers & DRV_GS2972) {
        gs2972_driver_set_data_rate(&handle->gs2972, msg->timing.pfreq);
    }

    if (msg->compress) {
        ret = vrp_drv_show_jpeg2000(&handle->vrp, msg->buffer, &msg->timing, msg->advcnt, bdt_return_device(&handle->bdt));
    } else {
        ret = vrp_drv_show_image(&handle->vrp, msg->buffer, &msg->timing, bdt_return_device(&handle->bdt));
    }

    return ret;
}

int hoi_drv_msg_debug(t_hoi* handle, t_hoi_msg_image* msg)
{
    uint32_t ret = SUCCESS;

    return ret;
}

int hoi_drv_msg_set_timing(t_hoi* handle, t_hoi_msg_image* msg)
{
    uint32_t ret = SUCCESS;

    vio_drv_debugx(&handle->vio, &msg->timing, msg->vtb, bdt_return_device(&handle->bdt), &handle->gs2972, &handle->adv7441a);

    return ret;
}

int hoi_drv_msg_bw(t_hoi* handle, t_hoi_msg_bandwidth* msg)
{
    vio_drv_set_bandwidth(&handle->vio, msg->bw, msg->chroma);
    return SUCCESS;
}

int hoi_drv_msg_led(t_hoi* handle, t_hoi_msg_param* msg)
{
    led_drv_set_status(&handle->led, msg->value);
    return SUCCESS;
}

int hoi_drv_msg_new_audio(t_hoi* handle, t_hoi_msg_param* msg)
{
    uint16_t ch_map;

    if (handle->drivers & DRV_ADV7441) {
        ch_map = asi_drv_get_detected_ch_map(&handle->asi, AUD_STREAM_NR_EMBEDDED);
        adv7441a_audio_fs_change(&handle->adv7441a, msg->value, ch_map);
    }
    if (handle->drivers & DRV_GS2971) {
        gs2971_get_audio_config(&handle->gs2971);
    }

    return SUCCESS;
}

//------------------------------------------------------------------------------
// Command

int hoi_drv_msg_loop(t_hoi* handle)
{
    vio_drv_loop(&handle->vio, bdt_return_device(&handle->bdt));
    return SUCCESS;
}

int hoi_drv_msg_osdon(t_hoi* handle)
{
    vio_drv_set_osd(&handle->vio, true);
    return SUCCESS;
}

int hoi_drv_msg_osdoff(t_hoi* handle)
{
    vio_drv_set_osd(&handle->vio, false);
    return SUCCESS;
}

int hoi_drv_msg_hpdon(t_hoi* handle)
{
    vio_drv_set_hpd(&handle->vio, true);
    return SUCCESS;
}

int hoi_drv_msg_hpdoff(t_hoi* handle)
{
    vio_drv_set_hpd(&handle->vio, false);
    return SUCCESS;
}

int hoi_drv_msg_hpdreset(t_hoi* handle)
{
    vio_hdp_reset(handle->p_vio);
    return SUCCESS;
}

//------------------------------------------------------------------------------
// TAG I/O

int hoi_drv_msg_rdvidtag(t_hoi* handle, t_hoi_msg_tag* msg)
{
    uint32_t ret = tag_drv_inquire(&handle->vtag);

    msg->available = handle->vtag.available;
    memcpy(msg->tag, handle->vtag.buf, 256);

    return ret;
}

int hoi_drv_msg_rdaudtag(t_hoi* handle, t_hoi_msg_tag* msg)
{
    uint32_t ret = tag_drv_inquire(&handle->atag);

    msg->available = handle->atag.available;
    memcpy(msg->tag, handle->atag.buf, 256);

    return ret;
}

int hoi_drv_msg_wrvidtag(t_hoi* handle, t_hoi_msg_tag* msg)
{
    memcpy(handle->vtag.buf, msg->tag, 256);
    return tag_drv_write(&handle->vtag);
}

int hoi_drv_msg_wraudtag(t_hoi* handle, t_hoi_msg_tag* msg)
{
    memcpy(handle->atag.buf, msg->tag, 256);
    return tag_drv_write(&handle->atag);
}

int hoi_drv_msg_rdedid(t_hoi* handle, t_hoi_msg_edid* msg)
{
    uint8_t *edid = msg->edid;

    if (handle->drivers & DRV_ADV9889) {
        adv9889_drv_get_edid(&handle->adv9889, msg->edid);
    } 

    if (handle->drivers & DRV_GS2972) {
        memcpy(edid, sdi_edid, 256);
    } 

    return SUCCESS;
}

int hoi_drv_msg_wredid(t_hoi* handle, t_hoi_msg_edid* msg)
{
    if (handle->drivers & DRV_ADV7441) {
        adv7441a_set_edid(&handle->adv7441a, msg->edid);
    } else {
        REPORT(INFO, "wredid: no driver loaded");
        // return NO_DRIVER_LOADED
    }
    return SUCCESS;
}

//------------------------------------------------------------------------------
// info

int hoi_drv_msg_info(t_hoi* handle, t_hoi_msg_info* msg)
{
    int min = 0;
    // read video input timing

    if (handle->drivers & DRV_ADV7441) {
        if (adv7441a_get_video_timing(&handle->adv7441a)) {
            REPORT(INFO, "adv7441a_get_video_timing results not valid");
        }
        vio_copy_adv7441_timing(&msg->timing, &handle->adv7441a);
    }
    if (handle->drivers & DRV_GS2971) {
        if (gs2971_get_interlaced(&handle->gs2971)) {
            vio_get_timing(handle->p_vio, &msg->timing);
        }
        gs2971_get_video_timing(&handle->gs2971, &msg->timing); // interlaced only
        vio_copy_gs2971_timing(&msg->timing, &handle->gs2971);
    }

    vio_get_input_frequency(handle->p_vio, &msg->timing);
    if (!msg->timing.interlaced) {
        vio_get_timing(handle->p_vio, &msg->timing);
    }

    adv212_drv_advcnt(&msg->timing, &min);
    if (msg->advcnt < min) msg->advcnt = min;

	msg->advcnt_old = handle->vio.adv.cnt;

    // read audio input timing (from video board) [ch: 0..15]
    if (handle->drivers & DRV_ADV7441) {
        // TODO: perhaps replace t_adv7441a_aud_st with hdoip_aud_params
        msg->aud_params[AUD_STREAM_NR_EMBEDDED].fs = handle->adv7441a.aud_st.fs;
        msg->aud_params[AUD_STREAM_NR_EMBEDDED].sample_width = handle->adv7441a.aud_st.bit_width;
        msg->aud_params[AUD_STREAM_NR_EMBEDDED].ch_map = handle->adv7441a.aud_st.ch_map;
    }
    if (handle->drivers & DRV_GS2971) {
        msg->aud_params[AUD_STREAM_NR_EMBEDDED].fs = handle->gs2971.aud_st.fs;
        msg->aud_params[AUD_STREAM_NR_EMBEDDED].sample_width = handle->gs2971.aud_st.bit_width;
        msg->aud_params[AUD_STREAM_NR_EMBEDDED].ch_map = handle->gs2971.aud_st.ch_map;
    }
    // read audio input timing (from audio board)
    msg->aud_params[AUD_STREAM_NR_IF_BOARD].fs = 0;
    msg->aud_params[AUD_STREAM_NR_IF_BOARD].sample_width = 0;
    msg->aud_params[AUD_STREAM_NR_IF_BOARD].ch_map = 0;
    return SUCCESS;
}


//------------------------------------------------------------------------------
// param read/write

int hoi_drv_msg_off(t_hoi* handle, t_hoi_msg_param* msg)
{
    hoi_drv_reset(handle, msg->value);
    return SUCCESS;
}

int hoi_drv_msg_ifmt(t_hoi* handle, t_hoi_msg_param* msg)
{
    vio_drv_set_format_in(&handle->vio, msg->value);
    return SUCCESS;
}


int hoi_drv_msg_ofmt(t_hoi* handle, t_hoi_msg_param* msg)
{
    vio_drv_set_format_out(&handle->vio, msg->value);
    return SUCCESS;
}


int hoi_drv_msg_pfmt(t_hoi* handle, t_hoi_msg_param* msg)
{
    vio_drv_set_format_proc(&handle->vio, msg->value);
    return SUCCESS;
}

int hoi_drv_msg_get_mtime(t_hoi* handle, t_hoi_msg_param* msg)
{
    msg->value = tmr_get_master(handle->p_tmr);
    return SUCCESS;
}

int hoi_drv_msg_set_mtime(t_hoi* handle, t_hoi_msg_param* msg)
{
    tmr_set_master(handle->p_tmr, msg->value);
    return SUCCESS;
}

int hoi_drv_msg_get_stime(t_hoi* handle, t_hoi_msg_param* msg)
{
    msg->value = tmr_get_slave(handle->p_tmr);
    return SUCCESS;
}

int hoi_drv_msg_get_fs(t_hoi* handle, t_hoi_msg_param* msg)
{
    msg->value = asi_drv_get_detected_fs(&handle->asi, AUD_STREAM_NR_EMBEDDED);
    return SUCCESS;
}

int hoi_drv_msg_get_analog_timing(t_hoi* handle, t_hoi_msg_param* msg)
{
    if (handle->drivers & DRV_ADV7441) {
        msg->value = adv7441a_get_analog_video_timing(&handle->adv7441a);
    }
    return SUCCESS;
}

int hoi_drv_msg_get_device_id(t_hoi* handle, t_hoi_msg_param* msg)
{
    msg->value = bdt_return_device(&handle->bdt);
    return SUCCESS;
}

int hoi_drv_mgs_get_encrypted_status(t_hoi* handle, t_hoi_msg_param* msg)
{
    msg->value = adv7441a_get_hdcp_status(&handle->adv7441a);
    return SUCCESS;
}

int hoi_drv_mgs_get_active_resolution(t_hoi* handle, t_hoi_msg_param* msg)
{
    msg->value = adv7441a_poll_active_resolution(&handle->adv7441a, handle->p_vio);
    return SUCCESS;
}

int hoi_drv_msg_get_reset_to_default(t_hoi* handle, t_hoi_msg_param* msg)
{
    msg->value = bdt_drv_get_reset_to_default(&handle->bdt, handle->p_video_mux);
    return SUCCESS;
}

int hoi_drv_msg_set_stime(t_hoi* handle, t_hoi_msg_param* msg)
{
    tmr_set_slave(handle->p_tmr, msg->value);
    REPORT(INFO, "Set Slave-Timer: %u", msg->value);
    return SUCCESS;
}

int hoi_drv_msg_tmr(t_hoi* handle, t_hoi_msg_param* msg)
{
    tmr_set_mux(handle->p_tmr, msg->value);
    return SUCCESS;
}


int hoi_drv_msg_poll(t_hoi* handle)
{
    hoi_drv_timer(handle);

    if (!queue_empty(handle->event)) {
        wake_up_interruptible(&handle->eq);
    }

    return SUCCESS;
}

int hoi_drv_msg_hdcp_get_key(t_hoi* handle, t_hoi_msg_hdcp_key* msg)
{
    hdcp_get_master_key(handle->p_hdcp, msg->key);
    return SUCCESS;
}

int hoi_drv_msg_hdcp_get_timer(t_hoi* handle, t_hoi_msg_hdcp_timer* msg)
{
    msg->config = hdcp_get_cfg(handle->p_hdcp, 0xFFFFFFFF);
    msg->status = hdcp_get_status(handle->p_hdcp, 0xFFFFFFFF);
    msg->start_time = hdcp_get_time(handle->p_hdcp);
    return SUCCESS;
}

int hoi_drv_msg_hdcp_set_timer(t_hoi* handle, t_hoi_msg_hdcp_timer* msg)
{
    hdcp_set_time(handle->p_hdcp, msg->start_time);
    hdcp_load(handle->p_hdcp);
    return SUCCESS;
}

int hoi_drv_msg_hdcp_timer_enable(t_hoi* handle)
{
    hdcp_enable(handle->p_hdcp);
    return SUCCESS;
}

int hoi_drv_msg_hdcp_timer_disable(t_hoi* handle)
{
    hdcp_disable(handle->p_hdcp);
    return SUCCESS;
}

int hoi_drv_msg_hdcp_timer_load(t_hoi* handle)
{
    hdcp_load(handle->p_hdcp);
    return SUCCESS;
}

int hoi_drv_msg_hdcp_black_output(t_hoi* handle)
{
    vio_drv_set_black_output(&handle->vio);
    return SUCCESS;
}

int hoi_drv_osd_clr_border(t_hoi* handle)
{
    vio_drv_osd_clr_border(&handle->vio.osd);
    return SUCCESS;
}

int hoi_drv_msg_wdg_enable(t_hoi* handle)
{
    wdg_enable(handle->p_wdg);
    return SUCCESS;
}

int hoi_drv_msg_wdg_disable(t_hoi* handle)
{
    wdg_disable(handle->p_wdg);
    return SUCCESS;
}

int hoi_drv_msg_wdg_service(t_hoi* handle)
{
    wdg_reset(handle->p_wdg);
    return SUCCESS;
}

int hoi_drv_msg_wdg_init(t_hoi* handle, t_hoi_msg_wdg* msg)
{
    wdg_disable(handle->p_wdg);
    wdg_set_time(handle->p_wdg, msg->service_time);
    wdg_reset(handle->p_wdg);
    return SUCCESS;
}

int hoi_drv_msg_read_ram(t_hoi* handle, t_hoi_msg_param* msg)
{
    int i;

    for (i=0;i<200;i++) {
        if (i%4 == 0) printk("\n 0x%03x | ", i*4);
        printk("%08x ", HOI_RD32(msg->value, i*4));
    }

    return SUCCESS;
}

int hoi_drv_msg_set_fps_reduction(t_hoi* handle, t_hoi_msg_param* msg)
{
    eto_drv_frame_rate_reduction(&handle->eto, msg->value);
    return SUCCESS;
}

int hoi_drv_msg_clear_osd(t_hoi* handle)
{
    vio_osd_clear_screen(handle->p_vio);

    handle->vio.osd.x = 0;
    handle->vio.osd.y = 0;

    return SUCCESS;
}

int hoi_drv_msg_set_fec_tx_params(t_hoi* handle, t_hoi_msg_fec_tx* msg)
{
    // reconfigure traffic shaping because count of overhead packets changes
    eto_drv_set_frame_period(&handle->eto, &handle->vio.timing, &msg->fec, handle->eto.traffic_shaping_enable);

    change_setting_fec_ip_tx(handle->p_fec_ip_tx, msg->fec.video_enable, msg->fec.video_l, msg->fec.video_d, msg->fec.video_interleaving, msg->fec.video_col_only, 
                                                  msg->fec.audio_enable, msg->fec.audio_l, msg->fec.audio_d, msg->fec.audio_interleaving, msg->fec.audio_col_only);
    return SUCCESS;
}


//------------------------------------------------------------------------------
// message

#define call(x, f) case x: ret = f(handle, (void*)msg); break
#define callsw(x, f) case x: ret = f(handle); break
int hoi_drv_message(t_hoi* handle, t_hoi_msg* msg)
{
    uint32_t ret;

    switch (msg->id) {
        call(HOI_MSG_LDRV,                  hoi_drv_msg_ldrv);
        call(HOI_MSG_BUF,                   hoi_drv_msg_buf);
        call(HOI_MSG_ETI,                   hoi_drv_msg_eti);

        call(HOI_MSG_INFO,                  hoi_drv_msg_info);

        call(HOI_MSG_CAPTURE,               hoi_drv_msg_capture);
        call(HOI_MSG_SHOW,                  hoi_drv_msg_show);
        call(HOI_MSG_DEBUG,                 hoi_drv_msg_debug);
        call(HOI_MSG_SET_TIMING,            hoi_drv_msg_set_timing);
        call(HOI_MSG_VSI,                   hoi_drv_msg_vsi);
        call(HOI_MSG_VSO,                   hoi_drv_msg_vso);
        call(HOI_MSG_ASI,                   hoi_drv_msg_asi);
        call(HOI_MSG_ASO,                   hoi_drv_msg_aso);
        call(HOI_MSG_BW,                    hoi_drv_msg_bw);
        call(HOI_MSG_LED,                   hoi_drv_msg_led);
        call(HOI_MSG_NEW_AUDIO,             hoi_drv_msg_new_audio);
        call(HOI_MSG_GET_FS,                hoi_drv_msg_get_fs);
        call(HOI_MSG_GET_ANALOG_TIMING,     hoi_drv_msg_get_analog_timing);
        call(HOI_MSG_GET_DEV_ID,            hoi_drv_msg_get_device_id);
        call(HOI_MSG_GET_RESET_TO_DEFAULT,  hoi_drv_msg_get_reset_to_default);
        call(HOI_MSG_GET_ENCRYPTED_STATUS,  hoi_drv_mgs_get_encrypted_status);
        call(HOI_MSG_GET_ACTIVE_RESOLUTION, hoi_drv_mgs_get_active_resolution);
        call(HOI_MSG_OFF,                   hoi_drv_msg_off);
        call(HOI_MSG_IFMT,                  hoi_drv_msg_ifmt);
        call(HOI_MSG_OFMT,                  hoi_drv_msg_ofmt);
        call(HOI_MSG_PFMT,                  hoi_drv_msg_pfmt);
        call(HOI_MSG_RDVIDTAG,              hoi_drv_msg_rdvidtag);
        call(HOI_MSG_RDAUDTAG,              hoi_drv_msg_rdaudtag);
        call(HOI_MSG_WRVIDTAG,              hoi_drv_msg_wrvidtag);
        call(HOI_MSG_WRAUDTAG,              hoi_drv_msg_wraudtag);
        call(HOI_MSG_RDEDID,                hoi_drv_msg_rdedid);
        call(HOI_MSG_WREDID,                hoi_drv_msg_wredid);

        call(HOI_MSG_GETMTIME,              hoi_drv_msg_get_mtime);
        call(HOI_MSG_SETMTIME,              hoi_drv_msg_set_mtime);
        call(HOI_MSG_GETSTIME,              hoi_drv_msg_get_stime);
        call(HOI_MSG_SETSTIME,              hoi_drv_msg_set_stime);
        call(HOI_MSG_TIMER,                 hoi_drv_msg_tmr);
        call(HOI_MSG_STSYNC,                hoi_drv_msg_stsync);
        call(HOI_MSG_SYNCDELAY,             hoi_drv_msg_syncdelay);
        call(HOI_MSG_SET_FPS_REDUCTION,     hoi_drv_msg_set_fps_reduction);

        call(HOI_MSG_ETHSTAT,               hoi_drv_msg_ethstat);
        call(HOI_MSG_VSOSTAT,               hoi_drv_msg_vsostat);
        call(HOI_MSG_VIOSTAT,               hoi_drv_msg_viostat);
        call(HOI_MSG_ASOREG,                hoi_drv_msg_asoreg);
        call(HOI_MSG_HDCP_INIT,             hoi_drv_msg_hdcp_init);
        call(HOI_MSG_HDCPSTAT,              hoi_drv_msg_hdcpstat);

        call(HOI_MSG_GETVERSION,            hoi_drv_msg_getversion);

        callsw(HOI_MSG_LOOP,                hoi_drv_msg_loop);
        callsw(HOI_MSG_OSDON,               hoi_drv_msg_osdon);
        callsw(HOI_MSG_OSDOFF,              hoi_drv_msg_osdoff);
        callsw(HOI_MSG_REPAIR,              hoi_drv_msg_vso_repaire);
        callsw(HOI_MSG_HPDON,               hoi_drv_msg_hpdon);
        callsw(HOI_MSG_HPDOFF,              hoi_drv_msg_hpdoff);
        callsw(HOI_MSG_HPDRESET,            hoi_drv_msg_hpdreset);

        call(HOI_MSG_HDCP_GET_KEY,          hoi_drv_msg_hdcp_get_key);
        call(HOI_MSG_HDCP_TIMER_GET,        hoi_drv_msg_hdcp_get_timer);
        call(HOI_MSG_HDCP_TIMER_SET,        hoi_drv_msg_hdcp_set_timer);
        callsw(HOI_MSG_HDCP_TIMER_ENABLE,   hoi_drv_msg_hdcp_timer_enable);
        callsw(HOI_MSG_HDCP_TIMER_DISABLE,  hoi_drv_msg_hdcp_timer_disable);
        callsw(HOI_MSG_HDCP_TIMER_LOAD,     hoi_drv_msg_hdcp_timer_load);
        callsw(HOI_MSG_HDCP_BLACK_OUTPUT,   hoi_drv_msg_hdcp_black_output);

        call(HOI_MSG_WDG_INIT,              hoi_drv_msg_wdg_init);
        callsw(HOI_MSG_OSD_CLR_BORDER,      hoi_drv_osd_clr_border);
        callsw(HOI_MSG_WDG_ENABLE,          hoi_drv_msg_wdg_enable);
        callsw(HOI_MSG_WDG_DISABLE,         hoi_drv_msg_wdg_disable);
        callsw(HOI_MSG_WDG_SERVICE,         hoi_drv_msg_wdg_service);

        callsw(HOI_MSG_HDCP_ENVIDEO_ETI,    hoi_drv_msg_hdcp_video_en_eti);
        callsw(HOI_MSG_HDCP_ENVIDEO_ETO,    hoi_drv_msg_hdcp_video_en_eto);
        callsw(HOI_MSG_HDCP_ENAUDIO_ETI,    hoi_drv_msg_hdcp_audio_en_eti);
        callsw(HOI_MSG_HDCP_ENAUDIO_ETO,    hoi_drv_msg_hdcp_audio_en_eto);

        callsw(HOI_MSG_HDCP_DISVIDEO_ETI,   hoi_drv_msg_hdcp_video_dis_eti);
        callsw(HOI_MSG_HDCP_DISVIDEO_ETO,   hoi_drv_msg_hdcp_video_dis_eto);
        callsw(HOI_MSG_HDCP_DISAUDIO_ETI,   hoi_drv_msg_hdcp_audio_dis_eti);
        callsw(HOI_MSG_HDCP_DISAUDIO_ETO,   hoi_drv_msg_hdcp_audio_dis_eto);

        callsw(HOI_MSG_HDCP_ENAD9889,       hoi_drv_msg_hdcp_ADV9889_en);
        callsw(HOI_MSG_HDCP_DISAD9889,      hoi_drv_msg_hdcp_ADV9889_dis);

        callsw(HOI_MSG_POLL,                hoi_drv_msg_poll);
        callsw(HOI_MSG_CLR_OSD,             hoi_drv_msg_clear_osd);

        call(HOI_MSG_FEC_TX,                hoi_drv_msg_set_fec_tx_params);
        call(HOI_MSG_DEBUG_READ_RAM,        hoi_drv_msg_read_ram);

        default: ret = ERR_HOI_CMD_NOT_SUPPORTED; break;
    }

    return ret;
}
