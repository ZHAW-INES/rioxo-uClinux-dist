/*
 * box_sys_vtb.h
 *
 *  Created on: 06.12.2012
 *      Author: montesas
 */

#ifndef BOX_SYS_VTB_H_
#define BOX_SYS_VTB_H_

#include "rtsp_media.h"

extern t_rtsp_media box_sys_vtb;

int box_sys_vtb_get_parameter(t_rtsp_media *media, void *data, t_rtsp_connection *rsp);

#endif /* BOX_SYS_VTB_H_ */
