
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
    aud_report_params(&aud_params, handle->stream_nr);

    REPORT(INFO, "Audio delay                : %d us", aso_get_aud_delay(handle->p_aso));
    REPORT(INFO, "Clock div. base value      : %d", aso_get_clk_div_base(handle->p_aso));
    REPORT(INFO, "Clock div. KP              : %d", aso_get_clk_div_kp(handle->p_aso));
    REPORT(INFO, "Clock div. KI              : 1/%d\n", aso_get_clk_div_ki(handle->p_aso));
    REPORT(INFO, "MCLK/BCLK ratio            : %d\n", aso_get_mclk2bclk_ratio(handle->p_aso));

    return ERR_ASO_SUCCESS; 
}

