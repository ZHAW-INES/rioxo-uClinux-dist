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
#include <sys/types.h>
#include "hoi_msg.h"

int hoi_drv_ldrv(int f, uint32_t drivers, pid_t pid);
int hoi_drv_buf(int f, void* ar, size_t arl, void* vr, size_t vrl, void* at, size_t atl, void* vt, size_t vtl);

int hoi_drv_eti(int f, uint32_t addr_dst, uint32_t addr_src, uint32_t vid, uint32_t aud);
int hoi_drv_vsi(int f, bool compress, int bandwidth, hdoip_eth_params* eth, t_video_timing* timing, uint32_t* advcnt);
int hoi_drv_vso(int f, bool compress, t_video_timing* timing, uint32_t advcnt);
int hoi_drv_capture(int f, bool compress, void* buffer, size_t size, t_video_timing* timing, uint32_t* advcnt);
int hoi_drv_show(int f, bool compress, void* buffer, t_video_timing* timing, uint32_t advcnt);


int hoi_drv_ifmt(int f, t_video_format fmt);
int hoi_drv_ofmt(int f, t_video_format fmt);
int hoi_drv_pfmt(int f, t_video_format fmt);
int hoi_drv_rdifmt(int f, t_video_format *fmt);
int hoi_drv_rdofmt(int f, t_video_format *fmt);
int hoi_drv_rdedid(int f, void* buffer);
int hoi_drv_wredid(int f, void* buffer);
int hoi_drv_rdvidtag(int f, void* buffer, bool* available);
int hoi_drv_wrvidtag(int f, void* buffer);
int hoi_drv_rdaudtag(int f, void* buffer, bool* available);
int hoi_drv_wraudtag(int f, void* buffer);
int hoi_drv_getevent(int f, uint32_t* event);
int hoi_drv_set_mtime(int f, uint32_t p);
int hoi_drv_get_mtime(int f, uint32_t* p);
int hoi_drv_osdon(int f);
int hoi_drv_osdoff(int f);
int hoi_drv_loop(int f);

#endif /* HOI_DRV_USER_H_ */
