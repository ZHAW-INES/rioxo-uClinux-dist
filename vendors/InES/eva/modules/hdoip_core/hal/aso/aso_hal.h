/*
 * aso_hal.h
 *
 * Created on: 22.10.2010
 *     Author: stth
 */

#ifndef ASO_HAL_H_
#define ASO_HAL_H_

#include "std.h"
#include "stdrbf.h"
#include "stdaud.h"
#include "aso_reg.h"

/* constants of hardware */
#define ASO_REORDERING_RAM_SIZE             (1024)
#define ASO_DATA_FIFO_SIZE                  (256)   /* in 32 bit words */

/* macros */
#define aso_set_ctrl(p, m)                  HOI_REG_SET((p), ASO_OFF_CONFIG, (m))
#define aso_get_ctrl(p, m)                  HOI_REG_RD((p), ASO_OFF_CONFIG, (m))
#define aso_clr_ctrl(p, m)                  HOI_REG_CLR((p), ASO_OFF_CONFIG, (m))
#define aso_get_status(p, m)                HOI_REG_RD((p), ASO_OFF_STATUS, (m))
#define aso_clr_status(p, m)                HOI_REG_CLR((p), ASO_OFF_STATUS, (m))

#define aso_enable(p)                       aso_set_ctrl((p), ASO_CFG_RUN)
#define aso_disable(p)                      aso_clr_ctrl((p), ASO_CFG_RUN)
#define aso_enable_i2s_output(p)            aso_set_ctrl((p), ASO_CFG_OE) /*TODO perhaps only needed for the test with stellaris board */
#define aso_disable_i2s_output(p)           aso_clr_ctrl((p), ASO_CFG_OE)

#define aso_set_aud_delay(p, v)             HOI_WR32((p), ASO_OFF_AUD_DELAY, (v))
#define aso_get_aud_delay(p)                HOI_RD32((p), ASO_OFF_AUD_DELAY)
#define aso_get_clk_diff(p)                 HOI_RD32((p), ASO_OFF_CLK_CTRL_DIFF_RO)
#define aso_get_frames_buffered(p)          HOI_RD32((p), ASO_OFF_FRAMES_BUFFERED_RO)
#define aso_set_clk_div_base(p, v)          HOI_WR32((p), ASO_OFF_CLK_DIV_BASE, (v))
#define aso_get_clk_div_base(p)             HOI_RD32((p), ASO_OFF_CLK_DIV_BASE)
#define aso_set_clk_div_kp(p, v)            HOI_WR32((p), ASO_OFF_CLK_DIV_KP, (v))
#define aso_get_clk_div_kp(p)               HOI_RD32((p), ASO_OFF_CLK_DIV_KP)
#define aso_set_clk_div_ki(p, v)            HOI_WR32((p), ASO_OFF_CLK_DIV_KI, (v))
#define aso_get_clk_div_ki(p)               HOI_RD32((p), ASO_OFF_CLK_DIV_KI)
#define aso_set_mclk2bclk_ratio(p, v)       HOI_WR32((p), ASO_OFF_MCLK_TO_BCLK_RATIO, (v))
#define aso_set_bclk2lrclk_ratio(p, v)      HOI_WR32((p), ASO_OFF_BCLK_TO_LRCLK_RATIO, (v))
#define aso_get_mclk2bclk_ratio(p)          HOI_RD32((p), ASO_OFF_MCLK_TO_BCLK_RATIO)
#define aso_get_bclk2lrclk_ratio(p)         HOI_RD32((p), ASO_OFF_BCLK_TO_LRCLK_RATIO)
#define aso_set_clk_div_diff(p, v)          HOI_WR32((p), ASO_OFF_CLK_DIV_DIFF, (v))
#define aso_get_clk_div_diff(p)             HOI_RD32((p), ASO_OFF_CLK_DIV_DIFF)
#define aso_set_clk_div_act(p, v)           HOI_WR32((p), ASO_OFF_CLK_DIV_ACT, (v))
#define aso_get_clk_div_act(p)              HOI_RD32((p), ASO_OFF_CLK_DIV_ACT)

#define aso_set_denominator(p, v)           HOI_WR32((p), ASO_OFF_DENOMINATOR, (v))

static inline void aso_set_aud_params(void* p, struct hdoip_aud_params* aud_params)
{
    uint16_t ch_cnt_left, ch_cnt_right, ch_cnt;//remove

    ch_cnt = aud_chmap2cnt(aud_params->ch_map & 0xFF);
    ch_cnt_left = ch_cnt>>1;
    ch_cnt_right = ch_cnt - ch_cnt_left;

    HOI_WR32(p,  ASO_OFF_CH_CNT_LEFT, ch_cnt_left);
    HOI_WR32(p,  ASO_OFF_CH_CNT_RIGHT, ch_cnt_right);
    HOI_WR32(p,  ASO_OFF_CONTAINER, aud_bits_to_container(aud_params->sample_width));
}
static inline void aso_get_aud_params(void* p, struct hdoip_aud_params* aud_params)
{
    uint16_t ch_cnt_left, ch_cnt_right;//remove

    ch_cnt_left = HOI_RD32(p,  ASO_OFF_CH_CNT_LEFT);
    ch_cnt_right = HOI_RD32(p,  ASO_OFF_CH_CNT_RIGHT);
    aud_params->sample_width = aud_container_to_bits(HOI_RD32(p,  ASO_OFF_CONTAINER));
}

#endif /* ASO_HAL_H_ */
