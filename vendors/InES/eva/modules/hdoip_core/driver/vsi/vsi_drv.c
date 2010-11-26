#include "vsi_drv.h"

/** Stops VSI FSM 
 *
 * @param handle pointer to the vsi handle
 * @return error code
 */
int vsi_drv_stop(t_vsi* handle) 
{
	vsi_disable(handle->p_vsi);
    
	while(vsi_get_status(handle->p_vsi, VSI_ST_FSM_IDLE) == 0); 
    handle->status = handle->status & ~VSI_DRV_STATUS_ACTIV;

	return ERR_VSI_SUCCESS;
}

/** Starts VSI FSM 
 *
 * @param handle pointer to the vsi handle
 * @return error code
 */
int vsi_drv_start(t_vsi* handle) 
{
    if((handle->status & VSI_DRV_STATUS_ETH_PARAMS_SET) == 0) {
        return ERR_VSI_ETH_PARAMS_NOT_SET;
    }

	vsi_enable(handle->p_vsi);
    handle->status = handle->status | VSI_DRV_STATUS_ACTIV;

	return ERR_VSI_SUCCESS;
}

/** Initialize VSI core driver
 *
 * @param handle pointer to the vsi handle
 * @param p_vsi pointer to the ACB registers
 * @return error code
 */
int vsi_drv_init(t_vsi* handle, void* p_vsi) 
{
    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| VSI-Driver: Initialize video stream in           |");
    REPORT(INFO, "+--------------------------------------------------+");
    
	handle->p_vsi = p_vsi;
	handle->status = 0;

	vsi_drv_stop(p_vsi);
	vsi_set_burst_size(p_vsi, VSI_DRV_DMA_BURST_SIZE);

	return ERR_VSI_SUCCESS;
}

int vsi_drv_update(t_vsi* handle, struct hdoip_eth_params* eth_params)
{
    uint32_t activ = handle->status & VSI_DRV_STATUS_ACTIV;
    uint32_t err;

    if(activ != 0) {
        vsi_drv_stop(handle);
    }

    err = vsi_drv_set_eth_params(handle, eth_params);
    if(err != ERR_VSI_SUCCESS) {
        return err;
    }

    if(activ != 0) {
        vsi_drv_start(handle);
    }
    return ERR_VSI_SUCCESS;
}

/** Read all network parameters from registers in VSI. Parameters are written in handle (t_vsi)
 *
 * @param handle pointer to the vsi handle
 * @return error code
 */
int vsi_drv_get_eth_params(t_vsi* handle, struct hdoip_eth_params* eth_params)
{
    if((handle->status & VSI_DRV_STATUS_ETH_PARAMS_SET) == 0) { 
        return ERR_VSI_ETH_PARAMS_NOT_SET;
    }

    vsi_get_eth_params(handle->p_vsi, eth_params);
    
    memcpy(eth_params, &(handle->eth_params), sizeof(struct hdoip_eth_params));

    return ERR_VSI_SUCCESS;
}

/** Write all network parameters to registers in VSI. 
 *
 * @param handle pointer to the vsi handle
 * @return error code
 */
int vsi_drv_set_eth_params(t_vsi* handle, struct hdoip_eth_params* eth_params) 
{
    if((handle->status & VSI_DRV_STATUS_ACTIV) != 0) {
        return ERR_VSI_RUNNING;
    }
   
    if((handle->eth_params.packet_length < VSI_DRV_MIN_PACKET_LEN) || (handle->eth_params.packet_length > VSI_DRV_MAX_PACKET_LEN)) {
        return ERR_VSI_PACKET_LENGTH_ERR;
    }

    vsi_set_eth_params(handle->p_vsi, eth_params);
	
    memcpy(&(handle->eth_params), eth_params, sizeof(struct hdoip_eth_params));
    handle->status = handle->status | VSI_DRV_STATUS_ETH_PARAMS_SET;

    REPORT(INFO, "VSI network parameters set:");
    vsi_report_eth(handle);

	return ERR_VSI_SUCCESS;
}

/** VSI core driver handler 
 *
 * @param handle pointer to the vsi handle
 * @param event_queue pointer to the event queue
 * @return error code
 */
int vsi_drv_handler(t_vsi* handle, t_queue* event_queue) 
{
	uint32_t status = vsi_get_status(handle->p_vsi, VSI_ST_MSK); 	

	/* FIFO2 error */
	if((status & VSI_ST_FIFO2_FULL) != 0) {
		if((handle->status & VSI_DRV_STATUS_FIFO2_FULL) == 0) {
			queue_put(event_queue, E_VSI_FIFO2_FULL);
		}
		
		handle->status = (handle->status | VSI_DRV_STATUS_FIFO2_FULL);
	} else {
		handle->status = (handle->status & (~VSI_DRV_STATUS_FIFO2_FULL));
	}

	/* code FIFO error */
	if((status & VSI_ST_CDFIFO_FULL) != 0) {
		if((handle->status & VSI_DRV_STATUS_CDFIFO_FULL) == 0) {
			queue_put(event_queue, E_VSI_CDFIFO_FULL);
		}
		
		handle->status = (handle->status | VSI_DRV_STATUS_CDFIFO_FULL);
	} else {
		handle->status = (handle->status & (~VSI_DRV_STATUS_CDFIFO_FULL));
	}

	return ERR_VSI_SUCCESS;
}

