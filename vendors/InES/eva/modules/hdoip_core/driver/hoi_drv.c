/*
 * hoi_drv.c
 *
 *  Created on: 08.10.2010
 *      Author: alda
 */
#include "hoi_drv.h"
#include "vid_const.h"
#include "ext_irq_pio.h"

// TODO: remove:
#include "hdoip.h"

// timer prototype
static int hoi_drv_timer(unsigned long data);


/* base driver initialization
 *
 * @param handle hoi driver handle
 */
void hoi_drv_init(t_hoi* handle)
{
    // TODO proper range in config.h
    handle->p_tx      = ioremap(BASE_VIDEO_TX,    0xffffffff);
    handle->p_rx      = ioremap(BASE_VIDEO_RX,    0xffffffff);
    handle->p_vio     = ioremap(BASE_VIO,         0xffffffff);
    handle->p_vsi     = ioremap(BASE_VSI,         0xffffffff);
    handle->p_vso     = ioremap(BASE_VSO,         0xffffffff);
    handle->p_adv212  = ioremap(BASE_ADV212,      0xffffffff);
    handle->p_vrp     = ioremap(BASE_VRP,         0xffffffff);
    handle->p_reset   = ioremap(BASE_RESET,       0xffffffff);
    handle->p_irq     = ioremap(BASE_EXT_IRQ,     0xffffffff);
    // TODO: proper ethernet
    //handle->p_esi     = ioremap(BASE_ESI,         0xffffffff);
    //handle->p_eso     = ioremap(BASE_ESO,         0xffffffff);
    handle->p_esi     = ioremap(ACB_ETH_IN_BASE,  0xffffffff);
    handle->p_eso     = ioremap(ACB_ETH_OUT_BASE, 0xffffffff);

    // apply reset signal for at least 100ms
    ext_reset_clear(handle->p_reset, 0);
    schedule_timeout(HZ/10+1);
    ext_reset_set(handle->p_reset, 0xf);
    schedule_timeout(1);

    // init
    spin_lock_init(&handle->sem);
    handle->event = queue_init(100);
    handle->response = queue_init(100);

    // setup components

    // init i2c with 400kHz
    i2c_drv_init(&handle->i2c_tx, handle->p_tx, 400000);
    i2c_drv_init(&handle->i2c_rx, handle->p_rx, 400000);

    // init
    vio_drv_init(&handle->vio, handle->p_vio, handle->p_adv212);
    vsi_drv_init(&handle->vsi, handle->p_vsi);
    vso_drv_init(&handle->vso, handle->p_vso);
    vio_drv_setup_osd(&handle->vio, &vid_font_8x13);
    vrp_drv_init(&handle->vrp, &handle->vio, handle->p_vrp);

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
void hoi_drv_stop(t_hoi* handle)
{
    vso_drv_stop(&handle->vso);
    vsi_drv_stop(&handle->vso);
    vrp_drv_off(&handle->vrp);
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

    irq = irq ^ EXT_IRQ_POLARITY;

    if (irq & EXT_IRQ_HDMI_RX_INT1) {
        adv7441a_irq1_handler(&handle->adv7441a, handle->event);
    }
    if (irq & EXT_IRQ_HDMI_RX_INT2) {
        adv7441a_irq2_handler(&handle->adv7441a, handle->event);
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
}

void hoi_drv_fsm(t_hoi* handle, uint32_t event)
{
    REPORT(INFO, "event = %08x", event); // TODO: remove

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
