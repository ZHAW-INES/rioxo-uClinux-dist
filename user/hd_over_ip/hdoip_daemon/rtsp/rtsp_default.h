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

void rtsp_default_response(t_rtsp_rsp_setup* p);
void rtsp_default_response_session(t_rtsp_rsp_setup* p);
void rtsp_default_response_setup(t_rtsp_rsp_setup* p);
void rtsp_default_response_play(t_rtsp_rsp_play* p);
void rtsp_default_response_pause(t_rtsp_rsp_pause* p);
void rtsp_default_response_teardown(t_rtsp_rsp_teardown* p);
void rtsp_default_response_hdcp(t_rtsp_rsp_hdcp* p);

#endif /* RTSP_DEFAULT_H_ */
