/**
 * rtsp_command.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 *
 *  RTSP response and request methods
 *  These methods collect the necessary parameters for an RTSP message
 *  an pack the parameters into a string. This string is sent to the
 *  server/client. Only the header of the RTSP protocol is used.
 */

#include "rtsp.h"
#include "rtsp_net.h"
#include "rtsp_command.h"
#include "hdoipd.h"

void rtsp_response_line(t_rtsp_connection* msg, int code, char* reason)
{
    msgprintf(msg, "%s %03d %s\r\n",
        RTSP_VERSION,
        code,
        reason);
    rtsp_request_cseq(msg, msg->sequence);
}

void rtsp_response_error(t_rtsp_connection* msg, int code, char* reason)
{
    if (reason == NULL)
    {
        switch (code)
        {
          case RTSP_STATUS_CONTINUE:
            reason = "Continue";
            break;
          case RTSP_STATUS_OK:
            reason = "OK";
            break;
          case RTSP_STATUS_CREATED:
            reason = "Created";
            break;
          case RTSP_STATUS_LOW_ON_STORAGE_SPACE:
            reason = "Low on Storage Space";
            break;
          case RTSP_STATUS_MULTIPLE_CHOICES:
            reason = "Multiple Choices";
            break;
          case RTSP_STATUS_MOVED_PERMANENTLY:
            reason = "Moved Permanently";
            break;
          case RTSP_STATUS_MOVED_TEMPORARILY:
            reason = "Moved Temporarily";
            break;
          case RTSP_STATUS_SEE_OTHER:
            reason = "See Other";
            break;
          case RTSP_STATUS_USE_PROXY:
            reason = "Use Proxy";
            break;
          case RTSP_STATUS_BAD_REQUEST:
            reason = "Bad Request";
            break;
          case RTSP_STATUS_UNAUTHORIZED:
            reason = "Unauthorized";
            break;
          case RTSP_STATUS_PAYMENT_REQUIRED:
            reason = "Payment Required";
            break;
          case RTSP_STATUS_FORBIDDEN:
            reason = "Forbidden";
            break;
          case RTSP_STATUS_NOT_FOUND:
            reason = "Not Found";
            break;
          case RTSP_STATUS_METHOD_NOT_ALLOWED:
            reason = "Method Not Allowed";
            break;
          case RTSP_STATUS_NOT_ACCEPTABLE:
            reason = "Not Acceptable";
            break;
          case RTSP_STATUS_PROXY_AUTHENTICATION_REQUIRED:
            reason = "Proxy Authentication Required";
            break;
          case RTSP_STATUS_REQUEST_TIMEOUT:
            reason = "Request Timeout";
            break;
          case RTSP_STATUS_GONE:
            reason = "Gone";
            break;
          case RTSP_STATUS_LENGTH_REQUIRED:
            reason = "Length Required";
            break;
          case RTSP_STATUS_PRECONDITION_FAILED:
            reason = "Precondition Failed";
            break;
          case RTSP_STATUS_REQUEST_ENTITY_TOO_LARGE:
            reason = "Request Entity Too Large";
            break;
          case RTSP_STATUS_REQUEST_URI_TOO_LONG:
            reason = "Request-URI Too Long";
            break;
          case RTSP_STATUS_UNSUPPORTED_MEDIA_TYPE:
            reason = "Unsupported Media Type";
            break;
          case RTSP_STATUS_INVALID_PARAMETER:
            reason = "Parameter Not Understood";
            break;
          case RTSP_STATUS_ILLEGAL_CONFERENCE_IDENTIFIER:
            reason = "Illegal Conference Identifier";
            break;
          case RTSP_STATUS_NOT_ENOUGH_BANDWIDTH:
            reason = "Not Enough Bandwidth";
            break;
          case RTSP_STATUS_SESSION_NOT_FOUND:
            reason = "Session Not Found";
            break;
          case RTSP_STATUS_METHOD_NOT_VALID_IN_THIS_STATE:
            reason = "Method Not Valid In This State";
            break;
          case RTSP_STATUS_HEADER_FIELD_NOT_VALID:
            reason = "Header Field Not Valid";
            break;
          case RTSP_STATUS_INVALID_RANGE:
            reason = "Invalid Range";
            break;
          case RTSP_STATUS_PARAMETER_IS_READ_ONLY:
            reason = "Parameter Is Read-Only";
            break;
          case RTSP_STATUS_AGGREGATE_OPERATION_NOT_ALLOWED:
            reason = "Aggregate Operation Not Allowed";
            break;
          case RTSP_STATUS_ONLY_AGGREGATE_OPERATION_ALLOWED:
            reason = "Only Aggregate Operation Allowed";
            break;
          case RTSP_STATUS_UNSUPPORTED_TRANSPORT:
            reason = "Unsupported Transport";
            break;
          case RTSP_STATUS_DESTINATION_UNREACHABLE:
            reason = "Destination Unreachable";
            break;
          case RTSP_STATUS_INTERNAL_SERVER_ERROR:
            reason = "Internal Server Error";
            break;
          case RTSP_STATUS_NOT_IMPLEMENTED:
            reason = "Not Implemented";
            break;
          case RTSP_STATUS_BAD_GATEWAY:
            reason = "Bad Gateway";
            break;
          case RTSP_STATUS_SERVICE_UNAVAILABLE:
            reason = "Service Unavailable";
            break;
          case RTSP_STATUS_GATEWAY_TIMEOUT:
            reason = "Gateway Timeout";
            break;
          case RTSP_STATUS_RTSP_VERSION_NOT_SUPPORTED:
            reason = "RTSP Version Not Supported";
            break;
          case RTSP_STATUS_OPTION_NOT_SUPPORTED:
            reason = "Option not support";
            break;
          case RTSP_STATUS_UNKNOWN:
          default:
            reason = "Unknown error";
            break;
        }
    }
  
    rtsp_response_line(msg, code, reason);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

/** First line of an RTSP request message. RTSP_VERSION is last parameter.
 *  In a response message, RTSP_VERSION is the first parameter
 * */
void rtsp_request_line(t_rtsp_connection* msg, char* method, char* uri, char* media)
{
    msgprintf(msg, "%s %s",
        method,
        uri);
    if (media != NULL)
      msgprintf(msg, "/%s", media);
    msgprintf(msg, " %s\r\n", RTSP_VERSION);
    rtsp_request_cseq(msg, msg->sequence);
}

void rtsp_request_cseq(t_rtsp_connection* msg, int s)
{
    msgprintf(msg, "CSeq: %d\r\n", s);
}
/** Terminate every message with "\r\n"
 *
 * */
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
    msgprintf(msg, "Session: %s", s);
    if (hdoipd.eth_timeout != RTSP_DEFAULT_TIMEOUT) {
        msgprintf(msg, ";timeout=%d", hdoipd.eth_timeout);
    }
    msgprintf(msg, "\r\n");
}

void rtsp_header_content_length(t_rtsp_connection* msg, size_t length)
{
    msgprintf(msg, "Content-Length: %zu\r\n", length);
}

void rtsp_event(t_rtsp_connection* msg, uint32_t event)
{
    msgprintf(msg, "Event: %u\r\n", event);
}

void rtsp_header_transport(t_rtsp_connection* msg, t_rtsp_transport* t)
{
    msgprintf(msg, "Transport: RTP/AVP;%s;", rtsp_str_multicast(t->multicast));
    if (t->port) {
        msgprintf(msg, "port=%d-%d;", PORT_RANGE_START(t->port), PORT_RANGE_STOP(t->port));
    }
    if (t->client_port) {
        msgprintf(msg, "client_port=%d-%d;", ntohs(PORT_RANGE_START(t->client_port)), ntohs(PORT_RANGE_STOP(t->client_port)));
    }
    if (t->server_port) {
        msgprintf(msg, "server_port=%d-%d;", ntohs(PORT_RANGE_START(t->server_port)), ntohs(PORT_RANGE_STOP(t->server_port)));
    }

    msgprintf(msg, "destination=");
    if (t->multicast) {
        msgprintf(msg, (char*)reg_get("multicast_group"));
    }
    else if (reg_test("system-dhcp", "true")) {
        msgprintf(msg, (char*)reg_get("system-hostname"));
    }
    else {
        msgprintf(msg, (char*)reg_get("system-ip"));
    }
    msgprintf(msg, ";");

    msgprintf(msg, "mode=\"PLAY\"\r\n");
}

void rtsp_header_transport_usb(t_rtsp_connection* msg, t_rtsp_transport* t)
{
    msgprintf(msg, "Transport: USB/RAW/TCP;%s;", rtsp_str_multicast(t->multicast));
    msgprintf(msg, "destination=%s;", t->destination_str);
    msgprintf(msg, "port=%d;",  ntohs(t->port));
    msgprintf(msg, "mode=\"PLAY\"\r\n");
}

void rtsp_header_edid(t_rtsp_connection* msg, t_rtsp_edid *edid)
{
    msgprintf(msg, "EDID-Segment: %02x", edid->segment);
    for (int i=0;i<256;i++) {
        msgprintf(msg, "%02x", edid->edid[i]);
    }
    msgprintf(msg, "\r\n");
}

void rtsp_header_hdcp(t_rtsp_connection* msg, t_rtsp_hdcp *hdcp)
{
    msgprintf(msg, "HDCP: HDCP-Status=%d", hdcp->hdcp_on);
    msgprintf(msg, "\r\n");
}

void rtsp_header_usb(t_rtsp_connection* msg, char* mouse, char* keyboard, char* storage)
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

void rtsp_header_timing(t_rtsp_connection* msg, t_video_timing* timing)
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

void rtsp_header_rtp_format(t_rtsp_connection* msg, t_rtsp_rtp_format* p)
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
void rtsp_request_setup(t_rtsp_connection* msg, char* uri, char* media, t_rtsp_transport* transport, t_rtsp_edid *edid, t_rtsp_hdcp *hdcp)
{
    rtsp_request_line(msg, "SETUP", uri, media);
    rtsp_header_transport(msg, transport);
    if (edid) rtsp_header_edid(msg, edid);
    rtsp_header_hdcp(msg, hdcp); //input hdcp info //if hdcp
    rtsp_eoh(msg);
    rtsp_send(msg);
}
/** Respond to the setup message. Send back the adapted values.
 * */
void rtsp_response_setup(t_rtsp_connection* msg, t_rtsp_transport* transport, char* session, char* media, t_rtsp_hdcp *hdcp)
{
    rtsp_response_line(msg, RTSP_STATUS_OK, "OK");
    rtsp_header_session(msg, session);
    rtsp_header_transport(msg, transport);
    rtsp_header_hdcp(msg, hdcp); //input hdcp info //if hdcp
    rtsp_eoh(msg);
    rtsp_send(msg);
}

/** Create a HDCP message for session key exchange
 * */
void rtsp_request_hdcp(t_rtsp_connection* msg, char* uri, char* session, char* id, char* content)
{
    rtsp_request_line(msg, "HDCP", uri, NULL);
    rtsp_header_session(msg, session);
    msgprintf(msg, "ID: %s\r\n", id);
    msgprintf(msg, "Content: %s\r\n", content);
    rtsp_eoh(msg);
    rtsp_send(msg);

}

void rtsp_request_play(t_rtsp_connection* msg, char* uri, char* session, t_rtsp_rtp_format* fmt)
{
    rtsp_request_line(msg, "PLAY", uri, NULL);
    rtsp_header_session(msg, session);
    rtsp_header_rtp_format(msg, fmt);
    rtsp_eoh(msg);
    rtsp_send(msg);

}

void rtsp_request_teardown(t_rtsp_connection* msg, char* uri, char* session)
{
    rtsp_request_line(msg, "TEARDOWN", uri, NULL);
    rtsp_header_session(msg, session);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_request_update(t_rtsp_connection* msg, char* uri, char* session, char* media, uint32_t event, t_rtsp_rtp_format* fmt)
{
    rtsp_request_line(msg, "UPDATE", uri, media);
    rtsp_header_session(msg, session);
    rtsp_event(msg, event);
    rtsp_header_rtp_format(msg, fmt);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_request_pause(t_rtsp_connection* msg, char* uri, char* session)
{
    rtsp_request_line(msg, "PAUSE", uri, NULL);
    rtsp_header_session(msg, session);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_response_pause(t_rtsp_connection* msg, char* session)
{
    rtsp_response_line(msg, RTSP_STATUS_OK, "OK");
    rtsp_header_session(msg, session);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_request_get_parameter(t_rtsp_connection* msg, char* uri, char* session)
{
    rtsp_request_line(msg, "GET_PARAMETER", uri, NULL);
    if (session != NULL)
        rtsp_header_session(msg, session);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_request_set_parameter(t_rtsp_connection* msg, char* uri, char* session, char* name, char* value)
{
    bool has_param = name != NULL && value != NULL && strlen(name) > 0;
    rtsp_request_line(msg, "SET_PARAMETER", uri, NULL);
    if (session != NULL)
        rtsp_header_session(msg, session);
    if (has_param)
        rtsp_header_content_length(msg, strlen(name) + strlen(value) + 1);
    rtsp_eoh(msg);

    if (has_param)
        msgprintf(msg, "%s:%s", name, value);

    rtsp_send(msg);
}

void rtsp_response_play(t_rtsp_connection* msg, char* session, t_rtsp_rtp_format* fmt, t_video_timing* timing)
{
    rtsp_response_line(msg, RTSP_STATUS_OK, "OK");
    rtsp_header_session(msg, session);
    rtsp_header_rtp_format(msg, fmt);
    if (timing) rtsp_header_timing(msg, timing);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_response_teardown(t_rtsp_connection* msg)
{
    rtsp_response_line(msg, RTSP_STATUS_OK, "OK");
    rtsp_eoh(msg);
    rtsp_send(msg);
}
/** Respond to HDCP message for session key exchange
 * */
void rtsp_response_hdcp(t_rtsp_connection* msg, char* session, char* id, char* content)
{
    rtsp_response_line(msg, RTSP_STATUS_OK, "OK");
    rtsp_header_session(msg, session);
    if (id) {
        msgprintf(msg, "ID: %s\r\n", id);
        msgprintf(msg, "Content: %s\r\n", content);
    }
    rtsp_eoh(msg);
    rtsp_send(msg);

}

void rtsp_request_usb_setup(t_rtsp_connection* msg, char* uri, t_rtsp_transport* transport)
{
    rtsp_request_line(msg, "SETUP", uri, "usb");
    rtsp_header_transport_usb(msg, transport);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_request_usb_play(t_rtsp_connection* msg, char* uri, char* session, char* mouse_msg, char* keyboard_msg, char* storage_msg)
{
    rtsp_request_line(msg, "PLAY", uri, NULL);
    rtsp_header_session(msg, session);
    rtsp_header_usb(msg, mouse_msg, keyboard_msg, storage_msg);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_request_usb_teardown(t_rtsp_connection* msg, char* uri, char* session)
{
    rtsp_request_line(msg, "TEARDOWN", uri, NULL);
    rtsp_header_session(msg, session);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_response_usb_setup(t_rtsp_connection* msg, t_rtsp_transport* transport, char* session)
{
    rtsp_response_line(msg, RTSP_STATUS_OK, "OK");
    rtsp_header_session(msg, session);
    rtsp_header_transport_usb(msg, transport);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_response_usb_play(t_rtsp_connection* msg, char* session)
{
    rtsp_response_line(msg, RTSP_STATUS_OK, "OK");
    rtsp_header_session(msg, session);
    rtsp_eoh(msg);
    rtsp_send(msg);
}

void rtsp_response_usb_teardown(t_rtsp_connection* msg)
{
    rtsp_response_line(msg, RTSP_STATUS_OK, "OK");
    rtsp_eoh(msg);
    rtsp_send(msg);
}
