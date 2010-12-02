/*
 * hoi_res.h
 *
 *  Created on: 20.10.2010
 *      Author: alda
 */

#ifndef HOI_RES_H_
#define HOI_RES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include "hoi_msg.h"

int hoi_res_fps(t_video_timing* p_vt);
t_video_timing* hoi_res_timing(uint32_t width, uint32_t height, uint32_t fps);

#endif /* HOI_RES_H_ */
