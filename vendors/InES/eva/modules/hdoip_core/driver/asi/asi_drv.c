
#include "asi_drv.h"

/** Starts audio stream in 
 *
 * @param asi pointer to the audio stream in asi struct
 * @return error code
 */
int asi_drv_start(t_asi* asi, int unsigned stream)
{
    if((asi->stream_status[stream] & ASI_DRV_STREAM_STATUS_AUD_PARAMS_SET) == 0) {
        return ERR_ASI_AUD_PARAMS_NOT_SET;
    }

    if((asi->stream_status[stream] & ASI_DRV_STREAM_STATUS_ETH_PARAMS_SET) == 0) {
        return ERR_ASI_ETH_PARAMS_NOT_SET;
    }
    
    asi_enable(asi->p_asi, stream);
    asi->settings_changed = 0;
    asi->stream_status[stream] |= ASI_DRV_STREAM_STATUS_ACTIV;

    return ERR_ASI_SUCCESS; 
}

/** Stops audio stream in 
 *
 * @param asi pointer to the audio stream in asi struct
 * @return error code
 */
int asi_drv_stop(t_asi* asi, int unsigned stream)
{
    uint32_t mask = asi_stat_idle_mask(stream);

    /* stop stream */
    asi_disable(asi->p_asi, stream);

    /* wait for DMA/FSM to return in IDLE */
    while(asi_get_status(asi->p_asi, mask) == 0);

    asi->stream_status[stream] &= ~ASI_DRV_STREAM_STATUS_ACTIV;

    return ERR_ASI_SUCCESS;
}

/** Initialize audio stream in core driver  
 *
 * @param asi pointer to the audio stream in asi struct
 * @param p_asi start address of the ACB registers
 * @return error code
 */
int asi_drv_init(t_asi* asi, void* p_asi)
{
    int i;

    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| ASI-Driver: Initialize audio stream in           |");
    REPORT(INFO, "+--------------------------------------------------+");

    asi->p_asi = p_asi;
    asi->settings_changed = 0;

    for (i = 0; i < AUD_STREAM_CNT; i++) {
      asi->stream_status[i] = 0;
      asi->detected_fs[i] = 0;
      asi->detected_ch_map[i] = 0;
      asi_drv_stop(asi, i);
    }

    asi_set_dma_burst_size(p_asi, ASI_DRV_DMA_BURST_SIZE);

    return ERR_ASI_SUCCESS;
}

/** Updates all parameters (audio and ethernet)
 *
 * @param asi pointer to the audio stream in asi struct
 * @param eth_params pointer to the ethernet paramerter struct
 * @param aud_params pointer to the audio parameter struct
 * @return error code
 */
int asi_drv_update(t_asi* asi, int unsigned stream,
    struct hdoip_eth_params* eth_params, struct hdoip_aud_params* aud_params)
{
    uint32_t err;

    asi_drv_stop(asi, stream);

    // if 20Bit audio, container size must be 24Bit
    if ((aud_params->sample_width) == 20 ) {
        aud_params->sample_width = 24;
    }

    err = asi_drv_set_eth_params(asi, stream, eth_params, aud_params);
    if(err != ERR_ASI_SUCCESS) {
        return err;
    }

    err = asi_drv_set_aud_params(asi, stream, aud_params);
    if(err != ERR_ASI_SUCCESS) {
        return err;
    }

    asi_drv_start(asi, stream);

    return ERR_ASI_SUCCESS;
}

/** Flush the ringbuffer (set write to read pointer)
 *
 * @param asi pointer to the asi asi
 * @param dsc ringbuffer descriptor set
 * @return error code
 */
int asi_drv_flush_buf(t_asi* asi)
{
    asi_set_write_dsc(asi->p_asi, asi_get_read_dsc(asi->p_asi));
    return ERR_ASI_SUCCESS;
}

/** Sets ethernet parameters (asi must be stoped) 
 *
 * @param asi pointer to the audio stream in asi struct
 * @param eth_params pointer to the ethernet parameter struct
 * @return error code
 */
int asi_drv_set_eth_params(t_asi* asi, int unsigned stream,
    struct hdoip_eth_params* eth_params, struct hdoip_aud_params* aud_params)
{
    uint16_t payload_words, frame_size;
    uint8_t sample_len, ch;

    /* check params */
    if((asi->stream_status[stream] & ASI_DRV_STREAM_STATUS_ACTIV) != 0) {
        return ERR_ASI_RUNNING;
    }
    if(((eth_params->packet_size) < ASI_DRV_MIN_FRAME_SIZE) || ((eth_params->packet_size) > ASI_DRV_MAX_FRAME_SIZE)) {
        return ERR_ASI_PACKET_LENGTH_ERR;
    }

    /* calculate payload words (frame size must match data container) */
    ch =  aud_chmap2cnt(aud_params->ch_map);
    payload_words = eth_params->packet_size/4 - ASI_DRV_ETH_HEADER_LEN - ASI_DRV_IPV4_HEADER_LEN - ASI_DRV_UDP_HEADER_LEN - ASI_DRV_RTP_HEADER_LEN;
    sample_len = aud_get_sample_length(aud_params->sample_width, ch);
    if (sample_len == 0) {
        return ERR_ASI_AUD_PARAMS_ERR;
    }

    payload_words = (payload_words / sample_len ) * sample_len; /* round */

    /* calculate header length fields */
    frame_size = (payload_words + ASI_DRV_RTP_HEADER_LEN - 1) * 4;

    /* set registers */
    asi_set_frame_size(asi->p_asi, stream, frame_size);
    asi_set_payload_words(asi->p_asi, stream, payload_words);
    asi_set_eth_params(asi->p_asi, stream, eth_params);

    /* finish */
    asi->stream_status[stream] |= ASI_DRV_STREAM_STATUS_ETH_PARAMS_SET;
    eth_report_params(eth_params, stream);
    return ERR_ASI_SUCCESS;
}

/** Get the ethernet parameters
 *
 * @param asi pointer to the audio stream in asi struct
 * @param eth_params pointe to the ethernet parameter struct
 * @return error code
 */
int asi_drv_get_eth_params(t_asi* asi, int unsigned stream, struct hdoip_eth_params* eth_params)
{
    if((asi->stream_status[stream] & ASI_DRV_STREAM_STATUS_ETH_PARAMS_SET) == 0) {
        return ERR_ASI_ETH_PARAMS_NOT_SET;
    }

    asi_get_eth_params(asi->p_asi, stream, eth_params);

    return ERR_ASI_SUCCESS;
}

/** Sets audio parameters (asi must be stoped and ethernet parameters set)
 *
 * @param asi pointer to the audio stream in asi struct
 * @param aud_params pointer to the audio parameter struct
 * @return error code
 */
int asi_drv_set_aud_params(t_asi* asi, int unsigned stream, struct hdoip_aud_params* aud_params)
{
    uint32_t steps_per_sec, tmp;
    uint16_t time_per_word;
    uint8_t ch;

    if((asi->stream_status[stream] & ASI_DRV_STREAM_STATUS_ACTIV) != 0) {
        return ERR_ASI_RUNNING;
    }

    /* calculate time per word */
    ch =  aud_chmap2cnt(aud_params->ch_map);
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
    asi_set_time_per_word(asi->p_asi, stream, time_per_word);
    asi_set_aud_params(asi->p_asi, stream, aud_params);

    asi->stream_status[stream] |= ASI_DRV_STREAM_STATUS_AUD_PARAMS_SET;
    aud_report_params(aud_params, stream);

    return ERR_ASI_SUCCESS;
}

/** Get the audio parameters
 *
 * @param asi pointer to the audio stream in asi struct
 * @param aud_params pointe to the audio parameter struct
 * @return error code
 */ 
int asi_drv_get_aud_params(t_asi* asi, int unsigned stream, struct hdoip_aud_params* aud_params)
{ 
    if((asi->stream_status[stream] & ASI_DRV_STREAM_STATUS_AUD_PARAMS_SET) == 0) {
        return ERR_ASI_AUD_PARAMS_NOT_SET;
    }

    asi_get_aud_params(asi->p_asi, stream, aud_params);
    return ERR_ASI_SUCCESS;
}

static uint32_t count2fs(uint32_t cnt)
{
    int i;
    uint32_t fs = 0;
    /* possible sampling rates */
    const uint32_t fs_tab[7] = {32000, 44100, 48000, 88200, 96000, 176400, 192000};
    /* counter counts over 2^17 micro seconds = 0.131s -> fs_tab * 0.131 =>  */
    const uint32_t cnt_tab[7] = {4194, 5780, 6291, 11561, 12583, 23121, 25166};
    const uint32_t tol = 7; // tolerance is 2^-tol * cnt_tab

    for (i=0; i < 7; i++) {
        if (cnt >= cnt_tab[i]) {
            if ((cnt - cnt_tab[i]) < (cnt_tab[i] >> tol))
                fs = fs_tab[i];
        }
        else {
            if ((cnt_tab[i] - cnt) < (cnt_tab[i] >> tol))
                fs = fs_tab[i];
        }
    }

    return fs;
}

/** audio stream in asir
 *
 * @param asi pointer to the audio stream in asi struct
 * @param event_queue pointer to the event queue struct
 * @return error code
 */
int asi_drv_handler(t_asi* asi, t_queue* event_queue)
{
    uint32_t fs_counter = asi_get_fs_detect(asi->p_asi, AUD_STREAM_NR_EMBEDDED);
    uint32_t fs;
    uint32_t detected_ch_map = asi_get_ch_map_detected(asi->p_asi, AUD_STREAM_NR_EMBEDDED);

    // detect fs change
    fs = count2fs(fs_counter);
    if (fs != asi->fs_old) {
        asi->settings_changed = 1;
    }
    asi->fs_old = fs;

    // detect ch_map change
    if (detected_ch_map != asi->ch_map_old) {
        asi->settings_changed = 1;
    }
    asi->ch_map_old = detected_ch_map;

    // To prevent that an audio event appears immediately after another audio event, wait some time  before send an event to deamon.
    // That occur on hdmi source plug in, than changes fs and ch-count almost at same time.
    if (asi->settings_changed >= 30) {
        asi->settings_changed = 0;
        asi->detected_ch_map[AUD_STREAM_NR_EMBEDDED] = detected_ch_map;
        asi->detected_fs[AUD_STREAM_NR_EMBEDDED] = fs;
        queue_put(event_queue, E_ASI_NEW_FS);
        // this event produces a new connection setup
        queue_put(event_queue, E_ADV7441A_NO_AUDIO);
    }
    else if (asi->settings_changed) {
      asi->settings_changed++;
    }
    
    return ERR_ASI_SUCCESS;
}
