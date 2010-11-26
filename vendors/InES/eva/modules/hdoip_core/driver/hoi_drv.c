/*
 * hoi_drv.c
 *
 *  Created on: 08.10.2010
 *      Author: alda
 */
#include "hoi_drv.h"
#include "vid_const.h"
#include "ext_irq_pio.h"

// timer prototype
static int hoi_drv_timer(unsigned long data);


/* base driver initialization
 *
 * @param handle hoi driver handle
 */
void hoi_drv_init(t_hoi* handle)
{
    // TODO proper range in config.h
    void *p_tx      = ioremap(BASE_VIDEO_TX,    0xffffffff);
    void *p_rx      = ioremap(BASE_VIDEO_RX,    0xffffffff);
    void *p_vio     = ioremap(BASE_VIO,         0xffffffff);
    void *p_adv212  = ioremap(BASE_ADV212,      0xffffffff);
    void *p_vrp     = ioremap(BASE_VRP,         0xffffffff);
    void *p_reset   = ioremap(BASE_RESET,       0xffffffff);
    void *p_extirq  = ioremap(BASE_EXT_IRQ,     0xffffffff);
    void *p_aso     = ioremap(BASE_ASO,         0xffffffff);
    void *p_asi     = ioremap(BASE_ASI,         0xffffffff);
    void *p_vso     = ioremap(BASE_VSO,         0xffffffff);
    void *p_vsi     = ioremap(BASE_VSI,         0xffffffff);


    struct hdoip_eth_params eth_params;
    struct hdoip_aud_params aud_params;
    t_video_timing vid_timing;
    uint32_t err = 0;


    // apply reset signal for at least 100ms
    ext_reset_clear(p_reset, 0);
    schedule_timeout(HZ/10+1);
    ext_reset_set(p_reset, 0xf);
    schedule_timeout(1);

    // init
    spin_lock_init(&handle->sem);
    handle->event = queue_init(100);
    handle->response = queue_init(100);
    handle->p_irq = p_extirq;

    // setup components

    // init i2c with 400kHz
    i2c_drv_init(&handle->i2c_tx, p_tx, 400000);
    i2c_drv_init(&handle->i2c_rx, p_rx, 400000);

    //

    // vio init
    vio_drv_init(&handle->vio, p_vio, p_adv212);

    // setup basic font
    vio_drv_setup_osd(&handle->vio, &vid_font_8x13);

    // vrp init
    vrp_drv_init(&handle->vrp, &handle->vio, p_vrp);

    // aso init
    
    aud_params.ch_cnt_left = 1;
    aud_params.ch_cnt_right = 1;
    aud_params.fs = 44100;
    aud_params.sample_width = 24;

    eth_params.packet_length = 100;
    eth_params.dst_mac_upper16 = 0x0102;
    eth_params.dst_mac_lower32 = 0x03040506;
    eth_params.src_mac_upper16 = 0x0102;
    eth_params.src_mac_lower32 = 0x03040506;
    eth_params.ipv4_src_ip = 0xC0A8011e;
    eth_params.ipv4_dst_ip = 0xC0A80128;
    eth_params.ipv4_ttl = 20;
    eth_params.ipv4_tos = 20;
    eth_params.udp_src_port = 0x4000;
    eth_params.udp_dst_port = 0x5000;
    eth_params.rtp_ssrc = 0x12345678;

   
    // aso init 
    aso_drv_init(&handle->aso, p_aso);
    err = aso_drv_update(&handle->aso, &aud_params, 1000);

    if(err != ERR_ASO_SUCCESS) {
        REPORT(INFO, "aso error : %s", aso_str_err(err));
    }

    // vso init
    vso_drv_init(&handle->vso, p_vso, 1000000, &vid_timing, 1000, 1000000, 1000);
    if(err != ERR_VSO_SUCCESS) {
        REPORT(INFO, "vso error : %s", vso_str_err(err));
    }

    // asi init
    asi_drv_init(&handle->asi, p_asi);
    err = asi_drv_update(&handle->asi, &eth_params, &aud_params); 
    
    if(err != ERR_ASI_SUCCESS) {
        REPORT(INFO, "asi error : %s", asi_str_err(err));
    }

    // vsi init
    vsi_drv_init(&handle->vsi, p_vsi);
    vsi_drv_update(&handle->vsi, &eth_params); 

    if(err != ERR_VSI_SUCCESS) {
        REPORT(INFO, "vsi error : %s", vsi_str_err(err));
    }



    /* Init timer for handler function */
    init_timer(&handle->timer);
    handle->timer.expires = jiffies + HANDLER_TIMER_INTERVAL;
    handle->timer.function = hoi_drv_timer;
    handle->timer.data = handle;
    add_timer(&handle->timer);
}

/** This function force the driver into idle state
 *
 * @param handle hoi handle
 */
void hoi_drv_goto_idle(t_hoi* handle)
{
    vio_drv_reset(&handle->vio);
}

void hoi_drv_user_event(t_hoi* handle, uint32_t event)
{
    queue_put(handle->response, event);

    //kill(handle->owner, POLL_MSG);
}

void hoi_drv_interrupt(t_hoi* handle)
{
    uint32_t irq = HOI_RD32(handle->p_irq, 0);

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
    asi_drv_handler(&handle->asi, handle->event);
    aso_drv_handler(&handle->aso, handle->event);
}

void hoi_drv_fsm(t_hoi* handle, uint32_t event)
{
    REPORT(INFO, "event = %08x", event); // TODO: remove

    // global events
    switch (event) {
        case E_FORCE_TO_READY:
            switch (handle->state) {
                case HOI_IDLE:
                case HOI_READY:
                    hoi_drv_state(handle, HOI_READY);
                break;
                default:
                    hoi_drv_goto_idle(handle);
                    hoi_drv_force_state(handle, HOI_READY);
                break;
            }
        break;
    }

    // state machine
    switch (handle->state) {
        case HOI_IDLE:
            switch (event) {
                case E_FORCE_TO_READY:
                break;
            }
        break;
        case HOI_ERROR:
            switch (event) {
                case E_FORCE_TO_READY:
                break;
            }
        break;
        case HOI_READY:
            switch (event) {
                case E_FORCE_TO_READY:
                break;
            }
        break;
        case HOI_SHOW:
            switch (event) {
                case E_FORCE_TO_READY:
                break;
            }
        break;
    }
}

static int hoi_drv_timer(unsigned long data)
{
    uint32_t event;
    t_hoi* handle = data;

    hoi_drv_lock(handle);

    // interrupt
    hoi_drv_interrupt(handle);

    // handler
    hoi_drv_handler(handle);

    // FSM
    while ((event = queue_get(handle->event))) {
        hoi_drv_fsm(handle, event);
    }

    // signal update

    hoi_drv_unlock(handle);

    handle->timer.expires = jiffies + HANDLER_TIMER_INTERVAL;
    add_timer(&handle->timer);
}
