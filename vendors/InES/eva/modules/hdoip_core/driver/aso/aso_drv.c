
#include "stdrbf.h"
#include "aso_drv.h"

/** Initialize audio stream out driver 
 *
 * @param handle pointer to the audio stream out handle struct
 * @param p_aso address of the ACB audio stream out register
 * @return error code
 */
int aso_drv_init(t_aso* handle, void* p_aso)
{
    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| ASO-Driver: Initialize audio stream out          |");
    REPORT(INFO, "+--------------------------------------------------+");

    handle->p_aso = p_aso;
    handle->status = 0;

    aso_drv_stop(handle);

    aso_set_dma_burst_size(handle->p_aso, ASO_DRV_DMA_BURST_SIZE);
    aso_set_dma_fifo_almost_full(handle->p_aso, ASO_DRV_DMA_FIFO_ALMOST_FULL);

    /* disable miss filter => no dummy data */
    aso_set_fifo_low_th(handle->p_aso, ASO_DRV_DISABLE_FIFO_LOW_TH);
    aso_set_min_frames_buffered(handle->p_aso, ASO_DRV_DISABLE_FRAMES_BUFFERED);

    return ERR_ASO_SUCCESS;
}

/** Initialize the ringbuffer (read pointer)
 *
 * @param handle pointer to the aso handle
 * @param start_ptr start address of the buffer
 * @param size size of the buffer
 * @return error code
 */
int aso_drv_set_buf(t_aso* handle, void* start_ptr, size_t size) 
{
    t_rbf_dsc dsc;

    rbf_dsc(&dsc, start_ptr, size);
    aso_set_dsc(handle->p_aso, &dsc);

    return ERR_ASO_SUCCESS;
}

/** Flush the ringbuffer (set read to write pointer)
 *
 * @param handle pointer to the aso handle
 * @return error code
 */
int aso_drv_flush_buf(t_aso* handle)
{
    aso_set_read_dsc(handle->p_aso, aso_get_write_dsc(handle->p_aso));
    return ERR_ASO_SUCCESS;
}


/** Starts audio stream out (parameters must be set)
*
* @param handle pointer to the audio stream out handle struct
* @return error code
*/
int aso_drv_start(t_aso* handle)
{
    if((handle->status & ASO_DRV_STATUS_ACTIV) != 0) {
        return ERR_ASO_RUNNING;
    }

    if((handle->status & ASO_DRV_STATUS_AUD_PARAMS_SET) == 0) {
        return ERR_ASO_AUD_PARAMS_NOT_SET;
    }

    if((handle->status & ASO_DRV_STATUS_AUD_DELAY_SET) == 0) {
        return ERR_ASO_AUD_DELAY_NOT_SET;
    }

    aso_enable(handle->p_aso);

    handle->status = handle->status | ASO_DRV_STATUS_ACTIV;
    
    return ERR_ASO_SUCCESS;
}


/** Stops audio stream out 
*
* @param handle pointer to the audio stream out handle struct
* @return error code
*/
int aso_drv_stop(t_aso* handle)
{
    aso_disable(handle->p_aso);
    // FIXME:
    //while(aso_get_status(handle->p_aso, ASO_STATUS_IDLE) == 0);
    handle->status = handle->status & ~ASO_DRV_STATUS_ACTIV;
    return ERR_ASO_SUCCESS;
}

/** Updates all parameters during runtime
 *
 * @param handle pointer to the audio stream out handle struct
 * @param aud_params pointer to the audio parameter struct
 * @param aud_delay_us audio delay in microseconds
 * @return error code
 */
int aso_drv_update(t_aso* handle, struct hdoip_aud_params* aud_params, uint32_t aud_delay_us)
{
    uint32_t activ = handle->status & ASO_DRV_STATUS_ACTIV;
    uint32_t err;

    if(activ != 0) {
        aso_drv_stop(handle);
    }

    err = aso_drv_set_aud_params(handle, aud_params);
    if(err != ERR_ASO_SUCCESS) {
        return err;
    }

    err = aso_drv_set_aud_delay(handle, aud_delay_us);
    if(err != ERR_ASO_SUCCESS) {
        return err;
    }

    if(activ != 0) {
        aso_drv_start(handle);
    }

    return ERR_ASO_SUCCESS;
}

/** Audio stream out handler
 *
 * @param handle pointer to the audio stream out handle struct
 * @param event_queue pointer to the event queue
 * @return error code
 */
int aso_drv_handler(t_aso* handle, t_queue* event_queue)
{
    uint32_t status = aso_get_status(handle->p_aso, ASO_STATUS_FRAME_SIZE_ERROR | ASO_STATUS_FIFO_EMPTY | ASO_STATUS_FIFO_FULL);
    uint32_t status_reordering = aso_get_reordering_status(handle->p_aso, ASO_REORDERING_STATUS_RAM_FULL);

    if((status & ASO_STATUS_FRAME_SIZE_ERROR) != 0) {
        if((handle->status & ASO_DRV_STATUS_SIZE_ERROR) == 0) {
            queue_put(event_queue, E_ASO_SIZE_ERROR);
        }
        handle->status = handle->status | ASO_DRV_STATUS_SIZE_ERROR;
    } else {
        handle->status = handle->status & ~ASO_DRV_STATUS_SIZE_ERROR;
    }

    if((status & ASO_STATUS_FIFO_EMPTY) != 0) {
        if((handle->status & ASO_DRV_STATUS_FIFO_EMPTY) == 0) {
            queue_put(event_queue, E_ASO_FIFO_EMPTY);
        }
        handle->status = handle->status | ASO_DRV_STATUS_FIFO_EMPTY;
    } else {
        handle->status = handle->status & ~ASO_DRV_STATUS_FIFO_EMPTY;
    }

    if((status & ASO_STATUS_FIFO_FULL) != 0) {
        if((handle->status & ASO_DRV_STATUS_FIFO_FULL) == 0) {
            queue_put(event_queue, E_ASO_FIFO_FULL);
        }
        handle->status = handle->status | ASO_DRV_STATUS_FIFO_FULL;
    } else {
        handle->status = handle->status &  ~ASO_DRV_STATUS_FIFO_FULL;
    }

    if((status_reordering & ASO_REORDERING_STATUS_RAM_FULL) != 0) {
        if((handle->status & ASO_DRV_STATUS_RAM_FULL) == 0) {
            queue_put(event_queue, E_ASO_RAM_FULL);
        }
        handle->status = handle->status | ASO_DRV_STATUS_RAM_FULL;
    } else {
        handle->status = handle->status & ~ASO_DRV_STATUS_RAM_FULL;
    }

    return ERR_ASO_SUCCESS;
}

/** Sets the audio parameters 
 *
 * @param handle pointer to the audio stream out handle struct
 * @param aud_params pointer to the audio parameter struct
 * @return error code
 */
int aso_drv_set_aud_params(t_aso* handle, struct hdoip_aud_params* aud_params)
{
    uint64_t div_base, div_lower_bound, div_upper_bound;
    uint32_t div_inc;

    if((handle->status & ASO_DRV_STATUS_ACTIV) != 0) {
        return ERR_ASO_RUNNING;
    }

    /* calculate i2s clock (in q5.27 format) */

    /* 64*fs = output frequency. division by 2 because clock signal toggles after division factor */
    div_base = ((uint64_t)(1 << 27) * SFREQ) / (aud_params->fs * 64 * 2); 
    
    /* fixed point multiplied with tolerance */
    div_upper_bound = div_base * ((uint64_t)ASO_DRV_I2S_FREQ_FACT_UPPER);
    div_upper_bound = div_upper_bound >> 27;

    div_lower_bound = div_base * ((uint64_t)ASO_DRV_I2S_FREQ_FACT_LOWER);  
    div_lower_bound = div_lower_bound >> 27;

    aso_set_clk_div_upper_bound(handle->p_aso, (uint32_t)div_upper_bound);
    aso_set_clk_div_lower_bound(handle->p_aso, (uint32_t)div_lower_bound);
    aso_set_clk_div_act(handle->p_aso, (uint32_t)div_base);

    /* calculate incremental value*/
    div_inc = (uint32_t)div_upper_bound - (uint32_t)div_base;
    div_inc = div_inc / ASO_DRV_I2S_FREQ_TOL_DIV; 
    aso_set_clk_div_inc(handle->p_aso, div_inc);

    /* set fs, sample_width, channel count left and channel count right */
    aso_set_aud_params(handle->p_aso, aud_params);
    
    handle->status = handle->status | ASO_DRV_STATUS_AUD_PARAMS_SET;
    
    aud_report_params(aud_params);

    return ERR_ASO_SUCCESS;
}

/** Gets the audio parameters
 *
 * @param handle pointer to the audio stream out handle struct
 * @param aud_params pointer to the audio parameter struct
 * @return error code
 */
int aso_drv_get_aud_params(t_aso* handle, struct hdoip_aud_params* aud_params)
{
    if((handle->status & ASO_DRV_STATUS_AUD_PARAMS_SET) == 0) {
        return ERR_ASO_AUD_PARAMS_NOT_SET;
    }

    aso_get_aud_params(handle->p_aso, aud_params); 
    return ERR_ASO_SUCCESS;
}

/** Sets the audio delay
 *
 * @param handle pointer to the audio stream out handle struct
 * @param aud_delay_us audio delay in microseconds
 * @return error code
 */
int aso_drv_set_aud_delay(t_aso* handle, uint32_t aud_delay_us) 
{
    if((handle->status & ASO_DRV_STATUS_ACTIV) != 0) {
        return ERR_ASO_RUNNING;
    }

    /* set audio delay */
    aso_set_aud_delay(handle->p_aso, aud_delay_us / AVPERIOD);    

    handle->status = handle->status | ASO_DRV_STATUS_AUD_DELAY_SET;

    return ERR_ASO_SUCCESS;
}

/** Gets the audio delay
 *
 * @param handle pointer to the audio stream out handle struct
 * @param aud_delay_us pointer to the audio delay in microseconds
 * @return error code
 */
int aso_drv_get_aud_delay(t_aso* handle, uint32_t* aud_delay_us)
{
    if((handle->status & ASO_DRV_STATUS_AUD_DELAY_SET) == 0) {
        return ERR_ASO_AUD_DELAY_NOT_SET;
    }

    *aud_delay_us = aso_get_aud_delay(handle->p_aso) * AVPERIOD;
    return ERR_ASO_SUCCESS;
}
