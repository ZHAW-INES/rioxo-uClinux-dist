/*
 * rscp_parse_request.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RSCP_PARSE_REQUEST_H_
#define RSCP_PARSE_REQUEST_H_

#include "rscp.h"
#include "rscp_string.h"
#include "rscp_connection.h"
#include "map.h"

typedef struct {
    char        line[100];
    t_str_request_line rq;
    t_str_uri   uri;
    char        session[20];
    char        content_type[50];
    size_t      content_length;
} t_rscp_header_common;

int rscp_parse_ip(char* str, uint32_t* p);
int rscp_parse_timing(char* line, t_video_timing* timing);
int rscp_parse_rtp_format(char* line, t_rscp_rtp_format* p);
int rscp_parse_uint32(char* line, uint32_t* p);
int rscp_parse_transport(char* line, t_rscp_transport* p);
int rscp_parse_rtp_info(char* line, t_rscp_rtp_info* p);
int rscp_parse_ui32(char* line, uint32_t* p);
int rscp_parse_str(char* line, char* p);
int rscp_parse_edid(char* line, t_rscp_edid *edid);
int rscp_parse_hdcp(char* line, t_rscp_hdcp *hdcp);
int rscp_parse_usb(char* line, t_rscp_usb *usb);

int rscp_ommit_header(t_rscp_connection* con, int timeout);
int rscp_parse_header(t_rscp_connection* con, const t_map_fnc attr[], void* base, t_rscp_header_common* common, int timeout);
int rscp_parse_response(t_rscp_connection* con, const t_map_fnc attr[], void* base, t_rscp_header_common* common, int timeout);
int rscp_parse_request(t_rscp_connection* con, const t_map_set srv_method[], const t_map_set** method, void* req, t_rscp_header_common* common);

#endif /* RSCP_PARSE_REQUEST_H_ */
