/*
 * hoi_msg.c
 *
 *  Created on: 08.10.2010
 *      Author: alda
 */

#include "std.h"
#include "hoi_drv.h"

//#include "adv7441a_drv_edid.h" // TODO no edid for init

// TODO: remove:
#include "hdoip.h"

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
/*    if (lddrv & DRV_ADV7441) {
        adv7441a_drv_init(&handle->adv7441a, &handle->i2c_rx, adv7441a_edid_table);
    }*/

    handle->drivers = msg->drivers & DRV_ALL;
    handle->owner = msg->pid;

    queue_put(handle->event, E_FORCE_TO_READY);

    return SUCCESS;
}


int hoi_drv_msg_buf(t_hoi* handle, t_hoi_msg_buf* msg)
{
    t_rbf_dsc tmp;

    // TODO: proper ethernet

    uint32_t ci = HOI_RD32(handle->p_esi, 0);
    uint32_t co = HOI_RD32(handle->p_eso, 0);
    HOI_WR32(handle->p_esi, 0, ci&~1);
    HOI_WR32(handle->p_eso, 0, co&~1);

    if (msg->vid_rx_buf) {
        rbf_dsc(&tmp, msg->vid_rx_buf, msg->vid_rx_len - MAX_FRAME_LENGTH);
        rbf_set_dsc_wc(handle->p_esi, ETHIO_VID_START_DESC, &tmp);
        rbf_set_dsc_r(handle->p_vso, VSO_OFF_DSC_START_RO, &tmp);
    }

    if (msg->aud_rx_buf) {
        rbf_dsc(&tmp, msg->aud_rx_buf, msg->aud_rx_len - MAX_FRAME_LENGTH);
        rbf_set_dsc_wc(handle->p_esi, ETHIO_AUD_START_DESC, &tmp);
        rbf_set_dsc_r(handle->p_aso, ASO_OFF_DSC_START_RO, &tmp);
    }

    if (msg->vid_tx_buf) {
        rbf_dsc(&tmp, msg->vid_tx_buf, msg->vid_tx_len - MAX_FRAME_LENGTH);
        rbf_set_dsc_rc(handle->p_eso, ETHIO_VID_START_DESC, &tmp);
        rbf_set_dsc_w(handle->p_vsi, VSI_OFF_DSC_START_RD, &tmp);
    }

    if (msg->aud_tx_buf) {
        rbf_dsc(&tmp, msg->aud_tx_buf, msg->aud_tx_len - MAX_FRAME_LENGTH);
        rbf_set_dsc_rc(handle->p_eso, ETHIO_AUD_START_DESC, &tmp);
        rbf_set_dsc_w(handle->p_asi, ASI_OFF_DSC_START_RO, &tmp);
    }

    REPORT(INFO, " --- ESI:  CPU  ---");
    rbf_report_dsc(handle->p_esi, ETHIO_CPU_START_DESC);
    REPORT(INFO, " --- ESI: Audio ---");
    rbf_report_dsc(handle->p_esi, ETHIO_AUD_START_DESC);
    REPORT(INFO, " --- ESI: Video ---");
    rbf_report_dsc(handle->p_esi, ETHIO_VID_START_DESC);
    REPORT(INFO, " --- ESO:  CPU  ---");
    rbf_report_dsc(handle->p_eso, ETHIO_CPU_START_DESC);
    REPORT(INFO, " --- ESO: Audio ---");
    rbf_report_dsc(handle->p_eso, ETHIO_AUD_START_DESC);
    REPORT(INFO, " --- ESO: Video ---");
    rbf_report_dsc(handle->p_eso, ETHIO_VID_START_DESC);

    HOI_WR32(handle->p_esi, 0, ci);
    HOI_WR32(handle->p_eso, 0, co);


    return SUCCESS;
}


int hoi_drv_msg_eti(t_hoi* handle, t_hoi_msg_eti* msg)
{
    // setup filter
    HOI_WR32(handle->p_esi, ETHI_IP_FILTER, swap32(msg->ip_address_dst));
    HOI_WR32(handle->p_esi, ETHI_IP_FILTER_SRC, swap32(msg->ip_address_src));
    HOI_WR32(handle->p_esi, ETHI_VID_FILTER, swap16(msg->udp_port_vid));
    HOI_WR32(handle->p_esi, ETHI_AUD_FILTER, swap16(msg->udp_port_aud));

    return SUCCESS;
}

//------------------------------------------------------------------------------
// Stream I/O

int hoi_drv_msg_vsi(t_hoi* handle, t_hoi_msg_vsi* msg)
{
    hoi_drv_stop(handle);

    // setup vsi
    vsi_drv_go(&handle->vsi, &msg->eth);

    // setup vio
    if (msg->compress) {
        vio_drv_encodex(&handle->vio, msg->bandwidth, 0);
    } else {
        vio_drv_plainin(&handle->vio);
    }

    vio_drv_get_advcnt(&handle->vio, &msg->advcnt);
    vio_drv_get_timing(&handle->vio, &msg->timing);

    return SUCCESS;
}


int hoi_drv_msg_vso(t_hoi* handle, t_hoi_msg_vso* msg)
{
    hoi_drv_stop(handle);

    // setup vio
    if (msg->compress) {
        vio_drv_decodex(&handle->vio, &msg->timing, msg->advcnt);
    } else {
        vio_drv_plainout(&handle->vio);
    }

    // setup vso (100ms delay, 80ms scomm5 delay, 1ms packet timeout)
    vso_drv_update(&handle->vso, &msg->timing, 100000, 0, 80000000, 1000000);
    vso_drv_start(&handle->vso);
    vso_report_timing(&handle->vso);

    return SUCCESS;
}


//------------------------------------------------------------------------------
// Image capture / show

int hoi_drv_msg_capture(t_hoi* handle, t_hoi_msg_image* msg)
{
    uint32_t ret = SUCCESS;

    hoi_drv_stop(handle);

    // TODO: access userspace buffer properly
    // TODO: proper bandwidth/size

    if (msg->compress) {
        ret = vrp_drv_capture_jpeg2000(&handle->vrp, msg->buffer, msg->size, msg->size * 3 / 4);
    } else {
        ret = vrp_drv_capture_image(&handle->vrp, msg->buffer, msg->size);
    }

    vio_drv_get_timing(&handle->vio, &msg->timing);
    vio_drv_get_advcnt(&handle->vio, &msg->advcnt);

    return ret;
}

int hoi_drv_msg_show(t_hoi* handle, t_hoi_msg_image* msg)
{
    uint32_t ret = SUCCESS;

    hoi_drv_stop(handle);

    if (msg->compress) {
        ret = vrp_drv_show_jpeg2000(&handle->vrp, msg->buffer, &msg->timing, msg->advcnt);
    } else {
        ret = vrp_drv_show_image(&handle->vrp, msg->buffer, &msg->timing);
    }

    return ret;
}


//------------------------------------------------------------------------------
// Command

int hoi_drv_msg_loop(t_hoi* handle)
{
    hoi_drv_stop(handle);

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

        case HOI_MSG_VSI: ret = hoi_drv_msg_vsi(handle, msg); break;
        case HOI_MSG_VSO: ret = hoi_drv_msg_vso(handle, msg); break;

        // Set Param
        case HOI_MSG_BUF: ret = hoi_drv_msg_buf(handle, msg); break;
        case HOI_MSG_IFMT: ret = hoi_drv_msg_ifmt(handle, msg); break;
        case HOI_MSG_OFMT: ret = hoi_drv_msg_ofmt(handle, msg); break;
        case HOI_MSG_PFMT: ret = hoi_drv_msg_pfmt(handle, msg); break;
        case HOI_MSG_GETEVENT: ret = hoi_drv_msg_getevent(handle, msg); break;
        case HOI_MSG_GETSTATE: ret = hoi_drv_msg_getstate(handle, msg); break;

        case HOI_MSG_LOOP: ret = hoi_drv_msg_loop(handle); break;
        case HOI_MSG_OSDON: ret = hoi_drv_msg_osdon(handle); break;
        case HOI_MSG_OSDOFF: ret = hoi_drv_msg_osdoff(handle); break;

        case HOI_MSG_GETMTIME: ret = hoi_drv_msg_get_mtime(handle, msg); break;
        case HOI_MSG_SETMTIME: ret = hoi_drv_msg_set_mtime(handle, msg); break;

        default: ret = ERR_HOI_CMD_NOT_SUPPORTED; break;
    }

    hoi_drv_unlock(handle);

    return ret;
}
