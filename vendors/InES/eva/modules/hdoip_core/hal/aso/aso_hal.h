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
#define aso_get_reordering_status(p, m)     HOI_REG_RD((p), ASO_OFF_REORDERING_STATUS_RO, (m))
#define aso_clr_reordering_status(p, m)     HOI_REG_CLR((p), ASO_OFF_REORDERING_STATUS_CLR, (m))

#define aso_enable(p)                       aso_set_ctrl((p), ASO_CFG_RUN)
#define aso_disable(p)                      aso_clr_ctrl((p), ASO_CFG_RUN)

#define aso_set_dsc(p, d)                   rbf_set_dsc_r((p), ASO_OFF_DSC_START_RO, (d))
#define aso_set_read_dsc(p, v)              RBF_SET_REG((p), ASO_OFF_DSC_READ, (v))
#define aso_get_dsc(p, d)                   rbf_get_dsc((p), ASO_OFF_DSC_START_RO, (d))
#define aso_get_start_dsc(p)                RBF_GET_REG((p), ASO_OFF_DSC_START_RO)
#define aso_get_stop_dsc(p)                 RBF_GET_REG((p), ASO_OFF_DSC_STOP_RO)
#define aso_get_write_dsc(p)                RBF_GET_REG((p), ASO_OFF_DSC_WRITE_RO)
#define aso_get_read_dsc(p)                 RBF_GET_REG((p), ASO_OFF_DSC_READ)

#define aso_set_aud_delay(p, v)             HOI_WR32((p), ASO_OFF_AUD_DELAY, (v))
#define aso_get_aud_delay(p)                HOI_RD32((p), ASO_OFF_AUD_DELAY)
#define aso_set_fifo_low_th(p, v)           HOI_WR32((p), ASO_OFF_FIFO_LOW_TH, (v))
#define aso_get_fifo_low_th(p)              HOI_RD32((p), ASO_OFF_FIFO_LOW_TH)
#define aso_get_clk_diff(p)                 HOI_RD32((p), ASO_OFF_CLK_CTRL_DIFF_RO)
#define aso_set_min_frames_buffered(p, v)   HOI_WR32((p), ASO_OFF_MIN_FRAMES_BUFFERED, (v))
#define aso_get_min_frames_buffered(p)      HOI_RD32((p), ASO_OFF_MIN_FRAMES_BUFFERED)
#define aso_get_frames_buffered(p)          HOI_RD32((p), ASO_OFF_FRAMES_BUFFERED_RO)
#define aso_set_dma_burst_size(p, v)        HOI_WR32((p), ASO_OFF_DMA_BURST_SIZE, (v))
#define aso_get_dma_burst_size(p)           HOI_RD32((p), ASO_OFF_DMA_BURST_SIZE)
#define aso_set_dma_fifo_almost_full(p, v)  HOI_WR32((p), ASO_OFF_DMA_FIFO_ALMOST_FULL, (v))
#define aso_get_dma_fifo_almost_full(p)     HOI_RD32((p), ASO_OFF_DMA_FIFO_ALMOST_FULL)

#define aso_set_clk_div_upper_bound(p, v)   HOI_WR32((p), ASO_OFF_CLK_DIV_UPPER_BOUND, (v))
#define aso_get_clk_div_upper_bound(p)      HOI_RD32((p), ASO_OFF_CLK_DIV_UPPER_BOUND)
#define aso_set_clk_div_lower_bound(p, v)   HOI_WR32((p), ASO_OFF_CLK_DIV_LOWER_BOUND, (v))
#define aso_get_clk_div_lower_bound(p)      HOI_RD32((p), ASO_OFF_CLK_DIV_LOWER_BOUND)
#define aso_set_clk_div_act(p, v)           HOI_WR32((p), ASO_OFF_CLK_DIV_ACT, (v))
#define aso_get_clk_div_act(p)              HOI_RD32((p), ASO_OFF_CLK_DIV_ACT)
#define aso_set_clk_div_inc(p, v)           HOI_WR32((p), ASO_OFF_CLK_DIV_INC, (v))
#define aso_get_clk_div_inc(p)              HOI_RD32((p), ASO_OFF_CLK_DIV_INC)


/* function prototypes */
void aso_set_aud_params(void* p, struct hdoip_aud_params* aud_params);
void aso_get_aud_params(void* p, struct hdoip_aud_params* aud_params);
uint8_t aso_set_aud_cfg(void* p, uint8_t ch_cnt_l, uint8_t ch_cnt_r, uint8_t bits);
void aso_get_aud_cfg(void* p, uint8_t* ch_cnt_l, uint8_t* ch_cnt_r, uint8_t* bits);

#endif /* ASO_HAL_H_ */
