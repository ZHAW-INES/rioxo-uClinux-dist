
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
    handle->audio_event_queue = 0;

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
    uint32_t err;

    REPORT(INFO, "status : %x", handle->status);

    asi_drv_stop(handle);

    err = asi_drv_set_eth_params(handle, eth_params);
    if(err != ERR_ASI_SUCCESS) {
        return err;
    }

    err = asi_drv_set_aud_params(handle, aud_params);
    if(err != ERR_ASI_SUCCESS) {
        return err;
    }

    asi_drv_start(handle);

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


    if ((aud_params->sample_width) > 16) {
        aud_params->sample_width = 24;
    }

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
    int i;
    uint32_t status =  asi_get_status(handle->p_asi,0xFFFFFFFF);
    uint32_t fs = asi_get_fs(handle->p_asi);
    uint32_t ch_cnt_tmp = asi_get_ch_cnt(handle->p_asi);
    uint32_t ch_cnt = 0;

    if((status & ASI_STAT_RBFULL) != 0) {
        if((handle->status & ASI_DRV_STATUS_RBF_ERROR) == 0) {
            queue_put(event_queue, E_ASI_RBF_ERROR);
        }
        handle->status = handle->status | ASI_DRV_STATUS_RBF_ERROR;
    } else { 
        handle->status = handle->status & ~ASI_DRV_STATUS_RBF_ERROR;
    }

    // detect sampling rate
    if ((handle->fs) != fs) {
        switch (fs/200) {                           // mysterious bug in gateware: why is double of fs measured
            case (31):  case (32):  handle->sampling_rate = 32000;
                                    break;
            case (43):  case (44):  handle->sampling_rate = 44100;
                                    break;
            case (47):  case (48):  handle->sampling_rate = 48000;
                                    break;
            case (87):  case (88):  handle->sampling_rate = 88200;
                                    break;
            case (95):  case (96):  handle->sampling_rate = 96000;
                                    break;
            case (175): case (176): handle->sampling_rate = 176400;
                                    break;
            case (191): case (192): handle->sampling_rate = 192000;
                                    break;
            default:                handle->sampling_rate = 0;
        }
        if (handle->sampling_rate_old != handle->sampling_rate) {
            handle->audio_event_queue = 1;
        }
        handle->sampling_rate_old = handle->sampling_rate;
    }
    handle->fs = fs;

    // detect number of channels
    for(i=0;i<32;i++) {
        ch_cnt += ((ch_cnt_tmp >> i) & 0x00000001);
    }
    if ((handle->ch_cnt) != ch_cnt) {
        if (ch_cnt == 0) {
            // wait 60s after no audio data is available to send a stop event
            handle->stop_queue = true;
            handle->stop_counter = 0;
        } else {
            if ((handle->ch_cnt_old != ch_cnt) || (handle->stop_queue == false)) {
                handle->audio_event_queue = 1;
            }
            handle->stop_queue = false;
        }
        handle->ch_cnt_old = handle->ch_cnt;
    }
    handle->ch_cnt = ch_cnt;

    // 60s timer to stop audio with delay
    if (handle->stop_queue == true) {
        if (handle->stop_counter == 20000) {
            handle->stop_queue = false;
            handle->audio_event_queue = 1;
        } else {
            handle->stop_counter ++;
        }
    }

    // To prevent that an audio event appears immediately after another audio event, wait some time (1s) before send an event to deamon.
    // That occur on hdmi source plug in, than changes fs and ch-count almost at same time.
    if (handle->audio_event_queue != 0) {
        handle->audio_event_queue++;
    }
    if (handle->audio_event_queue == 30) {
        handle->audio_event_queue = 0;
        handle->stop_queue = false;
        queue_put(event_queue, E_ASI_NEW_FS);
        queue_put(event_queue, E_ADV7441A_NO_AUDIO);        // audio is restarting after this stop
    }

    return ERR_ASI_SUCCESS;
}

int asi_drv_get_fs(t_asi* handle)
{
    return handle->sampling_rate;
}

int asi_drv_get_ch_cnt(t_asi* handle) 
{
    return handle->ch_cnt;
}
