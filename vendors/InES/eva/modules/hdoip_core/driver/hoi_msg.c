/*
 * hoi_msg.c
 *
 *  Created on: 08.10.2010
 *      Author: alda
 */

#include "std.h"
#include "hoi_drv.h"

#include "adv7441a_drv_edid.h" // TODO no edid for init

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

    // install driver...
    if (lddrv & DRV_ADV9889) {
        adv9889_drv_init(&handle->adv9889, &handle->i2c_tx);
    }
    if (lddrv & DRV_ADV7441) {
        adv7441a_drv_init(&handle->adv7441a, &handle->i2c_rx, adv7441a_edid_table);
    }

    handle->drivers = msg->drivers & DRV_ALL;
    handle->owner = msg->pid;

    queue_put(handle->event, E_FORCE_TO_READY);

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
        ret = vrp_drv_capture_jpeg2000(&handle->vrp, msg->buffer, msg->size, msg->size * 3 / 4);
    } else {
        ret = vrp_drv_capture_image(&handle->vrp, msg->buffer, msg->size);
    }

    vio_drv_get_timing(&handle->vio, &msg->timing);
    vio_drv_get_advcnt(&handle->vio, &msg->advcnt);

    // capture is finished -> now ready
    hoi_drv_force_state(handle, HOI_READY);

    return ret;
}

int hoi_drv_msg_show(t_hoi* handle, t_hoi_msg_image* msg)
{
    uint32_t ret = SUCCESS;

    REPORT(INFO, "hoi_drv_msg_show");

    // TODO: access userspace buffer properly

    if (msg->compress) {
        ret = vrp_drv_show_jpeg2000(&handle->vrp, msg->buffer, &msg->timing, msg->advcnt);
    } else {
        ret = vrp_drv_show_image(&handle->vrp, msg->buffer, &msg->timing);
    }

    hoi_drv_force_state(handle, HOI_SHOW);

    return ret;
}


//------------------------------------------------------------------------------
// Command

int hoi_drv_msg_loop(t_hoi* handle)
{
    vio_drv_loop(&handle->vio);
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


//------------------------------------------------------------------------------
// info

int hoi_drv_msg_info(t_hoi* handle, t_hoi_msg_info* msg)
{
    vio_drv_get_timing(&handle->vio, &msg->timing);
    return SUCCESS;
}


//------------------------------------------------------------------------------
// param read/write

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

int hoi_drv_msg_getevent(t_hoi* handle, t_hoi_msg_param* msg)
{
    msg->value = queue_get(handle->response);
    return SUCCESS;
}

int hoi_drv_msg_getstate(t_hoi* handle, t_hoi_msg_param* msg)
{
    msg->value = handle->state;
    return SUCCESS;
}



//------------------------------------------------------------------------------
// message

int hoi_drv_message(t_hoi* handle, t_hoi_msg* msg)
{
    uint32_t ret;
    hoi_drv_lock(handle);

    switch (msg->id) {
        case HOI_MSG_LDRV: ret = hoi_drv_msg_ldrv(handle, msg); break;
        case HOI_MSG_CAPTURE: ret = hoi_drv_msg_capture(handle, msg); break;
        case HOI_MSG_SHOW: ret = hoi_drv_msg_show(handle, msg); break;
        case HOI_MSG_RDVIDTAG: ret = hoi_drv_msg_rdvidtag(handle, msg); break;
        case HOI_MSG_RDAUDTAG: ret = hoi_drv_msg_rdaudtag(handle, msg); break;
        case HOI_MSG_WRVIDTAG: ret = hoi_drv_msg_wrvidtag(handle, msg); break;
        case HOI_MSG_WRAUDTAG: ret = hoi_drv_msg_wraudtag(handle, msg); break;
        case HOI_MSG_INFO: ret = hoi_drv_msg_info(handle, msg); break;

        // Set Param
        case HOI_MSG_IFMT: ret = hoi_drv_msg_ifmt(handle, msg); break;
        case HOI_MSG_OFMT: ret = hoi_drv_msg_ofmt(handle, msg); break;
        case HOI_MSG_PFMT: ret = hoi_drv_msg_pfmt(handle, msg); break;
        case HOI_MSG_GETEVENT: ret = hoi_drv_msg_getevent(handle, msg); break;
        case HOI_MSG_GETSTATE: ret = hoi_drv_msg_getstate(handle, msg); break;

        case HOI_MSG_LOOP: ret = hoi_drv_msg_loop(handle); break;
        case HOI_MSG_OSDON: ret = hoi_drv_msg_osdon(handle); break;
        case HOI_MSG_OSDOFF: ret = hoi_drv_msg_osdoff(handle); break;
        default: ret = ERR_HOI_CMD_NOT_SUPPORTED; break;
    }

    hoi_drv_unlock(handle);

    return ret;
}
