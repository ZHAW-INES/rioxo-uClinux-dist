#ifndef VSO_HAL_H_
#define VSO_HAL_H_

#include "std.h" 
#include "stdrbf.h"
#include "vso_reg.h"

#define vso_set_ctrl(p, m)              HOI_REG_SET(p, VSO_OFF_CTRL, m)
#define vso_clr_ctrl(p, m)              HOI_REG_CLR(p, VSO_OFF_CTRL, m)
#define vso_get_ctrl(p, m)              HOI_REG_RD(p, VSO_OFF_CTRL, m)
#define vso_clr_status(p, m)            HOI_REG_CLR(p, VSO_OFF_STATUS, m)
#define vso_get_status(p, m)            HOI_REG_RD(p, VSO_OFF_STATUS, m)

#define vso_enable(p)                   vso_set_ctrl(p, VSO_CTRL_RUN)
#define vso_disable(p)                  vso_clr_ctrl(p, VSO_CTRL_RUN) 
#define vso_start_clear(p)              vso_set_ctrl(p, VSO_CTRL_CLEAR) 
#define vso_stop_clear(p)               vso_clr_ctrl(p, VSO_CTRL_CLEAR)

#define vso_get_timestamp(p)            HOI_RD32(p, VSO_OFF_TIMESTAMP_RO)
#define vso_get_marker_timeout(p)       HOI_RD32(p, VSO_OFF_MARKER_TIMEOUT)
#define vso_set_marker_timeout(p, v)    HOI_WR32(p, VSO_OFF_MARKER_TIMEOUT, v)
#define vso_get_packet_timeout(p)       HOI_RD32(p, VSO_OFF_PACKET_TIMEOUT)
#define vso_set_packet_timeout(p, v)    HOI_WR32(p, VSO_OFF_PACKET_TIMEOUT, v)
#define vso_get_vs_delay(p)             HOI_RD32(p, VSO_OFF_VS_DELAY)
#define vso_set_vs_delay(p, v)          HOI_WR32(p, VSO_OFF_VS_DELAY, v)
#define vso_get_vs_duration(p)          HOI_RD32(p, VSO_OFF_VS_DURATION)
#define vso_set_vs_duration(p, v)       HOI_WR32(p, VSO_OFF_VS_DURATION, v)
#define vso_get_vframe_lost(p)          HOI_RD32(p, VSO_OFF_VFRAME_LOST_RO)
#define vso_get_vframe_cnt(p)           HOI_RD32(p, VSO_OFF_VFRAME_CNT_RO)
#define vso_get_packet_lost(p)          HOI_RD32(p, VSO_OFF_PACKET_LOST_RO)
#define vso_get_packet_cnt(p)           HOI_RD32(p, VSO_OFF_PACKET_CNT_RO)
#define vso_get_dma_burst_size(p)       HOI_RD32(p, VSO_OFF_DMA_BURST_SIZE)
#define vso_set_dma_burst_size(p, v)    HOI_WR32(p, VSO_OFF_DMA_BURST_SIZE, v)
#define vso_get_dma_almost_full(p)      HOI_RD32(p, VSO_OFF_DMA_ALMOST_FULL)
#define vso_set_dma_almost_full(p, v)   HOI_WR32(p, VSO_OFF_DMA_ALMOST_FULL, v)
#define vso_get_packet_in_cnt(p)        HOI_RD32(p, VSO_OFF_PACKET_IN_CNT_RO)
#define vso_get_vsync_delay(p)          HOI_RD32(p, VSO_OFF_VSYNC_DELAY)
#define vso_set_vsync_delay(p, v)       HOI_WR32(p, VSO_OFF_VSYNC_DELAY, v)
#define vso_get_scomm5_delay(p)         HOI_RD32(p, VSO_OFF_SCOMM5_DELAY)
#define vso_set_scomm5_delay(p, v)      HOI_WR32(p, VSO_OFF_SCOMM5_DELAY, v)
#define vso_get_timestamp_min(p)        HOI_RD32(p, VSO_OFF_TIMESTAMP_MIN)
#define vso_set_timestamp_min(p, v)     HOI_WR32(p, VSO_OFF_TIMESTAMP_MIN, v)
#define vso_get_timestamp_max(p)        HOI_RD32(p, VSO_OFF_TIMESTAMP_MAX)
#define vso_set_timestamp_max(p, v)     HOI_WR32(p, VSO_OFF_TIMESTAMP_MAX, v)
#define vso_get_timestamp_diff(p)       HOI_RD32(p, VSO_OFF_TIMESTAMP_DIFF_RO)

#endif /* VSO_HAL_H_ */ 
