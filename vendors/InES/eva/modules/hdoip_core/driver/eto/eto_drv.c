
#include "eto_drv.h"


/** Start FSM and CPU ringbuffer
 *
 */
int eto_drv_start(t_eto* handle)
{
	eto_set_config_cpu_frame_enable(handle->ptr);
	eto_set_config_start(handle->ptr);	
	return ERR_ETO_SUCCESS;
}

/** Stop FSM and CPU ringbuffer
 *
 */
int eto_drv_stop(t_eto* handle)
{   
	eto_set_config_cpu_frame_disable(handle->ptr);
	eto_set_config_stop(handle->ptr);
    while (!eto_get_status_fsm_idle(handle->ptr));
	while(!(eto_get_status_reg(handle->ptr) & ETO_STATUS_FSM_IDLE));

	return ERR_ETO_SUCCESS;
}

/** Starts audio ringbuffer 
 *
 *  @param handle pointer to the ethernet stream out handle
 *  @return error code
 */
int eto_drv_start_aud(t_eto* handle)
{
    eto_set_config_aud_frame_enable(handle->ptr);
    return ERR_ETO_SUCCESS;
}

/** Stops audio ringbuffer 
 *
 *  @param handle pointer to the ethernet stream out handle
 *  @return error code
 */
int eto_drv_stop_aud(t_eto* handle)
{
    eto_set_config_aud_frame_disable(handle->ptr);
    while (!eto_get_status_aud_idle(handle->ptr));
    eto_clr_status_aud_idle(handle->ptr);
    return ERR_ETO_SUCCESS;
}

/** Starts video ringbuffer 
 *
 *  @param handle pointer to the ethernet stream out handle
 *  @return error code
 */
int eto_drv_start_vid(t_eto* handle)
{
    eto_set_config_vid_frame_enable(handle->ptr);
    return ERR_ETO_SUCCESS;
}

/** Stops video ringbuffer 
 *
 *  @param handle pointer to the ethernet stream out handle
 *  @return error code
 */
int eto_drv_stop_vid(t_eto* handle)
{
    eto_set_config_vid_frame_disable(handle->ptr);
    while (!eto_get_status_vid_idle(handle->ptr));
    eto_clr_status_vid_idle(handle->ptr);
    return ERR_ETO_SUCCESS;
}

/** Enables audio aes encryption 
 *
 * @param handle pointer to the eto handle
 * @return error code
 */
int eto_drv_aes_aud_en(t_eto* handle)
{
    eto_set_config_audio_enc_en(handle->ptr);
    return ERR_ETO_SUCCESS;
}

/** Disables audio aes encryption 
 *
 * @param handle pointer to the eto handle
 * @return error code
 */
int eto_drv_aes_aud_dis(t_eto* handle)
{
    eto_set_config_audio_enc_dis(handle->ptr);
    return ERR_ETO_SUCCESS;
}

/** Get status of audio and video aes encryption (on/off?) 
 *
 * @param handle pointer to the eto handle
 * @return error code
 */
int eto_drv_aes_stat(t_eto* handle, uint32_t *audio_enc_en, uint32_t *video_enc_en)
{
    *audio_enc_en = eto_get_config_audio_enc_en(handle->ptr);
    *video_enc_en = eto_get_config_video_enc_en(handle->ptr);
    //*audio_enc_en = 12; //only to test if path up to userspace works
    //*video_enc_en = 34;
    return ERR_ETI_SUCCESS;
}

/** Enables video aes encryption 
 *
 * @param handle pointer to the eto handle
 * @return error code
 */
int eto_drv_aes_vid_en(t_eto* handle)
{
    eto_set_config_video_enc_en(handle->ptr);
    return ERR_ETO_SUCCESS;
}

/** Disables video aes encryption 
 *
 * @param handle pointer to the eto handle
 * @return error code
 */
int eto_drv_aes_vid_dis(t_eto* handle)
{
    eto_set_config_video_enc_dis(handle->ptr);
    return ERR_ETO_SUCCESS;
}

/** Sets encryption parameters
 *
 * @param handle pointer to eth eto handle
 * @param key array of 4 keys
 * @param riv array of 2 rivs
 * @return error code
 */
int eto_drv_set_aes(t_eto* handle, uint32_t key[4], uint32_t riv[2])
{
	eto_set_aes_session_key_0(handle->ptr, key[0]);
	eto_set_aes_session_key_1(handle->ptr, key[1]);
	eto_set_aes_session_key_2(handle->ptr, key[2]);
	eto_set_aes_session_key_3(handle->ptr, key[3]);
	eto_set_aes_riv_0(handle->ptr, riv[0]);
	eto_set_aes_riv_1(handle->ptr, riv[1]);

	return ERR_ETO_SUCCESS;
}

/** Reads encryption parameters
 *
 * @param handle pointer to eth eto handle
 * @param key array of 4 keys
 * @param riv array of 2 rivs
 * @return error code
 */
int eto_drv_get_aes(t_eto* handle, uint32_t key[4], uint32_t riv[2])
{
    key[0] = eto_get_aes_session_key_0(handle->ptr);
    key[1] = eto_get_aes_session_key_1(handle->ptr);
    key[2] = eto_get_aes_session_key_2(handle->ptr);
    key[3] = eto_get_aes_session_key_3(handle->ptr);
    riv[0] = eto_get_aes_riv_0(handle->ptr);
    riv[1] = eto_get_aes_riv_1(handle->ptr);

    return ERR_ETO_SUCCESS;
}

/** Initialize CPU ringbuffer 
 *
 *  @param handle pointer to the ethernet stream out handle
 *  @param start_ptr start address of the buffer
 *  @param size size of the buffer
 *  @return error code
 */
int eto_drv_set_cpu_buf(t_eto* handle, void* start_ptr, size_t size)
{
    t_rbf_dsc dsc;
    
    rbf_dsc(&dsc, start_ptr, size);
    eto_set_cpu_desc(handle->ptr, &dsc);

    return ERR_ETO_SUCCESS;
}

/** Initialize audio ringbuffer 
 *
 *  @param handle pointer to the ethernet stream out handle
 *  @param start_ptr start address of the buffer
 *  @param size size of the buffer
 *  @return error code
 */
int eto_drv_set_aud_buf(t_eto* handle, void* start_ptr, size_t size)
{
    t_rbf_dsc dsc;

    eto_drv_stop_aud(handle);
    
    rbf_dsc(&dsc, start_ptr, size);
    eto_set_aud_desc(handle->ptr, &dsc);

    return ERR_ETO_SUCCESS;
}

/** Initialize video ringbuffer 
 *
 *  @param handle pointer to the ethernet stream out handle
 *  @param start_ptr start address of the buffer
 *  @param size size of the buffer
 *  @return error code
 */
int eto_drv_set_vid_buf(t_eto* handle, void* start_ptr, size_t size)
{
    t_rbf_dsc dsc;

    eto_drv_stop_vid(handle);

    rbf_dsc(&dsc, start_ptr, size);
    eto_set_vid_desc(handle->ptr, &dsc);

    return ERR_ETO_SUCCESS;
}

int eto_drv_set_ptr(t_eto* handle, void* ptr)
{
    handle->ptr = ptr;
    handle->link_state = 0;
    return ERR_ETO_SUCCESS;
}

/** Initialize ETO core driver
 *
 */
int eto_drv_init(t_eto* handle, void* ptr)
{
	REPORT(INFO, "+--------------------------------------------------+");
	REPORT(INFO, "| ETO-Driver: Initialize ethernet stream out       |");
	REPORT(INFO, "+--------------------------------------------------+");

    handle->ptr = ptr;
    handle->link_state = 0;
    handle->vtx = 0;
    handle->atx = 0;
    handle->abtx = 0;
    handle->vcnt = 0;
    handle->acnt = 0;
    handle->abcnt = 0;
    handle->traffic_shaping_enable = 0;

	eto_drv_stop(handle);
    eto_set_config_cpu_frame_disable(handle->ptr);

    eto_set_config_audio_enc_dis(handle->ptr);
    eto_set_config_video_enc_dis(handle->ptr);

	eto_set_burst_size_reg(handle->ptr, ETO_DMA_BURST_SIZE);
	eto_set_dma_almost_full(handle->ptr, ETO_DMA_ALMOST_FULL);

    eto_prio_init(handle->ptr, ETO_DRV_PRIO_SHIFT_VALUE, ETO_DRV_PRIO_CLOCK_DIVISION_FACTOR);
    eto_set_prio_cpu_scale(handle->ptr, ETO_DRV_PRIO_CPU_SCALE);
    eto_set_prio_aud_scale(handle->ptr, ETO_DRV_PRIO_AUD_SCALE);
    eto_set_prio_vid_scale(handle->ptr, ETO_DRV_PRIO_VID_SCALE);
    eto_set_prio_cpu_load(handle->ptr, 0);
    eto_set_prio_aud_load(handle->ptr, 0);
    eto_set_prio_vid_load(handle->ptr, 0);

    REPORT(INFO, "config reg : %08x", eto_get_config_reg(handle->ptr));
    REPORT(INFO, "status reg : %08x", eto_get_status_reg(handle->ptr));
    REPORT(INFO, "burst size : %d", eto_get_burst_size_reg(handle->ptr));
    REPORT(INFO, "almost full: %d", eto_get_dma_almost_full(handle->ptr));

	return ERR_ETO_SUCCESS;
}


int eto_drv_handler(t_eto* handle, t_queue* event_queue)
{
    uint32_t reg = eto_get_config_reg(handle->ptr) & ETO_CONFIG_FSM_EN;
    uint32_t vtx = eto_get_vid_packet_cnt(handle->ptr);
    uint32_t atx = eto_get_aud_emb_packet_cnt(handle->ptr);
    uint32_t abtx = eto_get_aud_board_packet_cnt(handle->ptr);

    if(reg != handle->link_state) {
        if(reg > 0) {   /* link up */
            queue_put(event_queue, E_ETO_LINK_UP);     
        } else {        /* link down */
            queue_put(event_queue, E_ETO_LINK_DOWN);     
        }
        handle->link_state = reg;
    }

    if (vtx != handle->vtx) {
        handle->vtx = vtx;
        if (!handle->vcnt) {
            queue_put(event_queue, E_ETO_VIDEO_ON);
        }
        handle->vcnt = ETO_LINK_COUNT;
    } else {
        if (handle->vcnt == 1) {
            queue_put(event_queue, E_ETO_VIDEO_OFF);
        }
        if (handle->vcnt) {
            handle->vcnt--;
        }
    }

    if (atx != handle->atx) {
        handle->atx = atx;
        if (!handle->acnt) {
            queue_put(event_queue, E_ETO_AUDIO_EMB_ON);
        }
        handle->acnt = ETO_LINK_COUNT;
    } else {
        if (handle->acnt == 1) {
            queue_put(event_queue, E_ETO_AUDIO_EMB_OFF);
        }
        if (handle->acnt) {
            handle->acnt--;
        }
    }

    if (abtx != handle->abtx) {
        handle->abtx = abtx;
        if (!handle->abcnt) {
            queue_put(event_queue, E_ETO_AUDIO_BOARD_ON);
        }
        handle->abcnt = ETO_LINK_COUNT;
    } else {
        if (handle->abcnt == 1) {
            queue_put(event_queue, E_ETO_AUDIO_BOARD_OFF);
        }
        if (handle->abcnt) {
            handle->abcnt--;
        }
    }

    return ERR_ETO_SUCCESS;
}

void eto_drv_frame_rate_reduction(t_eto* handle, int reduction)
{
    switch (reduction) {
        case 1:     eto_set_config_reduce_fps_0_ON(handle->ptr);
                    eto_set_config_reduce_fps_1_OFF(handle->ptr);
                    break;
        case 2:     eto_set_config_reduce_fps_0_OFF(handle->ptr);
                    eto_set_config_reduce_fps_1_ON(handle->ptr);
                    break;
        case 3:     eto_set_config_reduce_fps_0_ON(handle->ptr);
                    eto_set_config_reduce_fps_1_ON(handle->ptr);
                    break;
        default:    eto_set_config_reduce_fps_0_OFF(handle->ptr);
                    eto_set_config_reduce_fps_1_OFF(handle->ptr);
    }
}

void eto_drv_set_frame_period(t_eto* handle, t_video_timing* timing, t_fec_setting* fec, int enable)
{
    uint32_t h;
    uint32_t v;
    uint32_t period_10ns;
    uint32_t packet_divider;
    uint32_t packet_multiplier;

    uint32_t l = fec->video_l;
    uint32_t d = fec->video_d;
    handle->traffic_shaping_enable = enable;

    h = (timing->width + timing->hfront + timing->hpulse + timing->hback);
    v = (timing->height + timing->vfront + timing->vpulse + timing->vback);
    period_10ns = (uint32_t)(((uint64_t)100000000*h*v)/timing->pfreq);

    // calculate overhead packets due to FEC
    if (fec->video_enable) {
        if (fec->video_col_only) {
            packet_divider = d;
            packet_multiplier = 1;
        } else {
            packet_divider = l * d;
            packet_multiplier = l + d;
        }
    } else {
        packet_divider = 1;
        packet_multiplier = 0;
    }

    // reduce the fraction
    if ((packet_divider % packet_multiplier) == 0) {
        packet_divider = packet_divider / packet_multiplier;
        packet_multiplier = 1;
    }
    if ((packet_divider % (packet_multiplier / 2)) == 0) {
        packet_divider = packet_divider / (packet_multiplier / 2);
        packet_multiplier = 2;
    }

    // set correction factor to modify count of packets
    eto_set_tf_divider_10ns(handle->ptr, packet_divider);
    eto_set_tf_multiplier_10ns(handle->ptr, packet_multiplier);

    // period_10ns must not be longer than real refresh rate of video.
    // There can be a measurement error of 5% and it still works
    period_10ns -= period_10ns / 10;

    eto_set_frame_period_10ns(handle->ptr, period_10ns);

    // enable or disable traffic shaping of video packets
    if (enable) {
        eto_set_config_traffic_shaping_en(handle->ptr);
    } else {
        eto_set_config_traffic_shaping_dis(handle->ptr);
    }
}

