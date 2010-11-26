#include "std.h"
#include "vio_str.h"
#include "vio_reg.h"

char* vio_str_err(int err)
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
        case ERR_VIO_NO_PLL_CONFIG: return "no pll config found";
        case ERR_VIO_VCO_OUT_OF_RANGE: return "vco out of range";
        case ERR_VIO_FORMAT_NOT_SUPPORTED: return "format not supported";
        case ERR_VIO_INVALID_VALUE: return "invalid value";
    }

    return "unknown";
}

char* vio_str_pllc(uint32_t sel)
{
    switch(sel&VIO_MUX_PLLC_MASK) {
        case VIO_MUX_PLLC_FREE: return "no PLL control";
        case VIO_MUX_PLLC_SW: return "PLL controlled by software";
        case VIO_MUX_PLLC_TG: return "PLL controlled by timinggenerator";
    }
    
    return "unknown";
}

char* vio_str_stin(uint32_t sel)
{
    switch(sel&VIO_MUX_STIN_MASK) {
        case VIO_MUX_STIN_LOOP: return "set video i/o loop (input)";
        case VIO_MUX_STIN_PLAIN: return "set plain input encoding";
        case VIO_MUX_STIN_ADV212: return "set adv212 input encoding";
    }
    
    return "unknown";
}

char* vio_str_vout(uint32_t sel)
{
    switch(sel&VIO_MUX_VOUT_MASK) {
        case VIO_MUX_VOUT_LOOP: return "set video i/o loop (output)";
        case VIO_MUX_VOUT_PLAIN: return "set plain decoding";
        case VIO_MUX_VOUT_ADV212: return "set adv212 output decoding";
    }
    
    return "unknown";
}

char* vio_str_advcnt(uint32_t cfg)
{
    switch(cfg) {
        case 0: return "1x ADV212";
        case 1: return "2x ADV212";
        case 2: return "3x ADV212";
        case 3: return "4x ADV212";
    }
    
    return "unknown";
}

char* vio_str_format_cs(uint32_t f)
{
	switch(vio_format_cs(f)) {
		case CS_RGB: return "RGB";
		case CS_YUV: return "YUV";
		case CS_YUV_ER: return "YUV extended range";
	}

	return "unknown";
}

char* vio_str_format_sm(uint32_t f)
{
	switch(vio_format_sm(f)) {
		case SM_444: return "4:4:4";
		case SM_422: return "4:2:2";
	}

	return "unknown";
}


