/*
 * rtsp_command.h
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#ifndef RTSP_COMMAND_H_
#define RTSP_COMMAND_H_

#include "rtsp_connection.h"
#include "map.h"

#define rtsp_err_server(x)          rtsp_response_error(x, 400, "Server error")
#define rtsp_err_busy(x)            rtsp_response_error(x, 404, "Busy")
#define rtsp_err_no_vtb(x)          rtsp_response_error(x, 405, "Not a VTB")
#define rtsp_err_retry(x)           rtsp_response_error(x, 300, "Please retry later")
#define rtsp_err_no_source(x)       rtsp_response_error(x, 406, "No Input")
#define rtsp_err_def_source(x)      rtsp_response_error(x, 407, "No Audio Input")
#define rtsp_err_hdcp(x)            rtsp_response_error(x, 408, "HDCP error")

void rtsp_response_line(t_rtsp_connection* msg, int code, char* reason);
void rtsp_response_error(t_rtsp_connection* msg, int code, char* reason);
void rtsp_request_line(t_rtsp_connection* msg, char* method, char* uri, char* media);
void rtsp_request_cseq(t_rtsp_connection* msg, int s);
void rtsp_header_session(t_rtsp_connection* msg, char* s);
void rtsp_header_timing(t_rtsp_connection* msg, t_video_timing* timing);
void rtsp_header_rtp_format(t_rtsp_connection* msg, t_rtsp_rtp_format* p);
void rtsp_header_usb(t_rtsp_connection* msg, char* mouse, char* keyboard, char* storage);
void rtsp_eoh(t_rtsp_connection* msg);

void rtsp_request_setup(t_rtsp_connection* msg, char* uri, char* media, t_rtsp_transport* transport, t_rtsp_edid *edid, t_rtsp_hdcp* hdcp);
void rtsp_request_play(t_rtsp_connection* msg, char* uri, char* session, char* media, t_rtsp_rtp_format* fmt);
void rtsp_request_teardown(t_rtsp_connection* msg, char* uri, char* session, char* media);
void rtsp_request_update(t_rtsp_connection* msg, char* uri, char* session, char* media, uint32_t event, t_rtsp_rtp_format* fmt);
void rtsp_request_pause(t_rtsp_connection* msg, char* uri, char* session, char* media);
void rtsp_request_get_parameter(t_rtsp_connection* msg, char* uri);
void rtsp_request_hdcp(t_rtsp_connection* msg, char* uri, char* session, char* media, char* id, char* content);
void rtsp_request_usb_setup(t_rtsp_connection* msg, char* uri, t_rtsp_transport* transport);
void rtsp_request_usb_play(t_rtsp_connection* msg, char* uri, char* session, char* mouse_msg, char* keyboard_msg, char* storage_msg);
void rtsp_request_usb_teardown(t_rtsp_connection* msg, char* uri, char* session);

void rtsp_response_pause(t_rtsp_connection* msg, char* session);
void rtsp_response_setup(t_rtsp_connection* msg, t_rtsp_transport* transport, char* session, char* media, t_rtsp_hdcp* hdcp);
void rtsp_response_play(t_rtsp_connection* msg, char* session, t_rtsp_rtp_format* fmt, t_video_timing* timing);
void rtsp_response_teardown(t_rtsp_connection* msg, char* session);
void rtsp_response_hdcp(t_rtsp_connection* msg, char* session, char* id, char* content);
void rtsp_response_usb_setup(t_rtsp_connection* msg, t_rtsp_transport* transport, char* session);
void rtsp_response_usb_play(t_rtsp_connection* msg, char* session);
void rtsp_response_usb_teardown(t_rtsp_connection* msg, char* session);

#endif /* RTSP_COMMAND_H_ */
