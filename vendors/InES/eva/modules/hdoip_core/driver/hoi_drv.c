/*
 * hoi_drv.c
 *
 *  Created on: 08.10.2010
 *      Author: alda
 */
#include "hoi_drv.h"
#include "vid_const.h"
#include "ext_irq_pio.h"
#include "hoi_msg.h"
#include "wdg_hal.h"



/* base driver initialization
 *
 * @param handle hoi driver handle
 */
void hoi_drv_init(t_hoi* handle)
{
    // TODO proper range in config.h
    handle->p_tx            = ioremap(BASE_VIDEO_TX,    0xffffffff);
    handle->p_rx            = ioremap(BASE_VIDEO_RX,    0xffffffff);
    handle->p_i2c_tag_aud   = ioremap(BASE_I2C_AUD_TAG, 0xffffffff);
    handle->p_i2c_tag_vid   = ioremap(BASE_I2C_VID_TAG, 0xffffffff);
    handle->p_vio           = ioremap(BASE_VIO,         0xffffffff);
    handle->p_vsi           = ioremap(BASE_VSI,         0xffffffff);
    handle->p_vso           = ioremap(BASE_VSO,         0xffffffff);
    handle->p_aso           = ioremap(BASE_ASO,         0xffffffff);
    handle->p_asi           = ioremap(BASE_ASI,         0xffffffff);
    handle->p_adv212        = ioremap(BASE_ADV212,      0xffffffff);
    handle->p_vrp           = ioremap(BASE_VRP,         0xffffffff);
    handle->p_tmr           = ioremap(BASE_TIMER,       0xffffffff);
    handle->p_wdg           = ioremap(BASE_WATCHDOG,    0xffffffff);
    handle->p_hdcp          = ioremap(BASE_HDCP,        0xffffffff);
    handle->p_irq           = ioremap(BASE_EXT_IRQ,     0xffffffff);
    handle->p_esi           = ioremap(BASE_ETI,         0xffffffff);
    handle->p_eso           = ioremap(BASE_ETO,         0xffffffff);
    handle->p_ver           = ioremap(BASE_VER,         0xffffffff);
    handle->p_sysid         = ioremap(BASE_SYSID,       0xffffffff);
    handle->p_led           = ioremap(BASE_LED,         0xffffffff);

    // init
    handle->event = queue_init(100);

    // setup components

    // timer init
    tmr_init(handle->p_tmr);

    // init i2c with 400kHz
    i2c_drv_init(&handle->i2c_tx, handle->p_tx, 100000);
    i2c_drv_init(&handle->i2c_rx, handle->p_rx, 400000);
//    i2c_drv_init(&handle->i2c_tag_aud, handle->p_i2c_tag_aud, 400000);
    i2c_drv_init(&handle->i2c_tag_vid, handle->p_i2c_tag_vid, 400000);
 
    // read video card id
    bdt_drv_read_id(&handle->bdt, &handle->i2c_tag_vid);

    // init
    led_drv_init(&handle->led, &handle->i2c_tag_vid, &handle->i2c_tag_aud, handle->p_led, &handle->bdt);
    eti_drv_set_ptr(&handle->eti, handle->p_esi);
    eto_drv_set_ptr(&handle->eto, handle->p_eso);
    vio_drv_init(&handle->vio, handle->p_vio, handle->p_adv212);
    vsi_drv_init(&handle->vsi, handle->p_vsi);
    vso_drv_init(&handle->vso, handle->p_vso);
    asi_drv_init(&handle->asi, handle->p_asi);
    aso_drv_init(&handle->aso, handle->p_aso);
    vio_drv_setup_osd(&handle->vio, (t_osd_font*)&vid_font_8x13);
    vrp_drv_init(&handle->vrp, &handle->vio, handle->p_vrp);
    stream_sync_init(&handle->sync, SIZE_MEANS, SIZE_RISES,
            handle->p_aso, handle->p_vso, handle->p_tmr,
            DEAD_TIME, P_GAIN, I_GAIN, INC_PPM);

    hoi_drv_stop(handle);
}

/** partially reset driver
 *
 * @param handle hoi handle
 * @param reset vectore
 */
void hoi_drv_reset(t_hoi* handle, uint32_t rv)
{
    // Stop Video I/O
    if (rv & (DRV_RST_VID_OUT | DRV_RST_VID_IN)) {
        REPORT(INFO, "reset vio");
        vio_drv_reset(&handle->vio);
    }

    // Deactivate VRP
    if (rv & DRV_RST_VRP) {
        REPORT(INFO, "reset vrp");
        vrp_drv_off(&handle->vrp);
    }

    // Stop Video Output
    if (rv & DRV_RST_VID_OUT) {
        REPORT(INFO, "reset vso/eti");
        vso_drv_stop(&handle->vso);
        eti_drv_stop_vid(&handle->eti);
        eti_set_config_video_enc_dis(handle->p_esi);
        vso_drv_flush_buf(&handle->vso);
    }

    // Stop Video Input
    if (rv & DRV_RST_VID_IN) {
        REPORT(INFO, "reset eto/vsi");
        eto_drv_stop_vid(&handle->eto);
        eto_set_config_video_enc_dis(handle->p_eso);
        vsi_drv_stop(&handle->vsi);
        vsi_drv_flush_buf(&handle->vsi);
    }

    // Stop Audio Output
    if (rv & DRV_RST_AUD_OUT) {
        REPORT(INFO, "reset aso/eti");
        aso_drv_stop(&handle->aso);
        eti_drv_stop_aud(&handle->eti);
        eti_set_config_audio_enc_dis(handle->p_esi);
        aso_drv_flush_buf(&handle->aso);
    }

    // Stop Audio Input
    if (rv & DRV_RST_AUD_IN) {
        REPORT(INFO, "reset eto/asi");
        eto_drv_stop_aud(&handle->eto);
        eto_set_config_audio_enc_dis(handle->p_eso);
        asi_drv_stop(&handle->asi);
        asi_drv_flush_buf(&handle->asi);
    }

    // Stop stream sync
    if (rv & DRV_RST_STSYNC) {
        REPORT(INFO, "reset st-sync");
        stream_sync_stop(&handle->sync);
    }

    if (rv & DRV_RST_TMR) {
        REPORT(INFO, "reset timer");
        tmr_init(handle->p_tmr);
    }

    REPORT(INFO, "reset done");
}


/** This function force the driver into idle state
 *
 * @param handle hoi handle
 */
void hoi_drv_stop(t_hoi* handle)
{
    hoi_drv_reset(handle, DRV_RST);
}

void hoi_drv_interrupt(t_hoi* handle)
{
    uint32_t irq = HOI_RD32(handle->p_irq, 0);

    irq = irq ^ EXT_IRQ_POLARITY;

    if (handle->drivers & DRV_ADV7441) {
        if (irq & EXT_IRQ_HDMI_RX_INT1) {
            adv7441a_irq1_handler(&handle->adv7441a, handle->event);
        }
        if (irq & EXT_IRQ_HDMI_RX_INT2) {
            adv7441a_irq2_handler(&handle->adv7441a, handle->event);
        }
    }

    if (handle->drivers & DRV_ADV9889) {
        if (irq & EXT_IRQ_HDMI_TX_INT1) {
            adv9889_irq_handler(&handle->adv9889, handle->event);
        }
    }

    if (irq & EXT_IRQ_J2K_CODEC_0) {
        vio_drv_irq_adv212(&handle->vio, 0, handle->event);
    }
    if (irq & EXT_IRQ_J2K_CODEC_1) {
        vio_drv_irq_adv212(&handle->vio, 1, handle->event);
    }
    if (irq & EXT_IRQ_J2K_CODEC_2) {
        vio_drv_irq_adv212(&handle->vio, 2, handle->event);
    }
    if (irq & EXT_IRQ_J2K_CODEC_3) {
        vio_drv_irq_adv212(&handle->vio, 3, handle->event);
    }
}

void hoi_drv_handler(t_hoi* handle)
{
    vio_drv_handler(&handle->vio, handle->event);
    vsi_drv_handler(&handle->vsi, handle->event);
    vso_drv_handler(&handle->vso, handle->event);
    asi_drv_handler(&handle->asi, handle->event);
    aso_drv_handler(&handle->aso, handle->event);
    eto_drv_handler(&handle->eto, handle->event);
    eti_drv_handler(&handle->eti, handle->event);
    led_drv_handler(&handle->led);
    stream_sync(&handle->sync);
    wdg_reset(handle->p_wdg); //reset watchdog
    if (handle->drivers & DRV_ADV9889) {
        adv9889_drv_handler(&handle->adv9889, handle->event);
    }
    if (handle->drivers & DRV_ADV7441) {
        adv7441a_handler(&handle->adv7441a, handle->event);
    }
    if ((handle->drivers & DRV_ADV9889) && (handle->drivers & DRV_ADV7441)) {
        hdcp_drv_handler(&handle->eti, &handle->eto, &handle->adv7441a, &handle->adv9889, &handle->vsi, &handle->vso, &handle->asi, &handle->aso, &handle->drivers, handle->event); //hdcp handler
    }
}

void hoi_drv_timer(t_hoi* handle)
{
    // interrupt
    hoi_drv_interrupt(handle);

    // handler
    hoi_drv_handler(handle);
}
