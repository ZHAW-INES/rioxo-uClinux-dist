#include "vsi_str.h"
#include "vsi_drv.h"
#include "stdeth.h"

char* vsi_str_err(int err)
{
    switch(err) {
        case ERR_VSI_SUCCESS: return "success";
        case ERR_VSI_RUNNING: return "is running";
        case ERR_VSI_PACKET_LENGTH_ERR: return "packet length not in range";
        case ERR_VSI_ETH_PARAMS_NOT_SET: return "ethernet parameters not set";
    }
    
    return "unknown";
}

void vsi_report_eth(t_vsi* handle) 
{
    struct hdoip_eth_params eth;
    vsi_drv_get_eth_params(handle, &eth);

    REPORT(INFO, "ctrl:   %8x", vsi_get_ctrl(handle->p_vsi, 0xffffffff));
    REPORT(INFO, "status: %8x", vsi_get_status(handle->p_vsi, 0xffffffff));
    eth_report_params(&eth, 0);
}

