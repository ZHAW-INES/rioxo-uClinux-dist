/*
 * hoi_drv_user.h
 *
 *  Created on: 18.10.2010
 *      Author: alda
 */

#ifndef HOI_DRV_USER_H_
#define HOI_DRV_USER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include "hoi_msg.h"
#include "hdoipd_msg.h"

const char* event_str(uint32_t e);

int hoi_drv_ldrv(uint32_t drivers);
int hoi_drv_buf(void* at, size_t atl, void* vt, size_t vtl);
int hoi_drv_hdcp(uint32_t* hdcp_keys);
int hoi_drv_hdcpstat(uint32_t *eto_hdcp_audio,uint32_t *eto_hdcp_video,uint32_t *eti_hdcp_audio, uint32_t *eti_hdcp_video);

int hoi_drv_eti(uint32_t addr_dst, uint32_t addr_src_vid, uint32_t addr_src_aud, uint32_t vid, uint32_t aud);
int hoi_drv_vsi(uint32_t compress, uint32_t chroma, uint32_t encrypt, int bandwidth, hdoip_eth_params* eth, t_video_timing* timing, uint32_t* advcnt, int enable_traffic_shaping, t_fec_setting* fec);
int hoi_drv_vso(uint32_t compress, uint32_t encrypt, t_video_timing* timing, uint32_t advcnt, uint32_t delay_ms);
int hoi_drv_asi(int unsigned stream_nr, struct hdoip_eth_params* eth, struct hdoip_aud_params* aud, t_fec_setting* fec);
int hoi_drv_aso(int unsigned stream_nr, uint32_t fs, uint32_t width, uint16_t ch_map, uint32_t delay_ms, uint32_t av_delay, uint32_t cfg);
int hoi_drv_capture(bool compress, void* buffer, size_t size, t_video_timing* timing, uint32_t* advcnt);
int hoi_drv_show(bool compress, void* buffer, t_video_timing* timing, uint32_t advcnt);
int hoi_drv_debug(void);
int hoi_drv_set_timing(t_video_timing* timing);
int hoi_drv_bw(uint32_t bw, uint32_t chroma);
int hoi_drv_set_fec_tx_params(t_fec_setting* fec);
int hoi_drv_set_led_status(uint32_t instruction);
int hoi_drv_new_audio(uint32_t fs);
int hoi_drv_read_ram(uint32_t addr);
int hoi_drv_get_fs(uint32_t* p);
int hoi_drv_get_analog_timing(uint32_t* p);
int hoi_drv_get_device_id(uint32_t* p);
int hoi_drv_info(t_video_timing* timing, uint32_t *advcnt, uint32_t *advcnt_old);
int hoi_drv_info_all(t_hoi_msg_info** nfo);
int hoi_drv_ethstat(t_hoi_msg_ethstat** stat);
int hoi_drv_vsostat(t_hoi_msg_vsostat** stat);
int hoi_drv_viostat(t_hoi_msg_viostat** stat);
int hoi_drv_asoreg(t_hoi_msg_asoreg** stat);

int hoi_drv_ifmt(t_video_format fmt);
int hoi_drv_ofmt(t_video_format fmt);
int hoi_drv_pfmt(t_video_format fmt);
int hoi_drv_rdifmt(t_video_format *fmt);
int hoi_drv_rdofmt(t_video_format *fmt);
int hoi_drv_rdedid(void* buffer);
int hoi_drv_wredid(void* buffer);
int hoi_drv_rdvidtag(void* buffer, bool* available);
int hoi_drv_wrvidtag(void* buffer);
int hoi_drv_rdaudtag(void* buffer, bool* available);
int hoi_drv_wraudtag();
int hoi_drv_getevent(uint32_t* event);
int hoi_drv_set_mtime(uint32_t p);
int hoi_drv_get_mtime(uint32_t* p);
int hoi_drv_set_stime(uint32_t p);
int hoi_drv_get_stime(uint32_t* p);
int hoi_drv_set_fps_reduction(uint32_t p);
int hoi_drv_get_syncdelay(uint32_t* p);
int hoi_drv_timer(uint32_t p);
int hoi_drv_osdon();
int hoi_drv_osdoff();
int hoi_drv_osd_clr_border();
int hoi_drv_hpdon();
int hoi_drv_hpdoff();
int hoi_drv_loop();
int hoi_drv_repair();
int hoi_drv_wdg_enable();
int hoi_drv_wdg_disable();
int hoi_drv_hdcp_viden_eti();
int hoi_drv_hdcp_viden_eto();
int hoi_drv_hdcp_auden_eti();
int hoi_drv_hdcp_auden_eto();
int hoi_drv_hdcp_viddis_eti();
int hoi_drv_hdcp_viddis_eto();
int hoi_drv_hdcp_auddis_eti();
int hoi_drv_hdcp_auddis_eto();
int hoi_drv_hdcp_adv9889dis();
int hoi_drv_hdcp_adv9889en();
int hoi_drv_hdcp_get_key(uint32_t key[4]);
int hoi_drv_hdcp_black_output();
int hoi_drv_clr_osd();
int hoi_drv_poll();
int hoi_drv_getversion(t_hoic_getversion* cmd);
int hoi_drv_getusb(t_hoic_getusb* cmd);
int hoi_drv_get_reset_to_default(uint32_t *p);
int hoi_drv_get_active_resolution(uint32_t *p);
int hoi_drv_wdg_init(uint32_t service_time);
int hoi_drv_get_encrypted_status(uint32_t *p);

int hoi_drv_reset(uint32_t rv);

#endif /* HOI_DRV_USER_H_ */
