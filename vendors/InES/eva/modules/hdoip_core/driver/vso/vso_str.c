#include "vso_str.h"

char* vso_str_err(int err) 
{
    switch(err) {
        case ERR_VSO_SUCCESS : return "success";
        case ERR_VSO_RUNNING : return "is running";
        case ERR_VSO_TIMING_NOT_SET : return "timing parameters not set";
    }
    return "unknown";
}

void vso_report_timing(t_vso* handle)
{
    REPORT(INFO, "Ethernet packet timeout : %d ns", vso_get_packet_timeout(handle->p_vso) * SPERIOD);
    REPORT(INFO, "Marker timeout          : %d ns", vso_get_marker_timeout(handle->p_vso) * SPERIOD);
    REPORT(INFO, "Timestamp min           : %d us", vso_get_timestamp_min(handle->p_vso) * AVPERIOD);    
    REPORT(INFO, "Timestamp max           : %d us", vso_get_timestamp_max(handle->p_vso) * AVPERIOD);   
    REPORT(INFO, "VS duration             : %d us", vso_get_vs_duration(handle->p_vso) * AVPERIOD);
    REPORT(INFO, "VS delay                : %d us", vso_get_vs_delay(handle->p_vso) * AVPERIOD);
    REPORT(INFO, "VSYNC delay             : %d ns", vso_get_vsync_delay(handle->p_vso) * SPERIOD);
    REPORT(INFO, "SCOMM5 delay            : %d ns", vso_get_scomm5_delay(handle->p_vso) * SPERIOD); 
}
