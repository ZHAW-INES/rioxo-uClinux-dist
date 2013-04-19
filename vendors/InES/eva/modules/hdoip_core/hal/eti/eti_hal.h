/*
 * eti_hal.h
 *
 * Created on: 08.12.2010
 *     Author: buan
 */

#ifndef ETI_HAL_H_
#define ETI_HAL_H_

#include "stdhal.h"
#include "stdrbf.h"
#include "eti_reg.h"

/* constants of hardware */


/* macros set*/
#define eti_set_config_reg(p, m)		    HOI_WR32((p), ETI_CONFIG_SET_REG, (m))
#define eti_clr_config_reg(p, m)		    HOI_WR32((p), ETI_CONFIG_CLR_REG, (m))
#define eti_set_config_audio_enc_en(p)		HOI_WR32(p, ETI_CONFIG_SET_REG, ETI_CONFIG_AUDIO_ENC_EN)
#define eti_set_config_video_enc_en(p)		HOI_WR32(p, ETI_CONFIG_SET_REG, ETI_CONFIG_VIDEO_ENC_EN)
#define eti_set_config_audio_enc_dis(p)		HOI_WR32(p, ETI_CONFIG_CLR_REG, ETI_CONFIG_AUDIO_ENC_EN)
#define eti_set_config_video_enc_dis(p)		HOI_WR32(p, ETI_CONFIG_CLR_REG, ETI_CONFIG_VIDEO_ENC_EN)
#define eti_set_config_aud_pkt_drop(p)		HOI_WR32(p, ETI_CONFIG_SET_REG, ETI_CONFIG_AUD_PKT_DROP)
#define eti_set_config_vid_pkt_drop(p)		HOI_WR32(p, ETI_CONFIG_SET_REG, ETI_CONFIG_VID_PKT_DROP)
#define eti_get_irq1(p)                     HOI_REG_RD(p, ETI_STATUS_REG, ETI_STATUS_IRQ1)
#define eti_clr_irq1(p)                     HOI_REG_CLR(p, ETI_STATUS_REG, ETI_STATUS_IRQ1)
#define eti_set_config_start(p)			    HOI_WR32(p, ETI_CONFIG_SET_REG, ETI_CONFIG_FSM_EN)
#define eti_set_config_stop(p)			    HOI_WR32(p, ETI_CONFIG_CLR_REG, ETI_CONFIG_FSM_EN)

#define eti_set_burst_size_reg(p, v)		HOI_WR32((p), ETI_BURST_SIZE_REG, (v))

#define eti_set_cpu_desc(p, d)              rbf_set_dsc((p), ETI_CPU_START_REG, (d))
#define eti_set_cpu_start_desc(p, v)		RBF_SET_REG((p), ETI_CPU_START_REG, (v))
#define eti_set_cpu_stop_desc(p, v)		    RBF_SET_REG((p), ETI_CPU_STOP_REG, (v))
#define eti_set_cpu_read_desc(p, v)		    RBF_SET_REG((p), ETI_CPU_READ_REG, (v))
#define eti_set_cpu_write_desc(p, v)		RBF_SET_REG((p), ETI_CPU_WRITE_REG, (v))

#define eti_set_cpu_filter_mask(p, v)		HOI_WR32((p), ETI_CPU_FILTER_MASK_REG, (v))
#define eti_set_vid_filter_mask(p, v)		HOI_WR32((p), ETI_VID_FILTER_MASK_REG, (v))
#define eti_set_aud_emb_filter_mask(p, v)	HOI_WR32((p), ETI_AUD_EMB_FILTER_MASK_REG, (v))
#define eti_set_aud_board_filter_mask(p, v) HOI_WR32((p), ETI_AUD_BOARD_FILTER_MASK_REG, (v))
#define eti_set_aud_emb_fec_col_filter_mask(p, v)   HOI_WR32((p), ETI_AUD_EMB_FEC_COL_FILTER_MASK_REG, (v))
#define eti_set_aud_emb_fec_row_filter_mask(p, v)   HOI_WR32((p), ETI_AUD_EMB_FEC_ROW_FILTER_MASK_REG, (v))
#define eti_set_aud_board_fec_col_filter_mask(p, v) HOI_WR32((p), ETI_AUD_BOARD_FEC_COL_FILTER_MASK_REG, (v))
#define eti_set_aud_board_fec_row_filter_mask(p, v) HOI_WR32((p), ETI_AUD_BOARD_FEC_ROW_FILTER_MASK_REG, (v))
#define eti_set_vid_fec_col_filter_mask(p, v)   HOI_WR32((p), ETI_VID_FEC_COL_FILTER_MASK_REG, (v))
#define eti_set_vid_fec_row_filter_mask(p, v)   HOI_WR32((p), ETI_VID_FEC_ROW_FILTER_MASK_REG, (v))

#define eti_set_device_ip(p, v)			    HOI_WR32((p), ETI_DEVICE_IP_REG, swap32(v))
#define eti_set_vid_src_ip(p, v)		    HOI_WR32((p), ETI_VID_SRC_IP_REG, swap32(v))
#define eti_set_aud_src_ip(p, v)		    HOI_WR32((p), ETI_AUD_SRC_IP_REG, swap32(v))
#define eti_set_aud_emb_dst_udp_port(p, v)  HOI_WR32((p), ETI_AUD_EMB_DST_UDP_PORT_REG, swap16(v))
#define eti_set_aud_board_dst_udp_port(p, v) HOI_WR32((p), ETI_AUD_BOARD_DST_UDP_PORT_REG, swap16(v))
#define eti_set_vid_dst_udp_port(p, v)		HOI_WR32((p), ETI_VID_DST_UDP_PORT_REG, swap16(v))

#define eti_set_aes_session_key_0(p, v)		HOI_WR32((p), ETI_AES_SESSION_KEY0_REG, (v))
#define eti_set_aes_session_key_1(p, v)		HOI_WR32((p), ETI_AES_SESSION_KEY1_REG, (v))
#define eti_set_aes_session_key_2(p, v)		HOI_WR32((p), ETI_AES_SESSION_KEY2_REG, (v))
#define eti_set_aes_session_key_3(p, v)		HOI_WR32((p), ETI_AES_SESSION_KEY3_REG, (v))
#define eti_set_aes_riv_0(p, v)			    HOI_WR32((p), ETI_AES_RIV0_REG, (v))
#define eti_set_aes_riv_1(p, v)			    HOI_WR32((p), ETI_AES_RIV1_REG, (v))


/* macros get*/
#define eti_get_config_reg(p)			    HOI_RD32((p), ETI_CONFIG_SET_REG)
#define eti_get_config_audio_enc_en(p)		HOI_REG_TST(p, ETI_CONFIG_SET_REG, ETI_CONFIG_AUDIO_ENC_EN)
#define eti_get_config_video_enc_en(p)		HOI_REG_TST(p, ETI_CONFIG_SET_REG, ETI_CONFIG_VIDEO_ENC_EN)
#define eti_get_config_start(p)			    HOI_REG_TST(p, ETI_CONFIG_SET_REG, ETI_CONFIG_FSM_EN)

#define eti_get_status_reg(p)			    HOI_RD32((p), ETI_STATUS_REG)
#define eti_get_burst_size_reg(p)		    HOI_RD32((p), ETI_BURST_SIZE_REG)

#define eti_report_cpu_desc(p)              rbf_report_dsc((p), ETI_CPU_START_REG)
#define eti_report_aud_desc(p)              rbf_report_dsc((p), ETI_AUD_START_REG)
#define eti_report_vid_desc(p)              rbf_report_dsc((p), ETI_VID_START_REG)
#define eti_get_cpu_desc(p, d)              rbf_get_dsc((p), ETI_CPU_START_REG, (d))
#define eti_get_cpu_start_desc(p)           RBF_GET_REG((p), ETI_CPU_START_REG)
#define eti_get_cpu_stop_desc(p)            RBF_GET_REG((p), ETI_CPU_STOP_REG)
#define eti_get_cpu_read_desc(p)            RBF_GET_REG((p), ETI_CPU_READ_REG)
#define eti_get_cpu_write_desc(p)           RBF_GET_REG((p), ETI_CPU_WRITE_REG)
#define eti_get_vid_desc(p, d)              rbf_get_dsc((p), ETI_VID_START_REG, (d))
#define eti_get_vid_start_desc(p)           RBF_GET_REG((p), ETI_VID_START_REG)
#define eti_get_vid_stop_desc(p)            RBF_GET_REG((p), ETI_VID_STOP_REG)
#define eti_get_vid_read_desc(p)            RBF_GET_REG((p), ETI_VID_READ_REG)
#define eti_get_vid_write_desc(p)           RBF_GET_REG((p), ETI_VID_WRITE_REG)
#define eti_get_aud_desc(p, d)              rbf_get_dsc((p), ETI_AUD_START_REG, (d))
#define eti_get_aud_start_desc(p)           RBF_GET_REG((p), ETI_AUD_START_REG)
#define eti_get_aud_stop_desc(p)            RBF_GET_REG((p), ETI_AUD_STOP_REG)
#define eti_get_aud_read_desc(p)            RBF_GET_REG((p), ETI_AUD_READ_REG)
#define eti_get_aud_write_desc(p)           RBF_GET_REG((p), ETI_AUD_WRITE_REG)

#define eti_get_cpu_filter_mask(p)		    HOI_RD32((p), ETI_CPU_FILTER_MASK_REG)
#define eti_get_vid_filter_mask(p)		    HOI_RD32((p), ETI_VID_FILTER_MASK_REG)
#define eti_get_aud_emb_filter_mask(p)      HOI_RD32((p), ETI_AUD_EMB_FILTER_MASK_REG)

#define eti_get_device_ip(p)			    swap32(HOI_RD32((p), ETI_DEVICE_IP_REG))
#define eti_get_vid_src_ip(p)			    swap32(HOI_RD32((p), ETI_VID_SRC_IP_REG))
#define eti_get_aud_src_ip(p)			    swap32(HOI_RD32((p), ETI_AUD_SRC_IP_REG))
#define eti_get_aud_dst_udp_port(p)		    swap16(HOI_RD32((p), ETI_AUD_DST_UDP_PORT_REG))
#define eti_get_vid_dst_udp_port(p)		    swap16(HOI_RD32((p), ETI_VID_DST_UDP_PORT_REG))

#define eti_get_aes_session_key_0(p)		HOI_RD32((p), ETI_AES_SESSION_KEY0_REG)
#define eti_get_aes_session_key_1(p)		HOI_RD32((p), ETI_AES_SESSION_KEY1_REG)
#define eti_get_aes_session_key_2(p)		HOI_RD32((p), ETI_AES_SESSION_KEY2_REG)
#define eti_get_aes_session_key_3(p)		HOI_RD32((p), ETI_AES_SESSION_KEY3_REG)
#define eti_get_aes_riv_0(p)			    HOI_RD32((p), ETI_AES_RIV0_REG)
#define eti_get_aes_riv_1(p)			    HOI_RD32((p), ETI_AES_RIV1_REG)

#define eti_get_aud_emb_packet_cnt(p)	    HOI_RD32((p), ETI_AUD_EMB_PACKET_CNT_REG)
#define eti_get_aud_board_packet_cnt(p)     HOI_RD32((p), ETI_AUD_BOARD_PACKET_CNT_REG)
#define eti_get_vid_packet_cnt(p)		    HOI_RD32((p), ETI_VID_PACKET_CNT_REG)
#define eti_get_cpu_packet_cnt(p)		    HOI_RD32((p), ETI_CPU_PACKET_CNT_REG)
#define eti_get_inv_packet_cnt(p)		    HOI_RD32((p), ETI_INV_PACKET_CNT_REG)
#define eti_get_debug_reg(p)			    HOI_RD32((p), ETI_DEBUG_REG)

#define eti_get_status_fifo_empty(p)		HOI_REG_TST(p, ETI_STATUS_REG, ETI_STATUS_FIFO_EMPTY)
#define eti_get_status_fsm_idle(p)		    HOI_REG_TST(p, ETI_STATUS_REG, ETI_STATUS_FSM_IDLE)
#define eti_get_status_invalid_frame(p)		HOI_REG_TST(p, ETI_STATUS_REG, ETI_STATUS_INVALID_FRAME)
#define eti_get_status_cpu_frame(p)		    HOI_REG_TST(p, ETI_STATUS_REG, ETI_STATUS_CPU_FRAME)
#define eti_get_status_vid_frame(p)		    HOI_REG_TST(p, ETI_STATUS_REG, ETI_STATUS_VIDEO_FRAME)
#define eti_get_status_aud_frame(p)		    HOI_REG_TST(p, ETI_STATUS_REG, ETI_STATUS_AUDIO_FRAME)
#define eti_get_status_mac_err(p)		    HOI_REG_TST(p, ETI_STATUS_REG, ETI_STATUS_MAC_ERROR)

#define eti_get_frame_period(p)             HOI_RD32((p), ETI_FRAME_PERIOD)
#define eti_get_frame_gap(p)                HOI_RD32((p), ETI_FRAME_GAP)

#endif /* ETI_HAL_H_ */






