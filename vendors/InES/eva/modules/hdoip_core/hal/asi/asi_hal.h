/*
 *  asi_hal.h
 *
 *  Created on: 20.10.2010
 *      Author: stth
 */

#ifndef ASI_HAL_H_
#define ASI_HAL_H_

#include "std.h"
#include "stdrbf.h"
#include "asi_reg.h"
#include "stdaud.h"
#include "stdeth.h"

#define asi_set_ctrl(p, v)              	HOI_WR32((p), ASI_OFF_CTRL_SET, (v))
#define asi_clr_ctrl(p, v)              	HOI_WR32((p), ASI_OFF_CTRL_CLR, (v))
#define asi_get_ctrl(p, m)              	HOI_REG_RD((p), ASI_OFF_CTRL_RD, (m)) 
#define asi_clr_status(p, v)            	HOI_WR32((p), ASI_OFF_STAT_CLR, (v))
#define asi_get_status(p, m)            	HOI_REG_RD((p), ASI_OFF_STAT_RD, (m))

#define asi_enable(p, st)               	asi_set_ctrl((p), asi_ctrl_en_mask((st)))
#define asi_disable(p, st)              	asi_clr_ctrl((p), asi_ctrl_en_mask((st)))
#define asi_set_write_dsc(p, v)         	RBF_SET_REG((p), ASI_OFF_DSC_WRITE_RW, (v))
#define asi_get_write_dsc(p)            	RBF_GET_REG((p), ASI_OFF_DSC_WRITE_RW)
#define asi_get_read_dsc(p)                     RBF_GET_REG((p), ASI_OFF_DSC_READ_RO)
#define asi_set_dma_burst_size(p, v)    	HOI_WR32((p), ASI_OFF_DMA_BURST_SIZE_RW, (v))
#define asi_get_dma_burst_size(p)       	HOI_RD32((p), ASI_OFF_DMA_BURST_SIZE_RW)
#define asi_get_send_request(p)            	RBF_GET_REG((p), ASI_OFF_SEND_REQUEST)
#define asi_get_active_stream(p)          	RBF_GET_REG((p), ASI_OFF_ACTIVE_STREAM)

#define asi_set_frame_size(p, st, v)    	HOI_WR32((p), asi_reg2off((st), ASI_REG_FRAME_SIZE), (v))
#define asi_get_frame_size(p, st)           HOI_RD32((p), asi_reg2off((st), ASI_REG_FRAME_SIZE))
#define asi_set_ssrc(p, st, v)              HOI_WR32((p), asi_reg2off((st), ASI_REG_SSRC), (v))
#define asi_get_ssrc(p, st)                 HOI_RD32((p), asi_reg2off((st), ASI_REG_SSRC))
#define asi_set_time_per_word(p, st, v)     HOI_WR32((p), asi_reg2off((st), ASI_REG_TIME_PER_WORD), (v))
#define asi_get_time_per_word(p, st)        HOI_RD32((p), asi_reg2off((st), ASI_REG_TIME_PER_WORD))
#define asi_set_payload_words(p, st, v)     HOI_WR32((p), asi_reg2off((st), ASI_REG_PAYLOAD_WORDS), (v))
#define asi_get_payload_words(p, st)        HOI_RD32((p), asi_reg2off((st), ASI_REG_PAYLOAD_WORDS))
#define asi_get_i2s_size(p, st)             HOI_RD32((p), asi_reg2off((st), ASI_REG_SIZE_RO)) /* only debug info */
#define asi_set_ch_map(p, st)               HOI_REG_SET((p), asi_reg2off((st), ASI_REG_AUD_INFO), (v) & ASI_CH_MAP_MSK)
#define asi_get_ch_map(p, st)               HOI_REG_RD((p), asi_reg2off((st), ASI_REG_AUD_INFO), ASI_CH_MAP_MSK)
#define asi_get_fs_detect(p, st)            HOI_REG_RD((p), asi_reg2off((st), ASI_REG_FS_DETECT), 0x3FFFF)
#define asi_get_ch_map_detected(p, st)      HOI_REG_RD((p), asi_reg2off((st), ASI_REG_CH_MAP_DETECT), 0xFFFF)

/* function prototypes */
void asi_set_aud_params(void* p, int unsigned stream, struct hdoip_aud_params* aud_params);
void asi_get_aud_params(void* p, int unsigned stream, struct hdoip_aud_params* aud_params);
void asi_set_eth_params(void* p, int unsigned stream, struct hdoip_eth_params* eth_params);
void asi_get_eth_params(void* p, int unsigned stream, struct hdoip_eth_params* eth_params);


#endif /* ASI_HAL_H_ */
