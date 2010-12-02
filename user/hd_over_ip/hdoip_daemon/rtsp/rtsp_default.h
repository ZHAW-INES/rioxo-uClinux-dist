/*
 * rtsp_default.h
 *
 *  Created on: 24.11.2010
 *      Author: alda
 */

#ifndef RTSP_DEFAULT_H_
#define RTSP_DEFAULT_H_

#include "rtsp.h"

void rtsp_default_transport(t_rtsp_transport* t);

void rtsp_default_response_setup(t_rtsp_rsp_setup* p);
void rtsp_default_response_play(t_rtsp_rsp_play* p);

#endif /* RTSP_DEFAULT_H_ */
