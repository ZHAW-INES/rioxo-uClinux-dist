#ifndef VIO_MUX_H_
#define VIO_MUX_H_

#include "stdhal.h"
#include "stdvid.h"
#include "vio_reg.h"
#include "vio_cfg.h"

#include "vio_osd.h"
#include "vio_pll.h"

/** Init Sequence:
 *  1) assert reset
 *  2) configure pll / wait for pll to lock
 *  3) set multiplexer path
 *  4) set configuration (enable needed hw)
 *  6) remove reset
 */

/** VIO Macros
 */
#define vio_set_cfg(p, m)       HOI_REG_SET((p), VIO_OFF_CONFIG, (m))
#define vio_clr_cfg(p, m)       HOI_REG_CLR((p), VIO_OFF_CONFIG, (m))
#define vio_set_cfg_2(p, m)     HOI_REG_SET((p), VIO_OFF_CONFIG_2, (m))
#define vio_clr_cfg_2(p, m)     HOI_REG_CLR((p), VIO_OFF_CONFIG_2, (m))
#define vio_get_cfg(p, m)       HOI_REG_RD((p), VIO_OFF_CONFIG, (m))
#define vio_get_sta(p, m)       HOI_REG_RD((p), VIO_OFF_STATUS, (m))
#define vio_set_mux(p, m, v)    HOI_REG_LD((p), VIO_OFF_MUX, (m), (v))
#define vio_get_mux(p, m)       HOI_REG_RD((p), VIO_OFF_MUX, (m))

/** VIO function macros
 */
#define vio_start(p)            vio_set_cfg((p), VIO_CFG_RUN)
#define vio_stop(p)             vio_clr_cfg((p), VIO_CFG_RUN)
#define vio_hdp_reset(p)        vio_set_cfg((p), VIO_CFG_HPD_RESET)
#define vio_set_stin(p, sel)    vio_set_mux((p), VIO_MUX_STIN_MASK, sel)
#define vio_set_vout(p, sel)    vio_set_mux((p), VIO_MUX_VOUT_MASK, sel)
#define vio_set_pllc(p, sel)    vio_set_mux((p), VIO_MUX_PLLC_MASK, sel)
#define vio_set_proc(p, sel)    vio_set_mux((p), VIO_MUX_PROC_MASK, sel)
#define vio_set_rng(p, o, a, b) HOI_WR32((p), (o), (((b)<<16)|(a)))
#define vio_set_advcnt(p, sel)  HOI_REG_LD((p), VIO_OFF_CONFIG, VIO_CFG_MODE, (((sel)&3)<<VIO_CFG_MODE_SHIFT))
#define vio_set_size(p, x)      HOI_WR32((p), VIO_OFF_FRAME_LENGTH, (x))
#define vio_set_hsplit(p, x)    HOI_WR32((p), VIO_OFF_HSPLIT, (x))
#define vio_get_stin(p)         vio_get_mux((p), VIO_MUX_STIN_MASK)
#define vio_get_vout(p)         vio_get_mux((p), VIO_MUX_VOUT_MASK)
#define vio_get_pllc(p)         vio_get_mux((p), VIO_MUX_PLLC_MASK)
#define vio_get_proc(p)         vio_get_mux((p), VIO_MUX_PROC_MASK)
#define vio_get_advcnt(p)       (HOI_REG_RD((p), VIO_OFF_CONFIG, VIO_CFG_MODE) >> VIO_CFG_MODE_SHIFT) 
#define vio_get_size(p)         HOI_RD32((p), VIO_OFF_FRAME_LENGTH)
#define vio_get_hsplit(p)       HOI_RD32((p), VIO_OFF_HSPLIT)
#define vio_get_timer(p)        HOI_RD32((p), VIO_OFF_TIMER)

#define vio_get_statistic_fvsnyc(p)     HOI_RD32((p), VIO_OFF_FVSYNC_CNT)
#define vio_get_statistic_vid_in(p)     HOI_RD32((p), VIO_OFF_VID_IN_CNT)
#define vio_get_statistic_vid_out(p)    HOI_RD32((p), VIO_OFF_VID_OUT_CNT)
#define vio_get_statistic_st_in(p)      HOI_RD32((p), VIO_OFF_ST_IN_CNT)
#define vio_get_statistic_st_out(p)     HOI_RD32((p), VIO_OFF_ST_OUT_CNT)


static inline void vio_reset(void* p)
{
    uint32_t tmp;
    tmp = vio_get_cfg(p, 0xFFFFFFFF);
    tmp = tmp & VIO_CFG_HPD;
	HOI_WR32(p, VIO_OFF_CONFIG, VIO_CFG_PLL_RUN | tmp);
	HOI_WR32(p, VIO_OFF_MUX, 0);
}

/** VIO prototype
 */
void vio_enable_output_timing(void* p);
void vio_set_timing(void* p, t_video_timing* p_vt, int triggersource);
void vio_set_control(void* p, t_video_timing* p_vt, int ppm, int sel);
void vio_set_transform(void* p, uint32_t o, t_color_transform m, uint32_t cfg, bool vpol, bool hpol, bool invert_cb_cr);

int  vio_get_timing(void* p, t_video_timing* p_vt);
void vio_get_input_frequency(void* p, t_video_timing* p_vt);
uint32_t vio_get_fin(void* p);
uint32_t vio_get_fout(void* p);
int32_t vio_get_tg_error(void* p);
int32_t vio_get_pll_error(void* p);
void vio_set_input_clockmux(void* p, int advcnt, bool hdmi_active);

#endif /*VIO_MUX_H_*/
