
#include "stdrbf.h"
#include "vso_drv.h"
#include "sta_hal.h"
#include "debug.h"

static int vso_drv_clear_reordering(t_vso* handle) 
{
    PTR(handle);
    PTR(handle->p_vso);

	vso_start_clear(handle->p_vso);
	udelay(VSO_DRV_CLEAR_TIME);
	vso_stop_clear(handle->p_vso);	

	return ERR_VSO_SUCCESS;
}

/** Starts VSO FSM
 *
 * @param handle pointer to the vso handle
 * @return error code
 */
int vso_drv_start(t_vso* handle) 
{
    PTR(handle);
    PTR(handle->p_vso);
/*
    if((handle->status & VSO_DRV_STATUS_TIMING_SET) == 0) {
        return ERR_VSO_TIMING_NOT_SET;
    }*/
    REPORT(INFO, "VSO_DRV_START");
	vso_enable(handle->p_vso);
    handle->status = handle->status | VSO_DRV_STATUS_ACTIV;
	return ERR_VSO_SUCCESS;
}


/** Stops VSO FSM and clears reordering RAM
 *
 * @param handle pointer to the vso handle
 * @return error code
 */
int vso_drv_stop(t_vso* handle)
{
    PTR(handle);
    PTR(handle->p_vso);

	vso_disable(handle->p_vso);
  	vso_drv_clear_reordering(handle);	
    handle->status = handle->status & ~VSO_DRV_STATUS_ACTIV;
	return ERR_VSO_SUCCESS;
}

/** Initialize VSO core driver
 *
 * @param handle pointer to the vso handle
 * @param p_vso pointer to the ACB registers
 * @param packet_timeout_ns ethernet packet timeout value in nanoseconds
 * @param vid_timing actual video timing structure
 * @param vs_delay_us video stream delay in microseconds
 * @param vsync_delay_ns VSYNC pulse delay in nanoseconds
 * @param scomm5_delay_ns SCOMM5 signla delay in nanoseconds
 * @return error code
 */
int vso_drv_init(t_vso* handle, void* p_vso) 
{
    PTR(handle);
    PTR(p_vso);

    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| VSO-Driver: Initialize video stream out          |");
    REPORT(INFO, "+--------------------------------------------------+");
    

	handle->p_vso = p_vso;
	handle->status = 0;

	vso_drv_stop(handle);

 	vso_set_dma_burst_size(p_vso, VSO_DRV_DMA_BURST_SIZE);
	vso_set_dma_almost_full(p_vso, VSO_DRV_DMA_ALMOST_FULL);

	return ERR_VSO_SUCCESS;
}

/** Initialize the ringbuffer (read pointer)
 *
 * @param handle pointer to the vso handle
 * @param start_ptr start address of the buffer
 * @param size size of the buffer
 * @return error code
 */
int vso_drv_set_buf(t_vso* handle, void* start_ptr, size_t size)
{
    t_rbf_dsc dsc;

    PTR(handle);    PTR(handle->p_vso);    PTR(start_ptr);

    rbf_dsc(&dsc, start_ptr, size);
    vso_set_dsc(handle->p_vso, &dsc);

    return ERR_VSO_SUCCESS;
}

/** Flush the ringbuffer (set read to write pointer)
 * 
 * @param handle pointer to the vso handle
 * @return error code
 */

int vso_drv_flush_buf(t_vso* handle)
{
    PTR(handle);
    PTR(handle->p_vso);
    vso_set_read_dsc(handle->p_vso, vso_get_write_dsc(handle->p_vso));
    return ERR_VSO_SUCCESS;
}

/** Returns the ethernet packet timeout value in nanoseconds 
 *
 * @param handle pointer to the vso handle
 * @return ethernet packet timeout in nanoseconds
 */
uint32_t vso_drv_get_packet_timeout(t_vso* handle) 
{
	return vso_get_packet_timeout(handle->p_vso) * SPERIOD; // in ns 
}

/** Returns all video delays
 *
 * @param handle pointer to the vso handle
 * @param vs_delay_us pointer to video stream delay variable (value in microseconds)
 * @param vsync_delay_ns pointer to VSYNC delay variable (value in nanoseconds)
 * @param scomm5_delay_ns pointer to SCOMM5 signal delay variable (value in nanoseconds)
 * @return error code
 */
int vso_drv_get_delays(t_vso* handle, uint32_t* vs_delay_us, uint32_t* vsync_delay_ns, uint32_t* scomm5_delay_ns) 
{
    PTR(vs_delay_us);
    PTR(vsync_delay_ns);
    PTR(scomm5_delay_ns);

	*vs_delay_us = vso_get_vs_delay(handle->p_vso) * AVPERIOD;
	*vsync_delay_ns = vso_get_vsync_delay(handle->p_vso) * SPERIOD;
	*scomm5_delay_ns = vso_get_scomm5_delay(handle->p_vso) * SPERIOD;
	return ERR_VSO_SUCCESS;	
}

/** Updates all video timing parameters 
 *
 * @param handle pointer to the vso handle 
 * @param vid_timing pointer to video timing structure
 * @param vs_delay_us video stream delay value in microseconds
 * @param vsync_delay_ns VSYNC signal delay value in nanoseconds
 * @param scomm5_delay_ns SCOMM5 signal delay value in nanoseconds
 * @param packet_timeout_ns packet timeout value in nanoseconds
 * @return error code
 */
int vso_drv_update(t_vso* handle, t_video_timing* vid_timing, uint32_t vs_delay_us, uint32_t vsync_delay_ns, uint32_t scomm5_delay_ns, uint32_t packet_timeout_ns) 
{
    uint32_t duration_us, duration_ns;
    uint32_t p_vso;

    PTR(handle);     PTR(handle->p_vso);    PTR(vid_timing);

	p_vso = (uint32_t) handle->p_vso;

	/* set video timing */
	duration_us = vid_duration_in_us(vid_timing);
	duration_ns = vid_duration_in_ns(vid_timing);

	vso_set_vs_duration(p_vso, duration_us / AVPERIOD);
	vso_set_marker_timeout(p_vso, ((duration_ns / SPERIOD) - ISYSTIME(VSO_DRV_MTIMEOUT_MARGIN)));
	vso_set_timestamp_min(p_vso, (duration_us - (duration_us * VSO_DRV_TIMESTAMP_TOLERANCE / 100)) / AVPERIOD);	
	vso_set_timestamp_max(p_vso, (duration_us + (duration_us * VSO_DRV_TIMESTAMP_TOLERANCE / 100)) / AVPERIOD);

	/* set delays */
	vso_set_vs_delay(p_vso, vs_delay_us / AVPERIOD); 
	vso_set_vsync_delay(p_vso, (vsync_delay_ns / SPERIOD)); /* f=100MHz => 10ns ticks */
	vso_set_scomm5_delay(p_vso, (scomm5_delay_ns / SPERIOD)); /* f=100MHz => 10ns ticks */	

    /* set packet timeout */ 
	vso_set_packet_timeout(handle->p_vso, (packet_timeout_ns / SPERIOD)); /* f=100MHz => 10ns ticks */	

    handle->status = handle->status | VSO_DRV_STATUS_TIMING_SET;

    vso_report_timing(handle);

    return ERR_VSO_SUCCESS;
}

static int vso_drv_put_event(t_vso* handle, t_queue* event_queue, uint32_t status, uint32_t mask, uint32_t statusbit, uint32_t event_true, uint32_t event_false)
{
	if((status & mask) != 0) { /* true */ 
		if((handle->status & statusbit) == 0) {
			if(event_true != 0) {
				queue_put(event_queue, event_true);
			}
			handle->status = handle->status | statusbit;
		}
	} else { /* false */
		if((handle->status & statusbit) == 1) {
			if(event_false != 0) {
				queue_put(event_queue, event_false);
			}
			handle->status = handle->status & (~statusbit);
		}

	}
	return ERR_VSO_SUCCESS;
}

/** vso core driver handler
 * 
 * @param handle pointer to vso handle
 * @param event_queue pointer to event queue
 * @return error code
 */
int vso_drv_handler(t_vso* handle, t_queue* event_queue) 
{
    uint32_t status;

    PTR(handle);    PTR(handle->p_vso);    PTR(event_queue);

	status = vso_get_status(handle->p_vso, VSO_ST_MSK);

    /* Error: choked */
	vso_drv_put_event(handle, event_queue, status, VSO_ST_CHOKED, VSO_DRV_STATUS_CHOKED, E_VSO_CHOKED, 0);

	/* Error: payload empty */
	vso_drv_put_event(handle, event_queue, status, VSO_ST_PAYLOAD_EMPTY, VSO_DRV_STATUS_PAYLOAD_EMPTY, E_VSO_PAYLOAD_EMPTY, 0);

	/* Error: payload marker timeout */
	vso_drv_put_event(handle, event_queue, status, VSO_ST_PAYLOAD_MTIMEOUT, VSO_DRV_STATUS_PAYLOAD_MTIMEOUT, E_VSO_PAYLOAD_MTIMEOUT, 0);

	/* Error: timestamp error */
	vso_drv_put_event(handle, event_queue, status, VSO_ST_TIMESTAMP_ERROR, VSO_DRV_STATUS_TIMESTAMP_ERROR, E_VSO_TIMESTAMP_ERROR, 0);

	// started streaming
	vso_drv_put_event(handle, event_queue, status, VSO_ST_ACTIVE, VSO_DRV_STATUS_ST_ACTIVE, E_VSO_ST_ACTIVE, 0);

	return ERR_VSO_SUCCESS;
}


void vso_set_vsync_blanking(t_vso* handle, t_video_timing* timing)
{
    // Disable continuous vsync blanking (enable only for debug)
    vso_clr_ctrl(handle->p_vso, VSO_CTRL_VSYNC_OFF);

    // Select source of vsync blanking (1=field-signal / 0=timer that blanks every second vsync-pulse)
    vso_clr_ctrl(handle->p_vso, VSO_CTRL_VSYNC_BLANK_SOURCE);
    vso_clr_ctrl(handle->p_vso, VSO_CTRL_FIELD_POL);

    // Enable blanking if interlaced
    if (timing->interlaced) {
        vso_set_ctrl(handle->p_vso, VSO_CTRL_INTERLACE);
    }
    else {
        vso_clr_ctrl(handle->p_vso, VSO_CTRL_INTERLACE);
    }
}
