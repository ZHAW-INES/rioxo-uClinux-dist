/*
 * eso_drv.h
 *
 * Created on: 08.12.2010
 *     Author: buan
 */

#ifndef ETO_DRV_H_
#define ETO_DRV_H_

#include "std.h"
#include "stdrbf.h"
#include "stdvid.h"
#include "eto_hal.h"
#include "eto_drv_cfg.h"
#include "fec_tx_struct.h"

// hdoip_ether functions
int eto_drv_start(t_eto* handle);
int eto_drv_stop(t_eto* handle);
int eto_drv_set_cpu_buf(t_eto* handle, void* start_ptr, size_t size);
int eto_drv_init(t_eto* handle, void* p_eto);

// hdoip_core functions
int eto_drv_set_ptr(t_eto* handle, void* ptr);
int eto_drv_handler(t_eto* handle, t_queue* event_queue);
int eto_drv_set_aes(t_eto* handle, uint32_t key[4], uint32_t riv[2]);
int eto_drv_get_aes(t_eto* handle, uint32_t key[4], uint32_t riv[2]);
int eto_drv_aes_aud_en(t_eto* handle);
int eto_drv_aes_aud_dis(t_eto* handle);
int eto_drv_aes_stat(t_eto* handle, uint32_t *audio_enc_en, uint32_t *video_enc_en);
int eto_drv_aes_vid_en(t_eto* handle);
int eto_drv_aes_vid_dis(t_eto* handle);
int eto_drv_start_aud(t_eto* handle);
int eto_drv_stop_aud(t_eto* handle);
int eto_drv_start_vid(t_eto* handle);
int eto_drv_stop_vid(t_eto* handle);
int eto_drv_set_aud_buf(t_eto* handle, void* start_ptr, size_t size);
int eto_drv_set_vid_buf(t_eto* handle, void* start_ptr, size_t size);
void eto_drv_frame_rate_reduction(t_eto* handle, int reduction);
void eto_drv_set_frame_period(t_eto* handle, t_video_timing* timing, t_fec_setting* fec, int enable);

#endif /* ETO_DRV_H_ */
