/*
 * rtsp_command.h
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#ifndef RTSP_COMMAND_H_
#define RTSP_COMMAND_H_

#include "rtsp_connection.h"

void rtsp_response_line(t_rtsp_connection* msg, int code, char* reason);
void rtsp_response_error(t_rtsp_connection* msg, int code, char* reason);
void rtsp_request_line(t_rtsp_connection* msg, char* method, char* uri);
void rtsp_request_cseq(t_rtsp_connection* msg, int s);
void rtsp_header_session(t_rtsp_connection* msg, char* s);
void rtsp_header_timing(t_rtsp_connection* msg, t_video_timing* timing);
void rtsp_header_rtp_format(t_rtsp_connection* msg, t_rtsp_rtp_format* p);
void rtsp_eoh(t_rtsp_connection* msg);

void rtsp_request_setup(t_rtsp_connection* msg, char* uri, t_rtsp_transport* transport);
void rtsp_response_setup(t_rtsp_connection* msg, t_rtsp_transport* transport, char* session);
void rtsp_request_play(t_rtsp_connection* msg, char* uri, char* session, t_rtsp_rtp_format* fmt);
void rtsp_response_play(t_rtsp_connection* msg, char* session, t_rtsp_rtp_format* fmt, t_video_timing* timing);



#endif /* RTSP_COMMAND_H_ */
