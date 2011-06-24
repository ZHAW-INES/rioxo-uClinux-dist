#include "vsi_drv.h"
#include "debug.h"

/** Stops VSI FSM 
 *
 * @param handle pointer to the vsi handle
 * @return error code
 */
int vsi_drv_stop(t_vsi* handle) 
{
    PTR(handle); PTR(handle->p_vsi);
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
    PTR(handle); PTR(handle->p_vsi);
    if((handle->status & VSI_DRV_STATUS_ETH_PARAMS_SET) == 0) {
        return ERR_VSI_ETH_PARAMS_NOT_SET;
    }
    REPORT(INFO, "VSI_DRV_START");
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
    PTR(handle); PTR(p_vsi);
    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| VSI-Driver: Initialize video stream in           |");
    REPORT(INFO, "+--------------------------------------------------+");
    
	handle->p_vsi = p_vsi;
	handle->status = 0;

	vsi_drv_stop(handle);
	vsi_set_burst_size(p_vsi, VSI_DRV_DMA_BURST_SIZE);

	return ERR_VSI_SUCCESS;
}

/** Inittialize the ringbuffer (write pointer)
 *
 * @param handle pointer to the vsi handle
 * @param start_ptr start address of the buffer
 * @param size size of the buffer
 * @return error code
 */
int vsi_drv_set_buf(t_vsi* handle, void* start_ptr, size_t size)
{
    t_rbf_dsc dsc;

    PTR(handle); PTR(handle->p_vsi); PTR(start_ptr);

    rbf_dsc(&dsc, start_ptr, size);
    vsi_set_dsc(handle->p_vsi, &dsc);

    return ERR_VSI_SUCCESS;
}

/** Flushs the buffer (set write to read pointer)
 *
 * @param handle pointer to the vsi handle
 * @return error code
 */
int vsi_drv_flush_buf(t_vsi* handle)
{
    PTR(handle); PTR(handle->p_vsi);
    vsi_set_write_dsc(handle->p_vsi, vsi_get_read_dsc(handle->p_vsi));
    return ERR_VSI_SUCCESS; 
}

int vsi_drv_update(t_vsi* handle, struct hdoip_eth_params* eth_params)
{
    uint32_t activ, err;

    PTR(handle); PTR(handle->p_vsi); PTR(eth_params);
    activ = handle->status & VSI_DRV_STATUS_ACTIV;

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

int vsi_drv_go(t_vsi* handle, struct hdoip_eth_params* eth)
{
    int n;

    n = vsi_drv_update(handle, eth);
    REPORT(INFO, "vsi_drv_update: %s", vsi_str_err(n));
    n = vsi_drv_start(handle);
    REPORT(INFO, "vsi_drv_start: %s", vsi_str_err(n));

    vsi_report_eth(handle);

    return n;
}

/** Read all network parameters from registers in VSI. Parameters are written in handle (t_vsi)
 *
 * @param handle pointer to the vsi handle
 * @return error code
 */
int vsi_drv_get_eth_params(t_vsi* handle, struct hdoip_eth_params* eth_params)
{
    vsi_get_eth_params(handle->p_vsi, &(handle->eth_params));
    
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
   
    if((eth_params->packet_size < VSI_DRV_MIN_PACKET_LEN) || (eth_params->packet_size > VSI_DRV_MAX_PACKET_LEN)) {
        return ERR_VSI_PACKET_LENGTH_ERR;
    }

    vsi_set_eth_params(handle->p_vsi, eth_params);
	
    memcpy(&(handle->eth_params), eth_params, sizeof(struct hdoip_eth_params));
    handle->status = handle->status | VSI_DRV_STATUS_ETH_PARAMS_SET;

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
    uint32_t status;

    PTR(handle); PTR(handle->p_vsi); PTR(event_queue);
	status = vsi_get_status(handle->p_vsi, VSI_ST_MSK);

	if (handle->status & VSI_DRV_STATUS_ACTIV) {
/*        static int xx=0;
        if (!xx && (vsi_get_start_dsc(handle->p_vsi)!=vsi_get_write_dsc(handle->p_vsi))){xx=1;
            printk("\n Master|Slave ( %d|%d) \n", HOI_RD32(na_acb_timer|0x80000000, 12), HOI_RD32(na_acb_timer|0x80000000, 16));
            uint32_t *p = vsi_get_start_dsc(handle->p_vsi)|0x80000000;
            //for (int i=0;i<16;i++) printk("\n[%8x]",p[i]);
            printk("\n Frame to send: %d\n", p[13]);
        }
*/
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
	}

	return ERR_VSI_SUCCESS;
}

