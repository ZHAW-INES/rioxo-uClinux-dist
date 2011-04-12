/*
 * eto_hal.h
 *
 * Created on: 08.12.2010
 *     Author: buan
 */

#ifndef ETO_HAL_H_
#define ETO_HAL_H_

#include "stdhal.h"
#include "eto_reg.h"

/* constants of hardware */
#define CLOCK_DIVISION_SHIFT_FACTOR		0x00000000
#define SHIFT_VALUE_SHIFT_FACTOR		0x00000005

/* macros set*/
#define eto_set_config_reg(p, v)                HOI_WR32((p), ETO_CONFIG_SET_REG, (v))
#define eto_clr_config_reg(p, v)                HOI_WR32((p), ETO_CONFIG_CLR_REG, (v))
#define eto_clr_status_reg(p, v)                HOI_WR32((p), ETO_STATUS_REG, (v))
#define eto_set_config_audio_enc_en(p) 		    HOI_WR32(p, ETO_CONFIG_SET_REG, ETO_CONFIG_AUDIO_ENC_EN)
#define eto_set_config_video_enc_en(p) 		    HOI_WR32(p, ETO_CONFIG_SET_REG, ETO_CONFIG_VIDEO_ENC_EN)
#define eto_set_config_audio_enc_dis(p) 	    HOI_WR32(p, ETO_CONFIG_CLR_REG, ETO_CONFIG_AUDIO_ENC_EN)
#define eto_set_config_video_enc_dis(p) 	    HOI_WR32(p, ETO_CONFIG_CLR_REG, ETO_CONFIG_VIDEO_ENC_EN)
#define eto_set_config_cpu_frame_disable(p) 	HOI_WR32(p, ETO_CONFIG_SET_REG, ETO_CONFIG_SEND_CPU_FRAME_DISABLE)
#define eto_set_config_vid_frame_disable(p) 	HOI_WR32(p, ETO_CONFIG_SET_REG, ETO_CONFIG_SEND_VID_FRAME_DISABLE)
#define eto_set_config_aud_frame_disable(p) 	HOI_WR32(p, ETO_CONFIG_SET_REG, ETO_CONFIG_SEND_AUD_FRAME_DISABLE)
#define eto_set_config_cpu_frame_enable(p) 	    HOI_WR32(p, ETO_CONFIG_CLR_REG, ETO_CONFIG_SEND_CPU_FRAME_DISABLE)
#define eto_set_config_vid_frame_enable(p) 	    HOI_WR32(p, ETO_CONFIG_CLR_REG, ETO_CONFIG_SEND_VID_FRAME_DISABLE)
#define eto_set_config_aud_frame_enable(p) 	    HOI_WR32(p, ETO_CONFIG_CLR_REG, ETO_CONFIG_SEND_AUD_FRAME_DISABLE)
#define eto_set_config_start(p) 		        HOI_WR32(p, ETO_CONFIG_SET_REG, ETO_CONFIG_FSM_EN)
#define eto_set_config_stop(p) 			        HOI_WR32(p, ETO_CONFIG_CLR_REG, ETO_CONFIG_FSM_EN)

#define eto_clr_status_cpu_idle(p)              eto_clr_status_reg(p, ETO_STATUS_CPU_IDLE)
#define eto_clr_status_vid_idle(p)              eto_clr_status_reg(p, ETO_STATUS_VID_IDLE)
#define eto_clr_status_aud_idle(p)              eto_clr_status_reg(p, ETO_STATUS_AUD_IDLE)

#define eto_set_burst_size_reg(p, v) 		    HOI_WR32((p), ETO_BURST_SIZE_REG, (v))
#define eto_set_dma_almost_full(p, v) 		    HOI_WR32((p), ETO_DMA_ALMOST_FULL_REG, (v))

#define eto_set_cpu_desc(p, d)                  rbf_set_dsc((p), ETO_CPU_START_REG, (d))
#define eto_set_cpu_start_desc(p, v) 		    RBF_SET_REG((p), ETO_CPU_START_REG, (v))
#define eto_set_cpu_stop_desc(p, v) 		    RBF_SET_REG((p), ETO_CPU_STOP_REG, (v))
#define eto_set_cpu_read_desc(p, v) 		    RBF_SET_REG((p), ETO_CPU_READ_REG, (v))
#define eto_set_cpu_write_desc(p, v) 		    RBF_SET_REG((p), ETO_CPU_WRITE_REG, (v))
#define eto_set_aud_desc(p, d)                  rbf_set_dsc_rc((p), ETO_AUD_START_REG, (d))
#define eto_set_aud_start_desc(p, v) 		    RBF_SET_REG((p), ETO_AUD_START_REG, (v))
#define eto_set_aud_stop_desc(p, v) 		    RBF_SET_REG((p), ETO_AUD_STOP_REG, (v))
#define eto_set_aud_read_desc(p, v) 		    RBF_SET_REG((p), ETO_AUD_READ_REG, (v))
#define eto_set_vid_desc(p, d)                  rbf_set_dsc_rc((p), ETO_VID_START_REG, (d))
#define eto_set_vid_start_desc(p, v) 		    RBF_SET_REG((p), ETO_VID_START_REG, (v))
#define eto_set_vid_stop_desc(p, v) 		    RBF_SET_REG((p), ETO_VID_STOP_REG, (v))
#define eto_set_vid_read_desc(p, v) 		    RBF_SET_REG((p), ETO_VID_READ_REG, (v))

#define eto_set_aes_session_key_0(p, v) 	    HOI_WR32((p), ETO_AES_SESSION_KEY0_REG, (v))
#define eto_set_aes_session_key_1(p, v) 	    HOI_WR32((p), ETO_AES_SESSION_KEY1_REG, (v))
#define eto_set_aes_session_key_2(p, v) 	    HOI_WR32((p), ETO_AES_SESSION_KEY2_REG, (v))
#define eto_set_aes_session_key_3(p, v) 	    HOI_WR32((p), ETO_AES_SESSION_KEY3_REG, (v))
#define eto_set_aes_riv_0(p, v) 		        HOI_WR32((p), ETO_AES_RIV0_REG, (v))
#define eto_set_aes_riv_1(p, v) 		        HOI_WR32((p), ETO_AES_RIV1_REG, (v))

#define eto_set_prio_cpu_load(p, v)             HOI_REG_LD((p), ETO_CPU_PRIORITY_REG, ETO_PRIO_LOAD_MASK, (v)) 
#define eto_set_prio_aud_load(p, v)             HOI_REG_LD((p), ETO_AUD_PRIORITY_REG, ETO_PRIO_LOAD_MASK, (v)) 
#define eto_set_prio_vid_load(p, v)             HOI_REG_LD((p), ETO_VID_PRIORITY_REG, ETO_PRIO_LOAD_MASK, (v)) 
#define eto_set_prio_cpu_scale(p, v)            HOI_REG_LD((p), ETO_CPU_PRIORITY_REG, ETO_PRIO_SCALE_MASK, ((v)<<24))
#define eto_set_prio_aud_scale(p, v)            HOI_REG_LD((p), ETO_AUD_PRIORITY_REG, ETO_PRIO_SCALE_MASK, ((v)<<24))
#define eto_set_prio_vid_scale(p, v)            HOI_REG_LD((p), ETO_VID_PRIORITY_REG, ETO_PRIO_SCALE_MASK, ((v)<<24))

/* macros get*/
#define eto_get_config_reg(p) 			        HOI_RD32((p), ETO_CONFIG_SET_REG)
#define eto_get_config_audio_enc_en(p) 		    HOI_REG_TST(p, ETO_CONFIG_SET_REG, ETO_CONFIG_AUDIO_ENC_EN)
#define eto_get_config_video_enc_en(p) 		    HOI_REG_TST(p, ETO_CONFIG_SET_REG, ETO_CONFIG_VIDEO_ENC_EN)
#define eto_get_config_cpu_frame_disable(p) 	HOI_REG_TST(p, ETO_CONFIG_SET_REG, ETO_CONFIG_SEND_CPU_FRAME_DISABLE)
#define eto_get_config_vid_frame_disable(p) 	HOI_REG_TST(p, ETO_CONFIG_SET_REG, ETO_CONFIG_SEND_VID_FRAME_DISABLE)
#define eto_get_config_aud_frame_disable(p) 	HOI_REG_TST(p, ETO_CONFIG_SET_REG, ETO_CONFIG_SEND_AUD_FRAME_DISABLE)
#define eto_get_config_start(p) 		        HOI_REG_TST(p, ETO_CONFIG_SET_REG, ETO_CONFIG_FSM_EN)

#define eto_get_status_reg(p)                   HOI_RD32((p), ETO_STATUS_REG)
#define eto_get_status_fsm_idle(p)              ((eto_get_status_reg(p) & ETO_STATUS_FSM_IDLE) && ETO_STATUS_FSM_IDLE)
#define eto_get_status_cpu_idle(p)              ((eto_get_status_reg(p) & ETO_STATUS_CPU_IDLE) && ETO_STATUS_CPU_IDLE)
#define eto_get_status_vid_idle(p)              ((eto_get_status_reg(p) & ETO_STATUS_VID_IDLE) && ETO_STATUS_VID_IDLE)
#define eto_get_status_aud_idle(p)              ((eto_get_status_reg(p) & ETO_STATUS_AUD_IDLE) && ETO_STATUS_AUD_IDLE)


#define eto_get_burst_size_reg(p) 		        HOI_RD32((p), ETO_BURST_SIZE_REG)
#define eto_get_dma_almost_full(p)   		    HOI_RD32((p), ETO_DMA_ALMOST_FULL_REG)

#define eto_report_cpu_desc(p)                  rbf_report_dsc((p), ETO_CPU_START_REG)
#define eto_report_aud_desc(p)                  rbf_report_dsc((p), ETO_AUD_START_REG)
#define eto_report_vid_desc(p)                  rbf_report_dsc((p), ETO_VID_START_REG)
#define eto_get_cpu_desc(p, d)                  rbf_get_dsc((p), ETO_CPU_START_REG, (d))
#define eto_get_cpu_start_desc(p)   		    RBF_GET_REG((p), ETO_CPU_START_REG)
#define eto_get_cpu_stop_desc(p)     		    RBF_GET_REG((p), ETO_CPU_STOP_REG)
#define eto_get_cpu_read_desc(p)     		    RBF_GET_REG((p), ETO_CPU_READ_REG)
#define eto_get_cpu_write_desc(p)    		    RBF_GET_REG((p), ETO_CPU_WRITE_REG)
#define eto_get_vid_desc(p, d)                  rbf_get_dsc((p), ETO_VID_START_REG, (d))
#define eto_get_vid_start_desc(p)    		    RBF_GET_REG((p), ETO_VID_START_REG)
#define eto_get_vid_stop_desc(p)     		    RBF_GET_REG((p), ETO_VID_STOP_REG)
#define eto_get_vid_read_desc(p)     		    RBF_GET_REG((p), ETO_VID_READ_REG)
#define eto_get_vid_write_desc(p)    		    RBF_GET_REG((p), ETO_VID_WRITE_REG)
#define eto_get_aud_desc(p, d)                  rbf_get_dsc((p), ETO_AUD_START_REG, (d))
#define eto_get_aud_start_desc(p)    		    RBF_GET_REG((p), ETO_AUD_START_REG)
#define eto_get_aud_stop_desc(p)     		    RBF_GET_REG((p), ETO_AUD_STOP_REG)
#define eto_get_aud_read_desc(p)     		    RBF_GET_REG((p), ETO_AUD_READ_REG)
#define eto_get_aud_write_desc(p)    		    RBF_GET_REG((p), ETO_AUD_WRITE_REG)

#define eto_get_aes_session_key_0(p) 		    HOI_RD32((p), ETO_AES_SESSION_KEY0_REG)
#define eto_get_aes_session_key_1(p) 		    HOI_RD32((p), ETO_AES_SESSION_KEY1_REG)
#define eto_get_aes_session_key_2(p) 		    HOI_RD32((p), ETO_AES_SESSION_KEY2_REG)
#define eto_get_aes_session_key_3(p) 		    HOI_RD32((p), ETO_AES_SESSION_KEY3_REG)
#define eto_get_aes_riv_0(p) 		 	        HOI_RD32((p), ETO_AES_RIV0_REG)
#define eto_get_aes_riv_1(p) 			        HOI_RD32((p), ETO_AES_RIV1_REG)

#define eto_get_status_frame_err(p) 		    HOI_REG_TST(p, ETO_STATUS_REG, ETO_STATUS_FRAME_ERROR)
#define eto_get_aud_packet_cnt(p) 		        HOI_RD32((p), ETO_AUD_PACKET_CNT_REG)
#define eto_get_vid_packet_cnt(p) 		        HOI_RD32((p), ETO_VID_PACKET_CNT_REG)
#define eto_get_cpu_packet_cnt(p) 		        HOI_RD32((p), ETO_CPU_PACKET_CNT_REG)
#define eto_get_inv_packet_cnt(p) 		        HOI_RD32((p), ETO_INV_PACKET_CNT_REG)

#define eto_get_prio_cpu_load(p)                HOI_REG_RD((p), ETO_CPU_PRIORITY_REG, ETO_PRIO_LOAD_MASK) 
#define eto_get_prio_aud_load(p)                HOI_REG_RD((p), ETO_AUD_PRIORITY_REG, ETO_PRIO_LOAD_MASK) 
#define eto_get_prio_vid_load(p)                HOI_REG_RD((p), ETO_VID_PRIORITY_REG, ETO_PRIO_LOAD_MASK) 
#define eto_get_prio_clock_div(p)               HOI_REG_RD((p), ETO_PRIORITY_CONFIG_REG, ETO_PRIO_CFG_CLOCK_DIV_MASK)
#define eto_get_prio_shift(p)                   ((HOI_REG_RD((p), ETO_PRIORITY_CONFIG_REG, ETO_PRIO_CFG_SHIFT_MASK))>>28)
#define eto_get_prio_cpu_scale(p)               ((HOI_REG_RD((p), ETO_CPU_PRIORITY_REG, ETO_PRIO_LOAD_MASK))>>24)
#define eto_get_prio_aud_scale(p)               ((HOI_REG_RD((p), ETO_AUD_PRIORITY_REG, ETO_PRIO_LOAD_MASK))>>24)
#define eto_get_prio_vid_scale(p)               ((HOI_REG_RD((p), ETO_VID_PRIORITY_REG, ETO_PRIO_LOAD_MASK))>>24)


/* function prototypes */
void eto_prio_init(void* p, uint8_t shift, uint32_t clock_div);
#endif /* ETO_HAL_H_ */






