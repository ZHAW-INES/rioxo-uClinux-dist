
#include "eti_drv.h"


/** Start FSM
 *
 */
int eti_drv_start(t_eti* handle)
{
    eti_set_cpu_filter_mask(handle->ptr, ETI_CPU_FILTER_MASK);
	eti_set_config_start(handle->ptr);
    
	return ERR_ETI_SUCCESS;
}

/** Stop FSM
 *
 */
int eti_drv_stop(t_eti* handle)
{
    eti_set_cpu_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
	eti_set_config_stop(handle->ptr);

    while(!(eti_get_status_reg(handle->ptr) & ETI_STATUS_FSM_IDLE));
 
	return ERR_ETI_SUCCESS;
}

/** Starts audio embedded
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_start_aud_emb(t_eti* handle)
{
    eti_set_aud_emb_filter_mask(handle->ptr, ETI_AUD_EMB_FILTER_MASK);
    eti_set_aud_emb_fec_col_filter_mask(handle->ptr, ETI_AUD_EMB_FEC_C_FILTER_MASK);
    eti_set_aud_emb_fec_row_filter_mask(handle->ptr, ETI_AUD_EMB_FEC_R_FILTER_MASK);

    return ERR_ETI_SUCCESS;
}

/** Starts audio board
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_start_aud_board(t_eti* handle)
{
    eti_set_aud_board_filter_mask(handle->ptr, ETI_AUD_BOARD_FILTER_MASK);
    eti_set_aud_board_fec_col_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
    eti_set_aud_board_fec_row_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);

    return ERR_ETI_SUCCESS;
}

/** Stops audio embedded
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_stop_aud_emb(t_eti* handle)
{
    eti_set_aud_emb_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
    eti_set_aud_emb_fec_col_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
    eti_set_aud_emb_fec_row_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);

    return ERR_ETI_SUCCESS;
}

/** Stops audio board
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_stop_aud_board(t_eti* handle)
{
    eti_set_aud_board_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
    eti_set_aud_board_fec_col_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
    eti_set_aud_board_fec_row_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);

    return ERR_ETI_SUCCESS;
}

/** Starts video ringbuffer
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_start_vid(t_eti* handle)
{
    uint32_t tmp_ip;
    eti_set_vid_filter_mask(handle->ptr, ETI_VID_FILTER_MASK);
    eti_set_vid_fec_col_filter_mask(handle->ptr, ETI_VID_FEC_C_FILTER_MASK);
    eti_set_vid_fec_row_filter_mask(handle->ptr, ETI_VID_FEC_R_FILTER_MASK);

    tmp_ip = eti_get_vid_src_ip(handle->ptr);
    REPORT(INFO, "[ETI] video ip : %pI4", &tmp_ip);
    tmp_ip = eti_get_aud_src_ip(handle->ptr);
    REPORT(INFO, "[ETI] video port : %d", ntohs(eti_get_vid_dst_udp_port(handle->ptr)));

    return ERR_ETI_SUCCESS;
}

/** Stops video ringbuffer
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_stop_vid(t_eti* handle)
{
    eti_set_vid_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
    eti_set_vid_fec_col_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
    eti_set_vid_fec_row_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);

    return ERR_ETI_SUCCESS;
}

/** Enables audio aes encryption 
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_aes_aud_en(t_eti* handle)
{
    eti_set_config_audio_enc_en(handle->ptr);
    return ERR_ETI_SUCCESS;
}

/** Disables audio aes encryption 
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_aes_aud_dis(t_eti* handle)
{
    eti_set_config_audio_enc_dis(handle->ptr);
    return ERR_ETI_SUCCESS;
}

/** Get status of audio and video aes encryption (on/off?) 
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_aes_stat(t_eti* handle, uint32_t *audio_enc_en, uint32_t *video_enc_en)
{
    *audio_enc_en = eti_get_config_audio_enc_en(handle->ptr);
    *video_enc_en = eti_get_config_video_enc_en(handle->ptr);
    //REPORT(INFO, "audio(): (audio stat : %08x)", *audio_enc_en);
    return ERR_ETI_SUCCESS;
}
/** Enables video aes encryption 
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_aes_vid_en(t_eti* handle)
{
    eti_set_config_video_enc_en(handle->ptr);
    return ERR_ETI_SUCCESS;
}

/** Disables video aes encryption 
 *
 * @param handle pointer to the eti handle
 * @return error code
 */
int eti_drv_aes_vid_dis(t_eti* handle)
{
    eti_set_config_video_enc_dis(handle->ptr);
    return ERR_ETI_SUCCESS;
}

/** Initialize CPU ringbuffer
 *
 * @param handle pointer to the eti handle
 * @param start_ptr start address of the buffer
 * @param size size of the buffer
 * @return error code
 */
int eti_drv_set_cpu_buf(t_eti* handle, void* start_ptr, size_t size)
{
    t_rbf_dsc dsc;

    rbf_dsc(&dsc, start_ptr, size);
    eti_set_cpu_desc(handle->ptr, &dsc);

    return ERR_ETI_SUCCESS;
}

/** Set the audio & video filter parameters 
 *
 * @param handle pointer to the eti handle
 * @param dev_ip device ip
 * @param aud_src_ip audio source ip
 * @param vid_src_ip video source ip
 * @oaram aud_port audio udp port
 * @param vid_port video udp port
 * @return error code
 */
int eti_drv_set_filter(t_eti* handle, uint32_t dev_ip, uint32_t aud_src_ip, uint32_t vid_src_ip, uint16_t aud_emb_port, uint16_t aud_board_port, uint16_t vid_port)
{
    eti_set_device_ip(handle->ptr, dev_ip);
    if (vid_port) eti_set_vid_src_ip(handle->ptr, vid_src_ip);
    if (vid_port) eti_set_vid_dst_udp_port(handle->ptr, vid_port);
    if (aud_emb_port)   eti_set_aud_src_ip(handle->ptr, aud_src_ip);
    if (aud_emb_port)   eti_set_aud_emb_dst_udp_port(handle->ptr, aud_emb_port);
    if (aud_board_port) eti_set_aud_src_ip(handle->ptr, aud_src_ip);
    if (aud_board_port) eti_set_aud_board_dst_udp_port(handle->ptr, aud_board_port);

    return ERR_ETI_SUCCESS;
}

/** Sets encryption parameters
 *
 * @param handle pointer to eth eti handle
 * @param key array of 4 keys
 * @param riv array of 2 rivs
 * @return error code
 */
int eti_drv_set_aes(t_eti* handle, uint32_t key[4], uint32_t riv[2])
{
	eti_set_aes_session_key_0(handle->ptr, key[0]);
	eti_set_aes_session_key_1(handle->ptr, key[1]);
	eti_set_aes_session_key_2(handle->ptr, key[2]);
	eti_set_aes_session_key_3(handle->ptr, key[3]);
	eti_set_aes_riv_0(handle->ptr, riv[0]);
	eti_set_aes_riv_1(handle->ptr, riv[1]);

	return ERR_ETI_SUCCESS;
}

/** Reads encryption parameters
 *
 * @param handle pointer to eth eti handle
 * @param key array of 4 keys
 * @param riv array of 2 rivs
 * @return error code
 */
int eti_drv_get_aes(t_eti* handle, uint32_t key[4], uint32_t riv[2])
{
    key[0] = eti_get_aes_session_key_0(handle->ptr);
    key[1] = eti_get_aes_session_key_1(handle->ptr);
    key[2] = eti_get_aes_session_key_2(handle->ptr);
    key[3] = eti_get_aes_session_key_3(handle->ptr);
    riv[0] = eti_get_aes_riv_0(handle->ptr);
    riv[1] = eti_get_aes_riv_1(handle->ptr);

    return ERR_ETI_SUCCESS;
}

/** Sets the pointer to the acb registers
 *
 * @param handle pointer to the eti handle
 * @param ptr pointer to the acb registers
 * @return error code
 */
int eti_drv_set_ptr(t_eti* handle, void* ptr)
{
    handle->ptr = ptr;
    handle->link_state = 0;
    return ERR_ETI_SUCCESS;
}

/** Initialize ETI core driver
 *
 */
int eti_drv_init(t_eti* handle, void* ptr)
{
	REPORT(INFO, "+--------------------------------------------------+");
	REPORT(INFO, "| ETI-Driver: Initialize ethernet stream in        |");
	REPORT(INFO, "+--------------------------------------------------+");

    handle->ptr = ptr;
    handle->link_state = 0;
    handle->vrx = 0;
    handle->arx = 0;
    handle->abrx = 0;
    handle->vcnt = 0;
    handle->acnt = 0;
    handle->abcnt = 0;

	eti_drv_stop(handle);
    eti_clr_irq1(handle->ptr);

	eti_set_burst_size_reg(handle->ptr, ETI_DMA_BURST_SIZE);
	eti_set_config_audio_enc_dis(handle->ptr);
	eti_set_config_video_enc_dis(handle->ptr);

	eti_set_cpu_filter_mask(handle->ptr, ETI_CPU_FILTER_MASK);


	eti_set_vid_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
	eti_set_vid_fec_col_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
	eti_set_vid_fec_row_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
	eti_set_aud_emb_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
	eti_set_aud_emb_fec_col_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
	eti_set_aud_emb_fec_row_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
	eti_set_aud_board_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
	eti_set_aud_board_fec_col_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);
	eti_set_aud_board_fec_row_filter_mask(handle->ptr, ETI_DIS_FILTER_MASK);


    REPORT(INFO, "config reg : %08x", eti_get_config_reg(handle->ptr));
    REPORT(INFO, "status reg : %08x", eti_get_status_reg(handle->ptr));
    REPORT(INFO, "burst size : %d", eti_get_burst_size_reg(handle->ptr));

	return ERR_ETI_SUCCESS;
}

int eti_drv_handler(t_eti* handle, t_queue* event_queue)
{
    uint32_t reg = eti_get_config_reg(handle->ptr) & ETI_CONFIG_FSM_EN;
    uint32_t vrx = eti_get_vid_packet_cnt(handle->ptr);
    uint32_t arx = eti_get_aud_emb_packet_cnt(handle->ptr);
    uint32_t abrx = eti_get_aud_board_packet_cnt(handle->ptr);

    if(reg != handle->link_state) {
        if(reg > 0) {   /* link up */
            queue_put(event_queue, E_ETI_LINK_UP);     
        } else {        /* link down */
            queue_put(event_queue, E_ETI_LINK_DOWN);     
        }
        handle->link_state = reg;
    }

    if (vrx != handle->vrx) {
        handle->vrx = vrx;
        if (!handle->vcnt) {
            queue_put(event_queue, E_ETI_VIDEO_ON);
        }
        handle->vcnt = ETI_LINK_COUNT;
    } else {
        if (handle->vcnt == 1) {
            queue_put(event_queue, E_ETI_VIDEO_OFF);
        }
        if (handle->vcnt) {
            handle->vcnt--;
        }
    }

    if (arx != handle->arx) {
        handle->arx = arx;
        if (!handle->acnt) {
            queue_put(event_queue, E_ETI_AUDIO_EMB_ON);
        }
        handle->acnt = ETI_LINK_COUNT;
    } else {
        if (handle->acnt == 1) {
            queue_put(event_queue, E_ETI_AUDIO_EMB_OFF);
        }
        if (handle->acnt) {
            handle->acnt--;
        }
    }

    if (abrx != handle->abrx) {
        handle->abrx = abrx;
        if (!handle->abcnt) {
            queue_put(event_queue, E_ETI_AUDIO_BOARD_ON);
        }
        handle->abcnt = ETI_LINK_COUNT;
    } else {
        if (handle->abcnt == 1) {
            queue_put(event_queue, E_ETI_AUDIO_BOARD_OFF);
        }
        if (handle->abcnt) {
            handle->abcnt--;
        }
    }

    return ERR_ETI_SUCCESS;
}
