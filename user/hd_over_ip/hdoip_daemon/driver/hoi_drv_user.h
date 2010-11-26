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
int hoi_drv_ifmt(int f, t_video_format fmt);
int hoi_drv_ofmt(int f, t_video_format fmt);
int hoi_drv_pfmt(int f, t_video_format fmt);
int hoi_drv_rdifmt(int f, t_video_format *fmt);
int hoi_drv_rdofmt(int f, t_video_format *fmt);
int hoi_drv_capture(int f, bool compress, void* buffer, size_t size, t_video_timing* timing, uint32_t* advcnt);
int hoi_drv_show(int f, bool compress, void* buffer, t_video_timing* timing, uint32_t advcnt);
int hoi_drv_rdedid(int f, void* buffer);
int hoi_drv_wredid(int f, void* buffer);
int hoi_drv_rdvidtag(int f, void* buffer, bool* available);
int hoi_drv_wrvidtag(int f, void* buffer);
int hoi_drv_rdaudtag(int f, void* buffer, bool* available);
int hoi_drv_wraudtag(int f, void* buffer);
int hoi_drv_getevent(int f, uint32_t* event);

#endif /* HOI_DRV_USER_H_ */
