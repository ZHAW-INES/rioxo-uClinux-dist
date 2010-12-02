/*
 * rtsp_command.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "rtsp.h"
#include "rtsp_command.h"

void rtsp_response_line(t_rtsp_connection* msg, int code, char* reason)
{
    rtsp_msginit(msg);
    msgprintf(msg, "%s %03d %s\r\n",
        RTSP_VERSION,
        code,
        reason);
    rtsp_request_cseq(msg, msg->sequence);
}

void rtsp_response_error(t_rtsp_connection* msg, int code, char* reason)
{
    rtsp_response_line(msg, code, reason);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_request_line(t_rtsp_connection* msg, char* method, char* uri)
{
    rtsp_msginit(msg);
    msgprintf(msg, "%s %s %s\r\n",
        method,
        uri,
        RTSP_VERSION);
    rtsp_request_cseq(msg, msg->sequence);
}

void rtsp_request_cseq(t_rtsp_connection* msg, int s)
{
    msgprintf(msg, "CSeq: %03d\r\n", s);
}

void rtsp_eoh(t_rtsp_connection* msg)
{
    msgprintf(msg, "\r\n");
}


char* rtsp_str_multicast(bool b)
{
    if (b) return "multicast"; else return "unicast";
}

void rtsp_header_session(t_rtsp_connection* msg, char* s)
{
    msgprintf(msg, "Session: %s\r\n", s);
}

void rtsp_header_transport(t_rtsp_connection* msg, t_rtsp_transport* t)
{
    msgprintf(msg, "Transport: RTP/AVP;%s;", rtsp_str_multicast(t->multicast));
    if (t->port) msgprintf(msg, "port=%d-%d;", PORT_RANGE_START(t->port), PORT_RANGE_STOP(t->port));
    if (t->client_port) msgprintf(msg, "client_port=%d-%d;", PORT_RANGE_START(t->client_port), PORT_RANGE_STOP(t->client_port));
    if (t->server_port) msgprintf(msg, "server_port=%d-%d;", PORT_RANGE_START(t->server_port), PORT_RANGE_STOP(t->server_port));
    msgprintf(msg, "mode=\"PLAY\"\r\n");
}

void rtsp_header_timing(t_rtsp_connection* msg, t_video_timing* timing)
{
    // Timing: pfreq width fp p bp pol height fp p bp pol
    msgprintf(msg, "Timing: %d %d %d %d %d %s %d %d %d %d %s",
            timing->pfreq, timing->width, timing->hfront, timing->hpulse, timing->hback,
            (timing->hpolarity ? "P " : "N "),
            timing->height, timing->vfront, timing->vpulse, timing->vback,
            (timing->vpolarity ? "P" : "N"));
}

void rtsp_header_rtp_format(t_rtsp_connection* msg, t_rtsp_rtp_format* p)
{
    // RTP-Format: compress[ value] rtptime
    if (p->compress == FORMAT_JPEG2000) {
        msgprintf(msg, "RTP-Format: jp2k %d %d", p->value, p->rtptime);
    } else {
        msgprintf(msg, "RTP-Format: plain %d", p->rtptime);
    }

}

void rtsp_request_setup(t_rtsp_connection* msg, char* uri, t_rtsp_transport* transport)
{
    rtsp_request_line(msg, "SETUP", uri);
    rtsp_header_transport(msg, transport);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_response_setup(t_rtsp_connection* msg, t_rtsp_transport* transport, char* session)
{
    rtsp_response_line(msg, RTSP_SC_OK, "OK");
    rtsp_header_session(msg, session);
    rtsp_header_transport(msg, transport);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_request_play(t_rtsp_connection* msg, char* uri, char* session, t_rtsp_rtp_format* fmt)
{
    rtsp_request_line(msg, "PLAY", uri);
    rtsp_header_session(msg, session);
    rtsp_header_rtp_format(msg, fmt);
    rtsp_eoh(msg);
    rtsp_send(msg);

}

void rtsp_response_play(t_rtsp_connection* msg, char* session, t_rtsp_rtp_format* fmt, t_video_timing* timing)
{
    rtsp_response_line(msg, RTSP_SC_OK, "OK");
    rtsp_header_session(msg, session);
    rtsp_header_rtp_format(msg, fmt);
    rtsp_header_timing(msg, timing);
    rtsp_eoh(msg);
    rtsp_send(msg);
}


