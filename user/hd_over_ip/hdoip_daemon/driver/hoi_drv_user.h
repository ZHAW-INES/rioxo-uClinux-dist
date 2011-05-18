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
int hoi_drv_buf(void* ar, size_t arl, void* vr, size_t vrl, void* at, size_t atl, void* vt, size_t vtl);

int hoi_drv_eti(uint32_t addr_dst, uint32_t addr_src, uint32_t vid, uint32_t aud);
int hoi_drv_vsi(uint32_t compress, uint32_t encrypt, int bandwidth, hdoip_eth_params* eth, t_video_timing* timing, uint32_t* advcnt);
int hoi_drv_vso(uint32_t compress, uint32_t encrypt, t_video_timing* timing, uint32_t advcnt, uint32_t delay_ms);
int hoi_drv_asi(uint32_t cfg, hdoip_eth_params* eth, uint32_t fs, uint32_t width, uint32_t cnt, uint8_t* sel);
int hoi_drv_aso(uint32_t fs, uint32_t fs_tol, uint32_t width, uint32_t cnt, uint8_t* sel, uint32_t delay_ms, uint32_t cfg);
int hoi_drv_capture(bool compress, void* buffer, size_t size, t_video_timing* timing, uint32_t* advcnt);
int hoi_drv_show(bool compress, void* buffer, t_video_timing* timing, uint32_t advcnt);
int hoi_drv_debug(void);
int hoi_drv_bw(uint32_t bandwidth);

int hoi_drv_info(t_video_timing* timing, uint32_t *advcnt);
int hoi_drv_info_all(t_hoi_msg_info** nfo);
int hoi_drv_ethstat(t_hoi_msg_ethstat** stat);
int hoi_drv_vsostat(t_hoi_msg_vsostat** stat);
int hoi_drv_viostat(t_hoi_msg_viostat** stat);

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
int hoi_drv_wraudtag(void* buffer);
int hoi_drv_getevent(uint32_t* event);
int hoi_drv_set_mtime(uint32_t p);
int hoi_drv_get_mtime(uint32_t* p);
int hoi_drv_set_stime(uint32_t p);
int hoi_drv_get_stime(uint32_t* p);
int hoi_drv_get_syncdelay(uint32_t* p);
int hoi_drv_timer(uint32_t p);
int hoi_drv_osdon();
int hoi_drv_osdoff();
int hoi_drv_hpdon();
int hoi_drv_hpdoff();
int hoi_drv_hpdreset();
int hoi_drv_loop();
int hoi_drv_repair();

int hoi_drv_hdcp_timer_enable();
int hoi_drv_hdcp_timer_disable();
int hoi_drv_hdcp_timer_load();
int hoi_drv_hdcp_set_timer(uint32_t start_time);
int hoi_drv_hdcp_get_key(uint32_t key[4]);

int hoi_drv_wdg_enable();
int hoi_drv_wdg_disable();
int hoi_drv_wdg_service();
int hoi_drv_wdg_init(uint32_t service_time);

int hoi_drv_poll();
int hoi_drv_getversion(t_hoic_getversion* cmd);

int hoi_drv_reset(uint32_t rv);

#endif /* HOI_DRV_USER_H_ */
