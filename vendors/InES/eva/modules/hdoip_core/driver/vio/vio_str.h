#ifndef VIO_STR_H_
#define VIO_STR_H_

#include "stdvid.h"
#include "vio_reg.h"
#include "vio_cfg.h"


/** report macros
 * 
 */

#define VIO_REPORT_ERROR(err) { \
    REPORT(INFO, "vio error: %s", vio_str_err(err)); \
}

#define VIO_REPORT_STATISTIC(p) { \
    REPORT(INFO, "<VIO I/O Count Statistic>"); \
    REPORT(INFO, "  video in  : %d", vio_get_statistic_vid_in(p)); \
    REPORT(INFO, "  video out : %d", vio_get_statistic_vid_out(p)); \
    REPORT(INFO, "  stream in : %d", vio_get_statistic_st_in(p)); \
    REPORT(INFO, "  stream out: %d", vio_get_statistic_st_out(p)); \
}

#define VIO_REPORT_TIMING(p) { \
DBG( \
    t_video_timing* _vt_ = (p); \
    REPORT(INFO, "<Timing> %d x %d @ %d Hz", _vt_->width, _vt_->height, vid_fps(_vt_)); \
    REPORT(INFO, "  H: (%d) %d-%d-%d", _vt_->width, _vt_->hfront, _vt_->hpulse, _vt_->hback); \
    REPORT(INFO, "  V: (%d) %d-%d-%d", _vt_->height, _vt_->vfront, _vt_->vpulse, _vt_->vback); \
) \
}

#define VIO_REPORT_TRANSFORM(p, s) { \
DBG( \
	REPORT(INFO, "<Transform %s> format is 3.13", s); \
	REPORT(INFO, "  Downsampling: %s - reverse order: %s", DBGSTA(HOI_RD32(p, VIO_OFF_PP_CFG)&VIO_PP_DOWN), DBGSTA(HOI_RD32(p, VIO_OFF_PP_CFG)&VIO_PP_DFIRST) );\
	REPORT(INFO, "  Upsampling  : %s - reverse order: %s", DBGSTA(HOI_RD32(p, VIO_OFF_PP_CFG)&VIO_PP_UP), DBGSTA(HOI_RD32(p, VIO_OFF_PP_CFG)&VIO_PP_UFIRST) );\
	REPORT(INFO, "    +----------------+    +------+"); \
	REPORT(INFO, "    | %04x %04x %04x |    | %04x |", HOI_RD16(p, 0), HOI_RD16(p, 2), HOI_RD16(p, 4), HOI_RD16(p,18));\
	REPORT(INFO, "    | %04x %04x %04x |    | %04x |", HOI_RD16(p, 6), HOI_RD16(p, 8), HOI_RD16(p,10), HOI_RD16(p,20));\
	REPORT(INFO, "    | %04x %04x %04x |    | %04x |", HOI_RD16(p,12), HOI_RD16(p,14), HOI_RD16(p,16), HOI_RD16(p,22));\
	REPORT(INFO, "    +----------------+    +------+");\
) \
}

#define VIO_REPORT_CONTROL(p, ppm) { \
    REPORT(INFO, "<PLL Control> Status: %s - Mode: %s: %d ppm", \
        DBGSTA(vio_get_cfg(p,VIO_CFG_PLL_CTRL)), \
        vio_str_pllc(vio_get_pllc(p)), (ppm)); \
    REPORT(INFO, "  Threshold: %d / %d", HOI_RD32(p, VIO_OFF_TG_THRESHOLD), HOI_RD32(p, VIO_OFF_TG_SCALE) ); \
}

#define VIO_REPORT_OSD(p) { \
    REPORT(INFO, "<OSD> %d x %d Characters", vio_osd_get_width(p), vio_osd_get_height(p)); \
    REPORT(INFO, "  Character: %d x %d Pixel", HOI_RD16(p, VIO_OFF_OSD_CHAR_WIDTH)+1, HOI_RD16(p, VIO_OFF_OSD_CHAR_HEIGHT)+1); \
}

#define VIO_REPORT_FONT(p) { \
	REPORT(INFO, "<Font> %d x %d Pixel, %d Character codes starting from %d", (p)->width, (p)->height, (p)->length, (p)->start); \
}

#define VIO_REPORT_FORMAT(p, s) { \
	REPORT(INFO, "<%s format> %s %s", s, vio_str_format_cs(p), vio_str_format_sm(p)); \
}

#define VIO_REPORT_BASE(p) { \
	REPORT(INFO, "<VIO Handle>"); \
	REPORT(INFO, "  video i/o base address    = 0x%08x", (uint32_t)(p)->p_vio); \
	REPORT(INFO, "  adv212 hdata base address = 0x%08x", (uint32_t)(p)->p_adv); \
}

#define VIO_REPORT_MUX(p) { \
    REPORT(INFO, "<Setup MUX> Input: %s - Output: %s - Compressor: %s", \
        DBGSTA(vio_get_cfg(p,VIO_CFG_VIN)), \
        DBGSTA(vio_get_cfg(p,VIO_CFG_VOUT|VIO_CFG_OVERLAY)), \
        DBGSTA(vio_get_cfg(p,VIO_CFG_ADV_ENC|VIO_CFG_ADV_DEC)) ); \
    REPORT(INFO, "  VIO run: %s", \
        DBGSTA(vio_get_cfg(p,VIO_CFG_RUN)) \
    ) \
	DBGIF(vio_get_cfg(p,VIO_CFG_VIN), \
        REPORT(INFO, "  Input: %s", vio_str_stin(vio_get_stin(p))); \
    ) \
    DBGIF(vio_get_cfg(p,VIO_CFG_VOUT), \
        REPORT(INFO, "  Output: %s", vio_str_vout(vio_get_vout(p))); \
    ) \
    DBGIF(vio_get_cfg(p,VIO_CFG_ADV_ENC), \
        REPORT(INFO, "  Encoding using %s", vio_str_advcnt(vio_get_advcnt(p))); \
    ) \
    DBGIF(vio_get_cfg(p,VIO_CFG_ADV_DEC), \
        REPORT(INFO, "  Decoding using %s", vio_str_advcnt(vio_get_advcnt(p))); \
    ) \
    DBGIF(vio_get_cfg(p,VIO_CFG_PLAIN_DEC), \
        REPORT(INFO, "  Plain output enabled"); \
    ) \
    DBGIF(vio_get_cfg(p,VIO_CFG_PLAIN_ENC), \
        REPORT(INFO, "  Plain input enabled (size = %d Byte/Frame)", vio_get_size(p)); \
    ) \
    REPORT(INFO, "  OSD: %s", \
        DBGSTA(vio_get_cfg(p,VIO_CFG_OVERLAY)) \
    ) \
    REPORT(INFO, "  VRP: %s", \
        DBGSTA(vio_get_proc(p)) \
    ) \
    REPORT(INFO, "  wait VSync: %s", \
        DBGSTA(vio_get_cfg(p,VIO_CFG_WAITVS)) \
    ) \
    REPORT(INFO, "  hsplit: %d", vio_get_hsplit(p)); \
}

#define VIO_REPORT_PLL(p) { \
    REPORT(INFO, "<Setup PLL> In: %d Hz - Out: %d Hz (%d ppm) - Sys: %d Hz", (p)->fin[(p)->insel], (p)->fout[0], (p)->ppm, (p)->fsys); \
    REPORT(INFO, "  [0] = %d Hz, c = %d", (p)->fout[0], (p)->c[0]) \
    REPORT(INFO, "  [1] = %d Hz, c = %d", (p)->fout[1], (p)->c[1]) \
    REPORT(INFO, "  [2] = %d Hz, c = %d", (p)->fout[2], (p)->c[2]) \
    REPORT(INFO, "  [3] = %d Hz, c = %d", (p)->fout[3], (p)->c[3]) \
    REPORT(INFO, "  VCO = IN * %d / %d = %d Hz", (p)->m, (p)->n, (p)->fvco); \
}

/** report functions
 */
char* vio_str_pllc(uint32_t sel);
char* vio_str_stin(uint32_t sel);
char* vio_str_vout(uint32_t sel);
char* vio_str_advcnt(uint32_t cfg);
char* vio_str_format_cs(uint32_t f);
char* vio_str_format_sm(uint32_t f);
char* vio_str_err(int err);

#endif /*VIO_STR_H_*/
