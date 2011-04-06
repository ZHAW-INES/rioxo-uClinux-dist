/*
 * rscp_command.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "rscp.h"
#include "rscp_net.h"
#include "rscp_command.h"

void rscp_response_line(t_rscp_connection* msg, int code, char* reason)
{
    msgprintf(msg, "%s %03d %s\r\n",
        RSCP_VERSION,
        code,
        reason);
    rscp_request_cseq(msg, msg->sequence);
}

void rscp_response_error(t_rscp_connection* msg, int code, char* reason)
{
    rscp_response_line(msg, code, reason);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_request_line(t_rscp_connection* msg, char* method, char* uri)
{
    msgprintf(msg, "%s %s %s\r\n",
        method,
        uri,
        RSCP_VERSION);
    rscp_request_cseq(msg, msg->sequence);
}

void rscp_request_cseq(t_rscp_connection* msg, int s)
{
    msgprintf(msg, "CSeq: %03d\r\n", s);
}

void rscp_eoh(t_rscp_connection* msg)
{
    msgprintf(msg, "\r\n");
}


char* rscp_str_multicast(bool b)
{
    if (b) return "multicast"; else return "unicast";
}

void rscp_header_session(t_rscp_connection* msg, char* s)
{
    msgprintf(msg, "Session: %s\r\n", s);
}

void rscp_event(t_rscp_connection* msg, uint32_t event)
{
    msgprintf(msg, "Event: %u\r\n", event);
}

void rscp_header_transport(t_rscp_connection* msg, t_rscp_transport* t)
{
    msgprintf(msg, "Transport: RTP/AVP;%s;", rscp_str_multicast(t->multicast));
    if (t->port) msgprintf(msg, "port=%d-%d;", PORT_RANGE_START(t->port), PORT_RANGE_STOP(t->port));
    if (t->client_port) msgprintf(msg, "client_port=%d-%d;", ntohs(PORT_RANGE_START(t->client_port)), ntohs(PORT_RANGE_STOP(t->client_port)));
    if (t->server_port) msgprintf(msg, "server_port=%d-%d;", ntohs(PORT_RANGE_START(t->server_port)), ntohs(PORT_RANGE_STOP(t->server_port)));
    msgprintf(msg, "mode=\"PLAY\"\r\n");
}

void rscp_header_edid(t_rscp_connection* msg, t_rscp_edid *edid)
{
    msgprintf(msg, "EDID-Segment: %02x", edid->segment);
    for (int i=0;i<256;i++) {
        msgprintf(msg, "%02x", edid->edid[i]);
    }
    msgprintf(msg, "\r\n");
}

void rscp_header_timing(t_rscp_connection* msg, t_video_timing* timing)
{
    // Timing: pfreq width fp p bp pol height fp p bp pol
    msgprintf(msg, "Timing: %d %d %d %d %d %s %d %d %d %d %s\r\n",
            timing->pfreq, timing->width, timing->hfront, timing->hpulse, timing->hback,
            (timing->hpolarity ? "P" : "N"),
            timing->height, timing->vfront, timing->vpulse, timing->vback,
            (timing->vpolarity ? "P" : "N"));
}

void rscp_header_rtp_format(t_rscp_connection* msg, t_rscp_rtp_format* p)
{
    // RTP-Format: compress[ value] rtptime
    if (p->compress == 16) {
        msgprintf(msg, "RTP-Format: aud16Bit %d %d %u\r\n", p->value, p->value2, p->rtptime);
    } else if (p->compress == 24) {
        msgprintf(msg, "RTP-Format: aud24Bit %d %d %u\r\n", p->value, p->value2, p->rtptime);
    } else if (p->compress == 32) {
        msgprintf(msg, "RTP-Format: aud32Bit %d %d %u\r\n", p->value, p->value2, p->rtptime);
    } else if (p->compress == FORMAT_JPEG2000) {
        msgprintf(msg, "RTP-Format: jp2k %d %u\r\n", p->value, p->rtptime);
    } else {
        msgprintf(msg, "RTP-Format: plain %u\r\n", p->rtptime);
    }

}

void rscp_request_setup(t_rscp_connection* msg, char* uri, t_rscp_transport* transport, t_rscp_edid *edid)
{
    rscp_request_line(msg, "SETUP", uri);
    rscp_header_transport(msg, transport);
    if (edid) rscp_header_edid(msg, edid);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_response_setup(t_rscp_connection* msg, t_rscp_transport* transport, char* session)
{
    rscp_response_line(msg, RSCP_SC_OK, "OK");
    rscp_header_session(msg, session);
    rscp_header_transport(msg, transport);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_request_play(t_rscp_connection* msg, char* uri, char* session, t_rscp_rtp_format* fmt)
{
    rscp_request_line(msg, "PLAY", uri);
    rscp_header_session(msg, session);
    rscp_header_rtp_format(msg, fmt);
    rscp_eoh(msg);
    rscp_send(msg);

}

void rscp_request_teardown(t_rscp_connection* msg, char* uri, char* session)
{
    rscp_request_line(msg, "TEARDOWN", uri);
    rscp_header_session(msg, session);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_request_update(t_rscp_connection* msg, char* uri, char* session, uint32_t event)
{
    rscp_request_line(msg, "UPDATE", uri);
    rscp_header_session(msg, session);
    rscp_event(msg, event);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_request_pause(t_rscp_connection* msg, char* uri, char* session)
{
    rscp_request_line(msg, "PAUSE", uri);
    rscp_header_session(msg, session);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_request_hello(t_rscp_connection* msg, char* uri)
{
    rscp_request_line(msg, "HELLO", uri);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_response_play(t_rscp_connection* msg, char* session, t_rscp_rtp_format* fmt, t_video_timing* timing)
{
    rscp_response_line(msg, RSCP_SC_OK, "OK");
    rscp_header_session(msg, session);
    rscp_header_rtp_format(msg, fmt);
    if (timing) rscp_header_timing(msg, timing);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_response_teardown(t_rscp_connection* msg, char* session)
{
    rscp_response_line(msg, RSCP_SC_OK, "OK");
    rscp_header_session(msg, session);
    rscp_eoh(msg);
    rscp_send(msg);
}
