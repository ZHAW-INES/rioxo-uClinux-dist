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

#define asi_set_ctrl(p, m)              HOI_REG_SET((p), ASI_OFF_CTRL_SET, (m)) 
#define asi_clr_ctrl(p, m)              HOI_REG_SET((p), ASI_OFF_CTRL_CLR, (m))
#define asi_get_ctrl(p, m)              HOI_REG_RD((p), ASI_OFF_CTRL_RD, (m)) 
#define asi_clr_status(p, m)            HOI_REG_SET((p), ASI_OFF_STAT_CLR, (m))
#define asi_get_status(p, m)            HOI_REG_RD((p), ASI_OFF_STAT_RD, (m))

#define asi_enable(p)                   asi_set_ctrl((p), ASI_CTRL_RUN)
#define asi_disable(p)                  asi_clr_ctrl((p), ASI_CTRL_RUN)

#define asi_set_dsc(p, dsc)             rbf_set_dsc_w((p), ASI_OFF_DSC_START_RO, (dsc))
#define asi_set_write_dsc(p, v)         RBF_SET_REG((p), ASI_OFF_DSC_WRITE_RW, (v))
#define asi_get_dsc(p, dsc)             rbf_get_dsc((p), ASI_OFF_DSC_START_RO, (dsc))
#define asi_get_start_dsc(p)            RBF_GET_REG((p), ASI_OFF_DSC_START_RO)
#define asi_get_stop_dsc(p)             RBF_GET_REG((p), ASI_OFF_DSC_STOP_RO)
#define asi_get_write_dsc(p)            RBF_GET_REG((p), ASI_OFF_DSC_WRITE_RW)
#define asi_get_read_dsc(p)             RBF_GET_REG((p), ASI_OFF_DSC_READ_RO)

#define asi_set_dma_burst_size(p, v)    HOI_WR32((p), ASI_OFF_DMA_BURST_SIZE_RW, (v))
#define asi_get_dma_burst_size(p)       HOI_RD32((p), ASI_OFF_DMA_BURST_SIZE_RW)
#define asi_set_frame_size(p, v)        HOI_WR32((p), ASI_OFF_FRAME_SIZE, (v))
#define asi_get_frame_size(p)           HOI_RD32((p), ASI_OFF_FRAME_SIZE)
#define asi_set_src_ip(p, v)            HOI_WR32((p), ASI_OFF_SRC_IP, (v))
#define asi_get_src_ip(p)               HOI_RD32((p), ASI_OFF_SRC_IP)
#define asi_set_dst_ip(p, v)            HOI_WR32((p), ASI_OFF_DST_IP, (v))
#define asi_get_dst_ip(p)               HOI_RD32((p), ASI_OFF_DST_IP)
#define asi_set_src_port(p, v)          HOI_WR32((p), ASI_OFF_SRC_PORT, (v))
#define asi_get_src_port(p)             HOI_RD32((p), ASI_OFF_SRC_PORT)
#define asi_set_dst_port(p, v)          HOI_WR32((p), ASI_OFF_DST_PORT, (v))
#define asi_get_dst_port(p)             HOI_RD32((p), ASI_OFF_DST_PORT)
#define asi_set_ssrc(p, v)              HOI_WR32((p), ASI_OFF_SSRC, (v))
#define asi_get_ssrc(p)                 HOI_RD32((p), ASI_OFF_SSRC)
#define asi_set_time_per_word(p, v)     HOI_WR32((p), ASI_OFF_TIME_PER_WORD, (v))
#define asi_get_time_per_word(p)        HOI_RD32((p), ASI_OFF_TIME_PER_WORD)
#define asi_set_ip_length(p, v)         HOI_WR32((p), ASI_OFF_IP_LENGTH, (v))
#define asi_get_ip_length(p)            HOI_RD32((p), ASI_OFF_IP_LENGTH)
#define asi_set_udp_word_length(p, v)   HOI_WR32((p), ASI_OFF_UDP_WORD_LENGTH, (v))
#define asi_get_udp_word_length(p)      HOI_RD32((p), ASI_OFF_UDP_WORD_LENGTH)
#define asi_set_payload_words(p, v)     HOI_WR32((p), ASI_OFF_PAYLOAD_WORDS, (v))
#define asi_get_payload_words(p)        HOI_RD32((p), ASI_OFF_PAYLOAD_WORDS)
#define asi_get_i2s_size                HOI_RD32((p), AIS_OFF_SIZE_RO)

/* function prototypes */
void asi_set_dst_mac(void* p, uint8_t* mac);
void asi_get_dst_mac(void* p, uint8_t* mac);
void asi_set_src_mac(void* p, uint8_t* mac);
void asi_get_src_mac(void* p, uint8_t* mac);
void asi_set_ttl_tos(void* p, uint8_t ttl, uint8_t tos);
void asi_get_ttl_tos(void* p, uint8_t* ttl, uint8_t* tos);
void asi_set_ttl(void* p, uint8_t ttl);
uint8_t asi_get_ttl(void* p);
void asi_set_tos(void* p, uint8_t tos);
uint8_t asi_get_tos(void* p);
uint8_t asi_set_aud_cfg(void* p, uint8_t ch_cnt_l, uint8_t ch_cnt_r, uint8_t bits);
void asi_get_aud_cfg(void* p, uint8_t* ch_cnt_l, uint8_t* ch_cnt_r, uint8_t* bits);
void asi_set_aud_params(void* p, struct hdoip_aud_params* aud_params);
void asi_get_aud_params(void* p, struct hdoip_aud_params* aud_params);
void asi_set_eth_params(void* p, struct hdoip_eth_params* eth_params);
void asi_get_eth_params(void* p, struct hdoip_eth_params* eth_params);


#endif /* ASI_HAL_H_ */
