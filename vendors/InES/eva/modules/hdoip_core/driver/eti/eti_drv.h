/*
 * eti_drv.h
 *
 * Created on: 08.12.2010
 *     Author: buan
 */

#ifndef ETI_DRV_H_
#define ETI_DRV_H_

#include "std.h"
#include "eti_hal.h"
#include "eti_drv_cfg.h"

// hdoip_ether functions
int eti_drv_start(t_eti* handle);
int eti_drv_stop(t_eti* handle);
int eti_drv_set_cpu_buf(t_eti* handle, void* start_ptr, size_t size);
int eti_drv_init(t_eti* handle, void* ptr);

// hdoip_core functions
int eti_drv_set_ptr(t_eti* handle, void* ptr);
int eti_drv_handler(t_eti* handle, t_queue* event_queue);
int eti_drv_set_aes(t_eti* handle, uint32_t key[4], uint32_t riv[2]);
int eti_drv_get_aes(t_eti* handle, uint32_t key[4], uint32_t riv[2]);
int eti_drv_aes_aud_en(t_eti* handle);
int eti_drv_aes_aud_dis(t_eti* handle);
int eti_drv_aes_stat(t_eti* handle, uint32_t *audio_enc_en, uint32_t *video_enc_en);
int eti_drv_aes_vid_en(t_eti* handle);
int eti_drv_aes_vid_dis(t_eti* handle);
int eti_drv_start_aud_emb(t_eti* handle, bool fec_disable);
int eti_drv_start_aud_board(t_eti* handle, bool fec_disable);
int eti_drv_stop_aud_emb(t_eti* handle);
int eti_drv_stop_aud_board(t_eti* handle);
int eti_drv_start_vid(t_eti* handle, bool fec_disable);
int eti_drv_stop_vid(t_eti* handle);
int eti_drv_set_filter(t_eti* handle, uint32_t dev_ip, uint32_t aud_src_ip, uint32_t vid_src_ip, uint16_t aud_emb_port, uint16_t aud_board_port, uint16_t vid_port);

#endif /* ETI_DRV_H_ */
