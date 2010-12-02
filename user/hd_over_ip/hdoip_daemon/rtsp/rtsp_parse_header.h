/*
 * rtsp_parse_request.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_PARSE_REQUEST_H_
#define RTSP_PARSE_REQUEST_H_

#include "rtsp.h"
#include "rtsp_connection.h"
#include "map.h"

int rtsp_parse_timing(char* line, t_video_timing* timing);
int rtsp_parse_rtp_format(char* line, t_rtsp_rtp_format* p);
int rtsp_parse_transport(char* line, t_rtsp_transport* p);
int rtsp_parse_rtp_info(char* line, t_rtsp_rtp_info* p);
int rtsp_parse_ui32(char* line, uint32_t* p);
int rtsp_parse_str(char* line, char* p);

int rtsp_ommit_header(t_rtsp_connection* con);
int rtsp_parse_header(t_rtsp_connection* con, t_map_fnc* attr, void* base, char* session);
int rtsp_parse_response(t_rtsp_connection* con, t_map_fnc* attr, void* base, char* session);

#endif /* RTSP_PARSE_REQUEST_H_ */
