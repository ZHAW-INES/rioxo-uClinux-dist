/*
 * hdoipd_callback.h
 *
 *  Created on: 20.01.2011
 *      Author: alda
 */

#ifndef HDOIPD_CALLBACK_H_
#define HDOIPD_CALLBACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hoi_drv_user.h"
#include "hoi_image.h"
#include "hoi_res.h"
#include "hoi_cfg.h"
#include "hdoipd_msg.h"
#include "hdoipd_osd.h"
#include "hdoipd_fsm.h"
#include "hdoipd_task.h"
#include "rscp_include.h"

#include "vrb_video.h"
#include "vtb_video.h"
#include "vrb_audio_emb.h"
#include "vrb_audio_board.h"
#include "vtb_audio_emb.h"
#include "vtb_audio_board.h"
#include "box_sys.h"

int hdoipd_media_callback(t_rscp_media* media, int (*f)(t_rscp_media*, void*), void* d);

#endif /* HDOIPD_CALLBACK_H_ */
