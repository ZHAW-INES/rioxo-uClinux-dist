/*
 * hoi_drv.h
 * central driver
 *
 *  Created on: 08.10.2010
 *      Author: alda
 */

#ifndef HOI_DRV_H_
#define HOI_DRV_H_

#include "std.h"
#include "queue.h"
#include "hoi_msg.h"

#include "i2c_drv.h"
#include "adv7441a_drv.h"
#include "adv9889_drv.h"
#include "gs2971_drv.h"
#include "gs2972_drv.h"
#include "adv212_drv.h"
#include "led_drv.h"
#include "eti_drv.h"
#include "eto_drv.h"
#include "vio_drv.h"
#include "vsi_drv.h"
#include "vso_drv.h"
#include "asi_drv.h"
#include "aso_drv.h"
#include "vrp_drv.h"
#include "hdcp_drv.h"
#include "tag_drv.h"
#include "tmr_hal.h"
#include "ver_hal.h"
#include "stream_sync.h"
#include "bdt_drv.h"
#include "spi_drv.h"


#define HANDLER_TIMER_INTERVAL      (HZ/20)


typedef struct {
    void                *p_irq;
    void                *p_tx;
    void                *p_rx;
    void                *p_i2c_tag_aud;
    void                *p_i2c_tag_vid;
    void                *p_vio;
    void                *p_eso;
    void                *p_vsi;
    void                *p_asi;
    void                *p_esi;
    void                *p_vso;
    void                *p_aso;
    void                *p_adv212;
    void                *p_vrp;
    void                *p_tmr;
    void                *p_wdg;
    void                *p_hdcp;
    void                *p_ver;
    void                *p_sysid;
    void                *p_led;
    void                *p_video_mux;
    void                *p_spi_tx;
    void                *p_spi_rx;

    t_i2c               i2c_tx;
    t_i2c               i2c_rx;
    t_i2c               i2c_tag_aud;
    t_i2c               i2c_tag_vid;

    uint32_t            drivers;
    t_led               led;
    t_tag               vtag;
    t_tag               atag;
    t_vio               vio;
    t_vrp               vrp;
    t_vsi               vsi;
    t_vso               vso;
    t_asi               asi;
    t_aso               aso;
    t_eto               eto;
    t_eti               eti;
    //t_hdcp				hdcp;
    t_adv9889           adv9889;
    t_adv7441a          adv7441a;
    t_gs2971            gs2971;
    t_gs2972            gs2972;
    t_sync_means        sync;
    t_bdt               bdt;

    t_queue             *event;
    wait_queue_head_t   eq;
} t_hoi;


void hoi_drv_init(t_hoi* handle);
void hoi_drv_stop(t_hoi* handle);
void hoi_drv_timer(t_hoi* handle);
void hoi_drv_reset(t_hoi* handle, uint32_t rv);
int hoi_drv_message(t_hoi* handle, t_hoi_msg* msg);


#endif /* HOI_DRV_H_ */
