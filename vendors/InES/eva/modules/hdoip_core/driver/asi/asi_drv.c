
#include "asi_drv.h"

/** Starts audio stream in 
 *
 * @param handle pointer to the audio stream in handle struct
 * @return error code
 */
int asi_drv_start(t_asi* handle)
{
    if((handle->status & ASI_DRV_STATUS_AUD_PARAMS_SET) == 0) {
        return ERR_ASI_AUD_PARAMS_NOT_SET;
    }

    if((handle->status & ASI_DRV_STATUS_ETH_PARAMS_SET) == 0) {
        return ERR_ASI_ETH_PARAMS_NOT_SET;
    }
    
    asi_enable(handle->p_asi);
    handle->status = handle->status | ASI_DRV_STATUS_ACTIV;
    return ERR_ASI_SUCCESS; 
}

/** Stops audio stream in 
 *
 * @param handle pointer to the audio stream in handle struct
 * @return error code
 */
int asi_drv_stop(t_asi* handle)
{
    asi_disable(handle->p_asi);

    while(asi_get_ctrl(handle->p_asi, ASI_STAT_IDLE) != 0);

    handle->status = handle->status & ~ASI_DRV_STATUS_ACTIV;
    return ERR_ASI_SUCCESS;
}



/** Initialize audio stream in core driver  
 *
 * @param handle pointer to the audio stream in handle struct
 * @param p_asi start address of the ACB registers
 * @return error code
 */
int asi_drv_init(t_asi* handle, void* p_asi) 
{
    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| ASI-Driver: Initialize audio stream in           |");
    REPORT(INFO, "+--------------------------------------------------+");

    handle->p_asi = p_asi;
    handle->status = 0;

    asi_drv_stop(handle); 

    asi_set_dma_burst_size(p_asi, ASI_DRV_DMA_BURST_SIZE);

    return ERR_ASI_SUCCESS;
}



/** Updates all parameters (audio and ethernet)
 *
 * @param handle pointer to the audio stream in handle struct
 * @param eth_params pointer to the ethernet paramerter struct
 * @param aud_params pointer to the audio parameter struct
 * @return error code
 */
int asi_drv_update(t_asi* handle, struct hdoip_eth_params* eth_params, struct hdoip_aud_params* aud_params)
{
    uint32_t activ = handle->status & ASI_DRV_STATUS_ACTIV;
    uint32_t err;

    REPORT(INFO, "status : %x", handle->status);

    if(activ != 0) {
        asi_drv_stop(handle);
    }

    err = asi_drv_set_eth_params(handle, eth_params);
    if(err != ERR_ASI_SUCCESS) {
        return err;
    }

    err = asi_drv_set_aud_params(handle, aud_params);
    if(err != ERR_ASI_SUCCESS) {
        return err;
    }

    if(activ != 0) {
        asi_drv_start(handle);
    }

    return ERR_ASI_SUCCESS;
}

/** Inittialize the ringbuffer (write pointer)
 *
 * @param handle pointer to the asi handle
 * @param start_ptr start address of the buffer
 * @param size size of the buffer
 * @return error code
 */
int asi_drv_set_buf(t_asi* handle, void* start_ptr, size_t size)
{
    t_rbf_dsc dsc;
    rbf_dsc(&dsc, start_ptr, size);
    asi_set_dsc(handle->p_asi,  &dsc);

    return ERR_ASI_SUCCESS;
}

/** Flush the ringbuffer (set write to read pointer)
 *
 * @param handle pointer to the asi handle
 * @return error code
 */
int asi_drv_flush_buf(t_asi* handle)
{
    asi_set_write_dsc(handle->p_asi, asi_get_read_dsc(handle->p_asi));
    return ERR_ASI_SUCCESS;
}

/** Sets ethernet parameters (asi must be stoped) 
 *
 * @param handle pointer to the audio stream in handle struct
 * @param eth_params pointer to the ethernet parameter struct
 * @return error code
 */
int asi_drv_set_eth_params(t_asi* handle, struct hdoip_eth_params* eth_params) 
{
    if((handle->status & ASI_DRV_STATUS_ACTIV) != 0) {
        return ERR_ASI_RUNNING;
    }

    if(((eth_params->packet_size) < ASI_DRV_MIN_FRAME_SIZE) || ((eth_params->packet_size) > ASI_DRV_MAX_FRAME_SIZE)) {
        return ERR_ASI_PACKET_LENGTH_ERR;
    }

    memcpy(&handle->eth_params, eth_params, sizeof(struct hdoip_eth_params));

    asi_set_eth_params(handle->p_asi, eth_params);

    handle->status = (handle->status & ~ASI_DRV_STATUS_AUD_PARAMS_SET) | ASI_DRV_STATUS_ETH_PARAMS_SET;

    eth_report_params(eth_params);

    return ERR_ASI_SUCCESS;
}

/** Get the ethernet parameters
 *
 * @param handle pointer to the audio stream in handle struct
 * @param eth_params pointe to the ethernet parameter struct
 * @return error code
 */
int asi_drv_get_eth_params(t_asi* handle, struct hdoip_eth_params* eth_params)
{
    if((handle->status & ASI_DRV_STATUS_ETH_PARAMS_SET) == 0) {
        return ERR_ASI_ETH_PARAMS_NOT_SET;
    }

    asi_get_eth_params(handle->p_asi, eth_params);

    memcpy(&handle->eth_params, eth_params, sizeof(struct hdoip_eth_params));  

    return ERR_ASI_SUCCESS;
}

/** Sets audio parameters (asi must be stoped and ethernet parameters set)
 *
 * @param handle pointer to the audio stream in handle struct
 * @param aud_params pointer to the audio parameter struct
 * @return error code
 */
int asi_drv_set_aud_params(t_asi* handle, struct hdoip_aud_params* aud_params) 
{
    uint32_t steps_per_sec, tmp;
    uint16_t payload_words, ip_length, udp_length, frame_size, time_per_word;
    uint8_t sample_len, ch;

    if((handle->status & ASI_DRV_STATUS_ACTIV) != 0) {
        return ERR_ASI_RUNNING;
    }

    if((handle->status & ASI_DRV_STATUS_ETH_PARAMS_SET) == 0) {
        return ERR_ASI_ETH_PARAMS_NOT_SET;
    }

    ch = aud_params->ch_cnt_left + aud_params->ch_cnt_right; 

    /* calculate payload words */
    payload_words = handle->eth_params.packet_size/4 - ASI_DRV_ETH_HEADER_LEN - ASI_DRV_IPV4_HEADER_LEN - ASI_DRV_UDP_HEADER_LEN - ASI_DRV_RTP_HEADER_LEN - ASI_DRV_AES_HEADER_LEN;
    sample_len = aud_get_sample_length(aud_params->sample_width, ch);
    payload_words = (payload_words / sample_len ) * sample_len; /* round */

    /* calculate header length fields */
    frame_size = (payload_words + ASI_DRV_ETH_HEADER_LEN + ASI_DRV_IPV4_HEADER_LEN + ASI_DRV_UDP_HEADER_LEN + ASI_DRV_RTP_HEADER_LEN + ASI_DRV_AES_HEADER_LEN - 1) * 4;
    ip_length  = (payload_words + ASI_DRV_IPV4_HEADER_LEN + ASI_DRV_UDP_HEADER_LEN + ASI_DRV_RTP_HEADER_LEN + ASI_DRV_AES_HEADER_LEN) * 4;
    udp_length = (payload_words + ASI_DRV_UDP_HEADER_LEN + ASI_DRV_RTP_HEADER_LEN + ASI_DRV_AES_HEADER_LEN); 

    /* calculate time per word */
    tmp = aud_params->fs * ch;

    switch(aud_params->sample_width) {
        case 24: tmp = tmp - (tmp>>2);  /* tmp = tmp * 3/4 */
                 break;
        case 16: tmp = tmp >> 1;        /* tmp = tmp / 2 */
                 break;
        case 8:  tmp = tmp >> 2;        /* tmp = tmp / 4 */
                 break;
    }
    steps_per_sec = (uint32_t) (1 / ((1.0 / (float)SFREQ) * (uint32_t)(SFREQ/1000000))); // timer steps per second
    time_per_word = (uint16_t) (((1 << 6) * steps_per_sec) / tmp);

    /* write parameter to registers */
    asi_set_frame_size(handle->p_asi, frame_size);
    asi_set_ip_length(handle->p_asi, ip_length);
    asi_set_udp_word_length(handle->p_asi, udp_length);
    asi_set_payload_words(handle->p_asi, payload_words);
    asi_set_time_per_word(handle->p_asi, time_per_word);
    asi_set_aud_params(handle->p_asi, aud_params);

    handle->status = handle->status | ASI_DRV_STATUS_AUD_PARAMS_SET;
    
    aud_report_params(aud_params);

    return ERR_ASI_SUCCESS;
}

/** Get the audio parameters
 *
 * @param handle pointer to the audio stream in handle struct
 * @param aud_params pointe to the audio parameter struct
 * @return error code
 */ 
int asi_drv_get_aud_params(t_asi* handle, struct hdoip_aud_params* aud_params) 
{ 
    if((handle->status & ASI_DRV_STATUS_AUD_PARAMS_SET) == 0) {
        return ERR_ASI_AUD_PARAMS_NOT_SET;
    }

    asi_get_aud_params(handle->p_asi, aud_params);
    return ERR_ASI_SUCCESS;
}

/** audio stream in handler
 *
 * @param handle pointer to the audio stream in handle struct
 * @param event_queue pointer to the event queue struct
 * @return error code
 */
int asi_drv_handler(t_asi* handle, t_queue* event_queue)
{
    uint32_t status =  asi_get_status(handle->p_asi,0xFFFFFFFF);

    if((status & ASI_STAT_RBFULL) != 0) {
        if((handle->status & ASI_DRV_STATUS_RBF_ERROR) == 0) {
            queue_put(event_queue, E_ASI_RBF_ERROR);
        }
        handle->status = handle->status | ASI_DRV_STATUS_RBF_ERROR;
    } else { 
        handle->status = handle->status & ~ASI_DRV_STATUS_RBF_ERROR;
    }

    return ERR_ASI_SUCCESS;
}

