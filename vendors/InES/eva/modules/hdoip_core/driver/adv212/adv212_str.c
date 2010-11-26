#include "adv212_str.h"

char* adv212_str_err(int err)
{
    switch(err) {
        case ERR_ADV212_SUCCESS: return "success";
        case ERR_ADV212_BOOT_FAIL: return "boot failed";
        case ERR_ADV212_BOOT_TIMEOUT: return "boot timeout";
        case ERR_ADV212_NULL_POINTER: return "null pointer";
        case ERR_ADV212_PIXEL_CLOCK_TOOHIGH: return "pixel clock too high";
        case ERR_ADV212_PIXEL_COUNT_TOOHIGH: return "pixel count too high";
        case ERR_ADV212_SAMPLE_RATE_TOOHIGH: return "sample rate too high";
        case ERR_ADV212_ADVCNT_OUT_OF_RANGE: return "advcnt out of possible range";
    }
    
    return "unknown error";
}
