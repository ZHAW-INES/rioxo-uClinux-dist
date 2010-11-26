
#include "asi_str.h"
#include "asi_hal.h"

/** Translate error code to string
 *
 * @param err error code
 * @return error string
 */
char* asi_str_err(int err) 
{
    switch(err) {
        case ERR_ASI_SUCCESS: return "success";
        case ERR_ASI_RUNNING: return "is running, but should not";
        case ERR_ASI_ETH_PARAMS_NOT_SET: return "ethernet parameters not set";
        case ERR_ASI_AUD_PARAMS_NOT_SET: return "audio parameters not set";
        case ERR_ASI_PACKET_LENGTH_ERR: return "ethernet packet length not in range";
        case ERR_ASI_AUD_PARAMS_ERR: return "audio parameters not in range";
    }

    return "unknown";
}

/** Reads ethernet parameter from register and print it to console
 * 
 * @param handle pointer to the audio stream in struct
 * @return error code
 */
int asi_str_report_eth(t_asi* handle)
{     
    struct hdoip_eth_params eth_params;

    asi_get_eth_params(handle->p_asi, &eth_params);
    eth_report_params(&eth_params);
   
    return ERR_ASI_SUCCESS;
}

/** Reads audio parameter from register and print it to console
 * 
 * @param handle pointer to the audio stream in struct
 * @return error code
 */
int asi_str_report_aud(t_asi* handle)
{
    struct hdoip_aud_params aud_params;

    asi_get_aud_params(handle->p_asi, &aud_params);
    aud_report_params(&aud_params);

    return ERR_ASI_SUCCESS;
}
