
#include "stdrbf.h"
#include "aso_drv.h"

/** Initialize audio stream out driver 
 *
 * @param aso pointer to the audio stream out handle struct
 * @param p_aso address of the ACB audio stream out register
 * @return error code
 */
int aso_drv_init(t_aso* aso, void* p_aso, int unsigned stream_nr)
{
    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| ASO-Driver: Initialize audio stream out %u        |", stream_nr);
    REPORT(INFO, "+--------------------------------------------------+");

    aso->p_aso = p_aso;
    aso->stream_nr = stream_nr;
    aso->stream_status = 0;
    aso_drv_stop(aso);

    return ERR_ASO_SUCCESS;
}

/** Initialize the ringbuffer (read pointer)
 *
 * @param aso pointer to the aso handle
 * @param start_ptr start address of the buffer
 * @param size size of the buffer
 * @return error code
 */ /*
int aso_drv_set_buf(t_aso* aso, void* start_ptr, size_t size)
{
    t_rbf_dsc dsc;

    rbf_dsc(&dsc, start_ptr, size);
    aso_set_dsc(aso->p_aso, &dsc);

    return ERR_ASO_SUCCESS;
}*/

/** Flush the ringbuffer (set read to write pointer)
 *
 * @param aso pointer to the aso handle
 * @return error code
 */
int aso_drv_flush_buf(t_aso* aso)
{
    //TODO: clear frame reordering ram
    return ERR_ASO_SUCCESS;
}

/** Starts audio stream out (parameters must be set)
*
* @param aso pointer to the audio stream out handle struct
* @return error code
*/
int aso_drv_start(t_aso* aso)
{
    if((aso->stream_status & ASO_DRV_STATUS_ACTIV) != 0) {
        return ERR_ASO_RUNNING;
    }

    if((aso->stream_status & ASO_DRV_STATUS_AUD_PARAMS_SET) == 0) {
        return ERR_ASO_AUD_PARAMS_NOT_SET;
    }

    if((aso->stream_status & ASO_DRV_STATUS_AUD_DELAY_SET) == 0) {
        return ERR_ASO_AUD_DELAY_NOT_SET;
    }

    aso_enable_i2s_output(aso->p_aso);
    aso_enable(aso->p_aso);

    aso->stream_status |= ASO_DRV_STATUS_ACTIV;
    
    return ERR_ASO_SUCCESS;
}


/** Stops audio stream out 
*
* @param aso pointer to the audio stream out handle struct
* @return error code
*/
int aso_drv_stop(t_aso* aso)
{
    uint32_t timeout = 10000000;

    aso_disable(aso->p_aso);

    while(timeout > 0) {
        if (aso_get_status(aso->p_aso, ASO_STATUS_IDLE) != 0) {
            break;
        }
        timeout -= 1;
    }
    if (timeout == 0) {
        REPORT(ERROR, "ASO blocked: unable to stop");
        return ERR_ASO;
    }
    
    aso_clr_status(aso->p_aso, ASO_STATUS_ALL);
    aso->stream_status = 0;
    return ERR_ASO_SUCCESS;
}

/** Updates all parameters during runtime
 *
 * @param aso pointer to the audio stream out handle struct
 * @param aud_params pointer to the audio parameter struct
 * @param aud_delay_us audio delay in microseconds
 * @return error code
 */
int aso_drv_update(t_aso* aso, struct hdoip_aud_params* aud_params, uint32_t aud_delay_us, uint16_t config)
{
    uint32_t err;

    aso_drv_stop(aso);

    // if sample width = 20Bit, a container size of 24Bit is used
    if ((aud_params->sample_width) == 20) {
        aud_params->sample_width = 24;
    }

    aso_set_clk_config(aso->p_aso, config);

    err = aso_drv_set_aud_params(aso, aud_params);
    if(err != ERR_ASO_SUCCESS) {
        return err;
    }

    err = aso_drv_set_aud_delay(aso, aud_delay_us);
    if(err != ERR_ASO_SUCCESS) {
        return err;
    }

    aso_drv_start(aso);

    return ERR_ASO_SUCCESS;
}

/** Audio stream out handler
 *
 * @param handle pointer to the audio stream out handle struct
 * @param event_queue pointer to the event queue
 * @return error code
 */
int aso_drv_handler(t_aso* aso, t_queue* event_queue)
{
    uint32_t status = 0;

    status = aso_get_status(aso->p_aso, ASO_STATUS_TIMESTAMP_ERROR | ASO_STATUS_FIFO_EMPTY | ASO_STATUS_FIFO_FULL);


    if((status & ASO_STATUS_TIMESTAMP_ERROR) != 0) {
        if((aso->stream_status & ASO_DRV_STATUS_TIMESTAMP_ERROR) == 0) {
            if (aso->stream_nr==0) queue_put(event_queue, E_ASO_EMB_TS_ERROR);
            else                   queue_put(event_queue, E_ASO_BOARD_TS_ERROR);
        }
        aso->stream_status |= ASO_DRV_STATUS_TIMESTAMP_ERROR;
    } else {
        aso->stream_status &=  ~ASO_DRV_STATUS_TIMESTAMP_ERROR;
    }

    if((status & ASO_STATUS_FIFO_EMPTY) != 0) {
        if((aso->stream_status & ASO_DRV_STATUS_FIFO_EMPTY) == 0) {
            if (aso->stream_nr==0) queue_put(event_queue, E_ASO_EMB_FIFO_EMPTY);
            else                   queue_put(event_queue, E_ASO_BOARD_FIFO_EMPTY);
        }
        aso->stream_status |= ASO_DRV_STATUS_FIFO_EMPTY;
    } else {
        aso->stream_status &=  ~ASO_DRV_STATUS_FIFO_EMPTY;
    }

    if((status & ASO_STATUS_FIFO_FULL) != 0) {
        if((aso->stream_status & ASO_DRV_STATUS_FIFO_FULL) == 0) {
            if (aso->stream_nr==0) queue_put(event_queue, E_ASO_EMB_FIFO_FULL);
            else                   queue_put(event_queue, E_ASO_BOARD_FIFO_FULL);
        }
        aso->stream_status |= ASO_DRV_STATUS_FIFO_FULL;
    } else {
        aso->stream_status &=  ~ASO_DRV_STATUS_FIFO_FULL;
    }

    return ERR_ASO_SUCCESS;
}

/** Sets the audio parameters 
 *
 * @param aso pointer to the audio stream out handle struct
 * @param aud_params pointer to the audio parameter struct
 * @return error code
 */
int aso_drv_set_aud_params(t_aso* aso, struct hdoip_aud_params* aud_params)
{
    uint64_t div_base;
    uint32_t div_ki, div_kp;
    uint32_t mclk_freq;

	//default values for clk control
	div_kp = 400;//400   //200
    div_ki = 250;//250   //4000	// 1/KI

    if((aso->stream_status & ASO_DRV_STATUS_ACTIV) != 0) {
        return ERR_ASO_RUNNING;
    }

    switch (aud_params->fs) {
      case 32000:
        mclk_freq = 32000*64;
        break;
      case 48000:
        mclk_freq = 48000*4*64; // Audio Codec use this frequency (12.288MHz)
        break;
      case 48000*2:
        mclk_freq = 48000*2*64;
        break;
      case 48000*4:
        mclk_freq = 48000*4*64;
        break;
      case 44100:
        mclk_freq = 44100*64;
        break;
      case 44100*2:
        mclk_freq = 44100*2*64;
        break;
      case 44100*4:
        mclk_freq = 44100*4*64;
        break;
      default:
        return ERR_ASO_SAMPLE_WIDTH_ERR;
    }

    /* calculate i2s clock (in q8.24 format) */

    /* division by 2 because clock signal toggles after division factor */
    div_base = ((uint64_t)(1 << 24) * (AUD_GEN_FREQ)) / (mclk_freq * 2);

    /* fixed point multiplied with tolerance */
    aso_set_clk_div_base(aso->p_aso, (uint32_t)div_base);
    aso_set_mclk2bclk_ratio(aso->p_aso, mclk_freq / (aud_params->fs * 64));
    aso_set_bclk2lrclk_ratio(aso->p_aso, 64);

	/* set the parameters for the clk control */
    aso_set_clk_div_kp(aso->p_aso, div_kp);
    aso_set_clk_div_ki(aso->p_aso, div_ki);

    /* set fs, sample_width, channel count left and channel count right */
    aso_set_aud_params(aso->p_aso, aud_params);

    /* set denominator for missing packet check (fs * bit_width * ch_count * 1.0485)*/    
    aso_set_denominator(aso->p_aso, (uint32_t)((uint64_t)aud_params->fs * aud_params->sample_width * aud_chmap2cnt(aud_params->ch_map & 0xFF) * 1048576 / 1000000));

    aso->stream_status |= ASO_DRV_STATUS_AUD_PARAMS_SET;
    
    aud_report_params(aud_params, aso->stream_nr);

    return ERR_ASO_SUCCESS;
}

/** Gets the audio parameters
 *
 * @param aso pointer to the audio stream out handle struct
 * @param aud_params pointer to the audio parameter struct
 * @return error code
 */
int aso_drv_get_aud_params(t_aso* aso, struct hdoip_aud_params* aud_params)
{
    if((aso->stream_status & ASO_DRV_STATUS_AUD_PARAMS_SET) == 0) {
        return ERR_ASO_AUD_PARAMS_NOT_SET;
    }

    aso_get_aud_params(aso->p_aso, aud_params);
    return ERR_ASO_SUCCESS;
}

/** Sets the audio delay
 *
 * @param aso pointer to the audio stream out handle struct
 * @param aud_delay_us audio delay in microseconds
 * @return error code
 */
int aso_drv_set_aud_delay(t_aso* aso, uint32_t aud_delay_us)
{
    if((aso->stream_status & ASO_DRV_STATUS_ACTIV) != 0) {
        return ERR_ASO_RUNNING;
    }

    /* set audio delay */
    aso_set_aud_delay(aso->p_aso, aud_delay_us / AVPERIOD);

    aso->stream_status |= ASO_DRV_STATUS_AUD_DELAY_SET;

    return ERR_ASO_SUCCESS;
}

/** Gets the audio delay
 *
 * @param aso pointer to the audio stream out handle struct
 * @param aud_delay_us pointer to the audio delay in microseconds
 * @return error code
 */
int aso_drv_get_aud_delay(t_aso* aso, uint32_t* aud_delay_us)
{
    if((aso->stream_status & ASO_DRV_STATUS_AUD_DELAY_SET) == 0) {
        return ERR_ASO_AUD_DELAY_NOT_SET;
    }

    *aud_delay_us = aso_get_aud_delay(aso->p_aso) * AVPERIOD;
    return ERR_ASO_SUCCESS;
}

/** Gets the audio board id
 *
 * @param aso pointer to the audio stream out handle struct
 * @return audio board id
 */
uint32_t aso_drv_get_aud_id(void* p_aso)
{
    return aso_get_aud_id(p_aso) & 0x03;
}
