#include "vsi_str.h"
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
   eth_report_params(&(handle->eth_params)); 
}

