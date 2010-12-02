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
#include "adv212_drv.h"
#include "vio_drv.h"
#include "vsi_drv.h"
#include "vso_drv.h"
#include "vrp_drv.h"
#include "tag_drv.h"


#define HANDLER_TIMER_INTERVAL      (HZ/20)


typedef struct {
    void                *p_irq;
    void                *p_reset;
    void                *p_tx;
    void                *p_rx;
    void                *p_vio;
    void                *p_eso;
    void                *p_vsi;
    void                *p_asi;
    void                *p_esi;
    void                *p_vso;
    void                *p_aso;
    void                *p_adv212;
    void                *p_vrp;

    t_i2c               i2c_tx;
    t_i2c               i2c_rx;

    uint32_t            drivers;
    t_tag               vtag;
    t_tag               atag;
    t_vio               vio;
    t_vrp               vrp;
    t_vsi               vsi;
    t_vso               vso;
    t_adv9889           adv9889;
    t_adv7441a          adv7441a;

    uint32_t            state;
    t_queue             *event;
    t_queue             *response;
    pid_t               owner;
    spinlock_t          sem;            //!<
    struct timer_list   timer;
} t_hoi;


static inline void hoi_drv_lock(t_hoi* handle)
{
    spin_lock(&handle->sem);
}

static inline void hoi_drv_unlock(t_hoi* handle)
{
    spin_unlock(&handle->sem);
}

static inline void hoi_drv_state(t_hoi* handle, uint32_t state)
{
    handle->state = state;
}

static inline void hoi_drv_force_state(t_hoi* handle, uint32_t state)
{
    handle->state = state;
    queue_flush(handle->event);
}

void hoi_drv_init(t_hoi* handle);
void hoi_drv_stop(t_hoi* handle);
int hoi_drv_message(t_hoi* handle, t_hoi_msg* msg);


#endif /* HOI_DRV_H_ */
