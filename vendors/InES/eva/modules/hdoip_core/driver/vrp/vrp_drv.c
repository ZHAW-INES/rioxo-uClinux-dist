/*
 * vrp_drv.c
 *
 *  Created on: 04.10.2010
 *      Author: alda
 */
#include "vrp_drv.h"
#include "vio_str.h"

void vrp_drv_init(t_vrp* handle, t_vio* vio, void* p_vrp)
{
    handle->vio = vio;
    handle->p_vrp = p_vrp;
}

int vrp_drv_off(t_vrp* handle)
{
    vrp_stop(handle->p_vrp);
    return SUCCESS;
}


int vrp_drv_capture_image(t_vrp* handle, void* buffer, size_t size)
{
    uint32_t jiffies_out;
    vrp_idle(handle->p_vrp);
    vrp_set_buffer(handle->p_vrp, buffer, size-4);
    vrp_do_record(handle->p_vrp, 1);
    vio_drv_set_cfg(handle->vio, VIO_CONFIG_VRP);
    vio_drv_plainin(handle->vio);
    jiffies_out = jiffies + HZ;
    while(!vrp_get_idle(handle->p_vrp) && !TIMEOUT(jiffies_out)) {
        schedule_timeout(HZ/20+1);
    }
    if (!vrp_get_idle(handle->p_vrp)) {
        REPORT(WARNING, "capture timeout");
    }
    return SUCCESS;
}

int vrp_drv_show_image(t_vrp* handle, void* buffer, t_video_timing* p_vt)
{
    vrp_idle(handle->p_vrp);
    vio_drv_set_cfg(handle->vio, VIO_CONFIG_VRP);
    vio_drv_plainoutx(handle->vio, p_vt);
    vrp_set_buffer(handle->p_vrp, buffer, p_vt->width*p_vt->height*3+4);
    vrp_do_playback(handle->p_vrp, 1, true);
    return SUCCESS;
}


int vrp_drv_capture_jpeg2000(t_vrp* handle, void* buffer, size_t size, size_t bandwidth)
{
    uint32_t jiffies_out;
    vrp_idle(handle->p_vrp);
    vrp_set_buffer(handle->p_vrp, buffer, size-4);
    vrp_do_record(handle->p_vrp, 1);
    vio_drv_set_cfg(handle->vio, VIO_CONFIG_VRP);
    vio_drv_encodex(handle->vio, bandwidth, 0);
    // wait for capture
    while(!vrp_get_idle(handle->p_vrp) && !TIMEOUT(jiffies_out)) {
        schedule_timeout(HZ/20+1);
    }
    if (!vrp_get_idle(handle->p_vrp)) {
        REPORT(WARNING, "capture timeout");
    }
    return SUCCESS;
}

int vrp_drv_show_jpeg2000(t_vrp* handle, void* buffer, t_video_timing* p_vt, int advcnt)
{
    int ret;
    vrp_idle(handle->p_vrp);
    vio_drv_set_cfg(handle->vio, VIO_CONFIG_VRP);
    if (ret = vio_drv_decodex(handle->vio, p_vt, advcnt)) {
        VIO_REPORT_ERROR(ret);
        return ret;
    }
    vrp_set_buffer(handle->p_vrp, buffer, ((uint32_t*)buffer)[0]*4+4);
    vrp_do_playback(handle->p_vrp, 1, true);
    return SUCCESS;
}

