/**
 * rscp_command.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 *
 *  RSCP response and request methods
 *  These methods collect the necessary parameters for an RSCP message
 *  an pack the parameters into a string. This string is sent to the
 *  server/client. Only the header of the RSCP protocol is used.
 */

#include "rscp.h"
#include "rscp_net.h"
#include "rscp_command.h"
#include "hdoipd.h"

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

/** First line of an RSCP request message. RSCP_VERSION is last parameter.
 *  In a response message, RSCP_VERSION is the first parameter
 * */
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
/** Terminate every message with "\r\n"
 *
 * */
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
    msgprintf(msg, "multicast_group=%s;", reg_get("multicast_group"));
    msgprintf(msg, "mode=\"PLAY\"\r\n");
}

void rscp_header_transport_usb(t_rscp_connection* msg, t_rscp_transport* t)
{
    msgprintf(msg, "Transport: TCP;%s;", rscp_str_multicast(t->multicast));
    msgprintf(msg, "usb-host-ip=%i.%i.%i.%i;", ((t->usb_host_ip & 0x000000FF) >> 0), ((t->usb_host_ip & 0x0000FF00) >> 8), ((t->usb_host_ip & 0x00FF0000) >> 16),((t->usb_host_ip & 0xFF000000) >> 24));
    msgprintf(msg, "usb-host-port=%d;",  ntohs(t->usb_host_port));
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

void rscp_header_hdcp(t_rscp_connection* msg, t_rscp_hdcp *hdcp)
{
    msgprintf(msg, "HDCP: HDCP-Status=%d", hdcp->hdcp_on);
    msgprintf(msg, "\r\n");
}

void rscp_header_usb(t_rscp_connection* msg, char* mouse, char* keyboard, char* storage)
{
    msgprintf(msg, "USB: ");
    if (strcmp(mouse, "")) {
        msgprintf(msg, "device-mouse=%s", mouse);      
        if ((strcmp(keyboard, "")) || (strcmp(storage, ""))) {
            msgprintf(msg, ";");
        }
    }
    if (strcmp(keyboard, "")) {
        msgprintf(msg, "device-keyboard=%s", keyboard);
        if (strcmp(storage, "")) {
            msgprintf(msg, ";");
        }
    }
    if (strcmp(storage, "")) {
        msgprintf(msg, "device-storage=%s", storage);
    }
    msgprintf(msg, "\r\n");
}

void rscp_header_timing(t_rscp_connection* msg, t_video_timing* timing)
{
    // Timing:   Horizontal       : pfreq width fp p bp pol 
    //           Vertical Field 0 : height fp p bp pol
    //           Vertical Field 1 : eight fp p bp pol interlaced     
    msgprintf(msg, "Timing: %d %d %d %d %d %s %d %d %d %d %s %d %d %d %d %s %s \r\n",
            timing->pfreq, timing->width, timing->hfront, timing->hpulse, timing->hback,
            (timing->hpolarity ? "P" : "N"),
            timing->height, timing->vfront, timing->vpulse, timing->vback,
            (timing->vpolarity ? "P" : "N"),
            timing->height_1, timing->vfront_1, timing->vpulse_1, timing->vback_1,
            (timing->fpolarity ? "P" : "N"),
            (timing->interlaced ? "I" : "P"));
}

void rscp_header_rtp_format(t_rscp_connection* msg, t_rscp_rtp_format* p)
{
    // RTP-Format: compress[ value] rtptime
    if (p->compress == 16) {
        msgprintf(msg, "RTP-Format: aud16Bit %d %d %u\r\n", p->value, p->value2, p->rtptime);
    } else if (p->compress == 20) {
        msgprintf(msg, "RTP-Format: aud20Bit %d %d %u\r\n", p->value, p->value2, p->rtptime);
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
/** Create message to initiate setup
 * */
void rscp_request_setup(t_rscp_connection* msg, char* uri, t_rscp_transport* transport, t_rscp_edid *edid, t_rscp_hdcp *hdcp)
{
    rscp_request_line(msg, "SETUP", uri);
    rscp_header_transport(msg, transport);
    if (edid) rscp_header_edid(msg, edid);
    rscp_header_hdcp(msg, hdcp); //input hdcp info //if hdcp
    rscp_eoh(msg);
    rscp_send(msg);
}
/** Respond to the setup message. Send back the adapted values.
 * */
void rscp_response_setup(t_rscp_connection* msg, t_rscp_transport* transport, char* session, t_rscp_hdcp *hdcp)
{
    rscp_response_line(msg, RSCP_SC_OK, "OK");
    rscp_header_session(msg, session);
    rscp_header_transport(msg, transport);
    rscp_header_hdcp(msg, hdcp); //input hdcp info //if hdcp
    rscp_eoh(msg);
    rscp_send(msg);
}

/** Create a HDCP message for session key exchange
 * */
void rscp_request_hdcp(t_rscp_connection* msg, char* session, char* uri, char* id, char* content)
{
    rscp_request_line(msg, "HDCP", uri);
    rscp_header_session(msg, session);
    msgprintf(msg, "ID: %s\r\n", id);
    msgprintf(msg, "Content: %s\r\n", content);
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

void rscp_request_update(t_rscp_connection* msg, char* uri, char* session, uint32_t event, t_rscp_rtp_format* fmt)
{
    rscp_request_line(msg, "UPDATE", uri);
    rscp_header_session(msg, session);
    rscp_event(msg, event);
    rscp_header_rtp_format(msg, fmt);
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

void rscp_response_pause(t_rscp_connection* msg, char* session)
{
    rscp_response_line(msg, RSCP_SC_OK, "OK");
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
/** Respond to HDCP message for session key exchange
 * */
void rscp_response_hdcp(t_rscp_connection* msg, char* session, char* id, char* content)
{
	rscp_response_line(msg, RSCP_SC_OK, "OK");
    rscp_header_session(msg, session);
    if (id) {
        msgprintf(msg, "ID: %s\r\n", id);
        msgprintf(msg, "Content: %s\r\n", content);
    }
    rscp_eoh(msg);
    rscp_send(msg);

}

void rscp_request_usb_setup(t_rscp_connection* msg, char* uri, t_rscp_transport* transport, char* session)
{
    rscp_request_line(msg, "SETUP", uri);
    rscp_header_transport_usb(msg, transport);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_request_usb_play(t_rscp_connection* msg, char* uri, char* session, char* mouse_msg, char* keyboard_msg, char* storage_msg)
{
    rscp_request_line(msg, "PLAY", uri);
    rscp_header_session(msg, session);
    rscp_header_usb(msg, mouse_msg, keyboard_msg, storage_msg);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_request_usb_teardown(t_rscp_connection* msg, char* uri, char* session)
{
    rscp_request_line(msg, "TEARDOWN", uri);
    rscp_header_session(msg, session);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_response_usb_setup(t_rscp_connection* msg, t_rscp_transport* transport, char* session)
{
    rscp_response_line(msg, RSCP_SC_OK, "OK");
    rscp_header_session(msg, session);
    rscp_header_transport_usb(msg, transport);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_response_usb_play(t_rscp_connection* msg, char* session)
{
    rscp_response_line(msg, RSCP_SC_OK, "OK");
    rscp_header_session(msg, session);
    rscp_eoh(msg);
    rscp_send(msg);
}

void rscp_response_usb_teardown(t_rscp_connection* msg, char* session)
{
    rscp_response_line(msg, RSCP_SC_OK, "OK");
    rscp_header_session(msg, session);
    rscp_eoh(msg);
    rscp_send(msg);
}
