
#include "aso_str.h"
#include "aso_hal.h"

char* aso_str_err(int err)
{
    switch(err) {
        case ERR_ASO_SUCCESS: return "success";
        case ERR_ASO_RUNNING: return "is running, but should not";
        case ERR_ASO_AUD_PARAMS_NOT_SET: return "audio parameters not set";
        case ERR_ASO_AUD_DELAY_NOT_SET: return "audio delay not set";
        case ERR_ASO_SAMPLE_WIDTH_ERR: return "sample width not in range";
        case ERR_ASO_CHANNEL_CNT_ERR: return "channel count not in range";
    }
    return "unknown";
}

int aso_str_report_aud(t_aso* handle)
{
    struct hdoip_aud_params aud_params;
    
    aso_get_aud_params(handle->p_aso, &aud_params);
    aud_report_params(&aud_params);

    REPORT(INFO, "Audio delay                : %d us", aso_get_aud_delay(handle->p_aso));
    REPORT(INFO, "Clock div. upper bound     : %d", aso_get_clk_div_upper_bound(handle->p_aso));
    REPORT(INFO, "Clock div. lower bound     : %d", aso_get_clk_div_lower_bound(handle->p_aso));
    REPORT(INFO, "Clock div. incremental     : %d", aso_get_clk_div_inc(handle->p_aso));

    return ERR_ASO_SUCCESS; 
}

