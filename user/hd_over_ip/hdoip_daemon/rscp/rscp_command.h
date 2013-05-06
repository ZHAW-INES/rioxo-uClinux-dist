/*
 * rscp_command.h
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#ifndef RSCP_COMMAND_H_
#define RSCP_COMMAND_H_

#include "rscp_connection.h"

#define rscp_err_server(x)          rscp_response_error(x, 400, "Server error")
#define rscp_err_busy(x)            rscp_response_error(x, 404, "Busy")
#define rscp_err_no_vtb(x)          rscp_response_error(x, 405, "Not a VTB")
#define rscp_err_retry(x)           rscp_response_error(x, 300, "Please retry later")
#define rscp_err_no_source(x)       rscp_response_error(x, 406, "No Input")
#define rscp_err_def_source(x)      rscp_response_error(x, 407, "No Audio Input")
#define rscp_err_hdcp(x)            rscp_response_error(x, 408, "HDCP error")
#define rscp_err_multicast(x)       rscp_response_error(x, 409, "Audio board in multicast")

void rscp_response_line(t_rscp_connection* msg, int code, char* reason);
void rscp_response_error(t_rscp_connection* msg, int code, char* reason);
void rscp_request_line(t_rscp_connection* msg, char* method, char* uri);
void rscp_request_cseq(t_rscp_connection* msg, int s);
void rscp_header_session(t_rscp_connection* msg, char* s);
void rscp_header_timing(t_rscp_connection* msg, t_video_timing* timing);
void rscp_header_rtp_format(t_rscp_connection* msg, t_rscp_rtp_format* p);
void rscp_header_usb(t_rscp_connection* msg, char* mouse, char* keyboard, char* storage);
void rscp_eoh(t_rscp_connection* msg);

void rscp_request_setup(t_rscp_connection* msg, char* uri, t_rscp_transport* transport, t_rscp_edid *edid, t_rscp_hdcp* hdcp);
void rscp_request_play(t_rscp_connection* msg, char* uri, char* session, t_rscp_rtp_format* fmt);
void rscp_request_teardown(t_rscp_connection* msg, char* uri, char* session);
void rscp_request_update(t_rscp_connection* msg, char* uri, char* session, uint32_t event, t_rscp_rtp_format* fmt);

void rscp_request_pause(t_rscp_connection* msg, char* uri, char* session);
void rscp_request_hello(t_rscp_connection* msg, char* uri);
void rscp_request_hdcp(t_rscp_connection* msg, char* session, char* uri, char* id, char* content);
void rscp_request_usb_setup(t_rscp_connection* msg, char* uri, t_rscp_transport* transport);
void rscp_request_usb_play(t_rscp_connection* msg, char* uri, char* session, char* mouse_msg, char* keyboard_msg, char* storage_msg);
void rscp_request_usb_teardown(t_rscp_connection* msg, char* uri, char* session);

void rscp_response_pause(t_rscp_connection* msg, char* session);
void rscp_response_setup(t_rscp_connection* msg, t_rscp_transport* transport, char* session, t_rscp_hdcp* hdcp);
void rscp_response_play(t_rscp_connection* msg, char* session, t_rscp_rtp_format* fmt, t_video_timing* timing);
void rscp_response_teardown(t_rscp_connection* msg, char* session);
void rscp_response_hdcp(t_rscp_connection* msg, char* session, char* id, char* content);
void rscp_response_usb_setup(t_rscp_connection* msg, t_rscp_transport* transport, char* session);
void rscp_response_usb_play(t_rscp_connection* msg, char* session);
void rscp_response_usb_teardown(t_rscp_connection* msg, char* session);

#endif /* RSCP_COMMAND_H_ */
