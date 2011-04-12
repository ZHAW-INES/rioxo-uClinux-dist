/*
 * aso_drv_cfg.h
 *
 * Created on: 01.11.2010
 *     Author: stth
 */

#ifndef ASO_DRV_CFG_H_
#define ASO_DRV_CFG_H_

#include "std.h"
#include "hoi_burst.h"

/* config values */
#define ASO_DRV_DMA_FIFO_ALMOST_FULL        (ASO_DATA_FIFO_SIZE - ASO_DRV_DMA_BURST_SIZE - 4)
#define ASO_DRV_I2S_FREQ_TOL                (((uint64_t)1050<<27)/1e6)      /* 1000(source) + 50(local) ppm */
#define ASO_DRV_I2S_FREQ_FACT_UPPER         ((1<<27) + ASO_DRV_I2S_FREQ_TOL)
#define ASO_DRV_I2S_FREQ_FACT_LOWER         ((1<<27) - ASO_DRV_I2S_FREQ_TOL)
#define ASO_DRV_I2S_FREQ_TOL_DIV            (1050/50)                       /* max 50ppm change per frequency step*/
#define ASO_DRV_DISABLE_FIFO_LOW_TH         (0)
#define ASO_DRV_DEFAULT_FIFO_LOW_TH         (5)
#define ASO_DRV_DISABLE_FRAMES_BUFFERED     (ASO_REORDERING_RAM_SIZE - 1)
#define ASO_DRV_MIN_CH_CNT                  (1)
#define ASO_DRV_MAX_CH_CNT                  (8)

/* status bits */
#define ASO_DRV_STATUS_ACTIV                (0x00000001)
#define ASO_DRV_STATUS_AUD_PARAMS_SET       (0x00000002)
#define ASO_DRV_STATUS_AUD_DELAY_SET        (0x00000004)
#define ASO_DRV_STATUS_SIZE_ERROR           (0x00000008)
#define ASO_DRV_STATUS_FIFO_EMPTY           (0x00000010)
#define ASO_DRV_STATUS_FIFO_FULL            (0x00000020)
#define ASO_DRV_STATUS_RAM_FULL             (0x00000040)

typedef struct {
    uint32_t                    status;
    void*                       p_aso;
} t_aso;

#endif /* ASO_DRV_CFG_H_ */
