/*
 * rtsp_server_media.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "rtsp_server.h"

extern t_rtsp_media vtb_video;

// t_rtsp_media
const t_map_rec srv_media[] = {
        { "video", &vtb_video },
        MAP_REC_NULL
};
