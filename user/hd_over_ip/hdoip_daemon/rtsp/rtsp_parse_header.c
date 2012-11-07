/*
 * rtsp_parse_header.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 *
 *  Functions to parse the received message string back to the struct values
 *  Every line of the RTSP message has its own parsing function.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "rtsp_parse_header.h"
#include "rtsp_error.h"
#include "rtsp_string.h"
#include "map.h"
#include "hoi_msg.h"
#include "hdoipd.h"

#define next(x, l) \
{ \
    x = str_next_token(&(l), ";%0"); \
    if (!x || !*x) return RTSP_PARSE_ERROR; \
}

#define nextsp(x, l) \
{ \
    x = str_next_token(&(l), " %0"); \
    if (!x || !*x) return RTSP_PARSE_ERROR; \
}

static inline uint8_t nextbyte(char* s)
{
    uint8_t r = 0;
    if ((s[0] >= '0') && (s[0] <= '9')) r = r + (s[0] - '0');
    else if ((s[0] >= 'a') && (s[0] <= 'f')) r = r + (s[0] - 'a' + 10);
    else if ((s[0] >= 'A') && (s[0] <= 'F')) r = r + (s[0] - 'A' + 10);
    r = r << 4;
    if ((s[1] >= '0') && (s[1] <= '9')) r = r + (s[1] - '0');
    else if ((s[1] >= 'a') && (s[1] <= 'f')) r = r + (s[1] - 'a' + 10);
    else if ((s[1] >= 'A') && (s[1] <= 'F')) r = r + (s[1] - 'A' + 10);
    return r;
}

int rtsp_parse_port(char* str, uint32_t* p)
{
    char* s2;

    if ((s2 = str_next_token(&str, "-"))) {
        *p = PORT_RANGE(htons(atoi(s2)), htons(atoi(str)));
    } else {
        *p = htons(atoi(str));
    }

    return RTSP_SUCCESS;
}

int rtsp_parse_ip(char* str, uint32_t* p)
{
    char* s;
    uint32_t ip = 0;
    int i;    

    for (i=0;i<3;i++) {
        if ((s = str_next_token(&str, ".")))
            ip |= (uint32_t)(atoi(s) << (8*i));
    }
    ip |= (uint32_t)(atoi(str) << (8*3));

    *p = ip;

    return RTSP_SUCCESS;
}

int rtsp_parse_timing(char* line, t_video_timing* timing)
{
    char* token;
    // Timing:   Horizontal       : pfreq width fp p bp pol 
    //           Vertical Field 0 : height fp p bp pol
    //           Vertical Field 1 : height fp p bp pol interlaced    

    nextsp(token, line);
    timing->pfreq = atoi(token);

    nextsp(token, line);
    timing->width = atoi(token);
    nextsp(token, line);
    timing->hfront = atoi(token);
    nextsp(token, line);
    timing->hpulse = atoi(token);
    nextsp(token, line);
    timing->hback = atoi(token);
    nextsp(token, line);
    if (strcmp(token, "N") == 0) timing->hpolarity = 0;
    else timing->hpolarity = 1;

    nextsp(token, line);
    timing->height = atoi(token);
    nextsp(token, line);
    timing->vfront = atoi(token);
    nextsp(token, line);
    timing->vpulse = atoi(token);
    nextsp(token, line);
    timing->vback = atoi(token);
    nextsp(token, line);
    if (strcmp(token, "N") == 0) timing->vpolarity = 0;
    else timing->vpolarity = 1;

    nextsp(token, line);
    timing->height_1 = atoi(token);
    nextsp(token, line);
    timing->vfront_1 = atoi(token);
    nextsp(token, line);
    timing->vpulse_1 = atoi(token);
    nextsp(token, line);
    timing->vback_1 = atoi(token);
    nextsp(token, line);
    if (strcmp(token, "N") == 0) timing->fpolarity = 0;
    else timing->fpolarity = 1;
    nextsp(token, line);
    if (strcmp(token, "P") == 0) timing->interlaced = 0;
    else timing->interlaced = 1;

    return RTSP_SUCCESS;
}

int rtsp_parse_rtp_format(char* line, t_rtsp_rtp_format* p)
{
    char* token;
    // RTP-Format: compress[ value] rtptime

    nextsp(token, line);

    if (strcmp(token, "plain") == 0) {
        p->compress = FORMAT_PLAIN;
    } else if (strcmp(token, "aud16Bit") == 0) {
        p->compress = 16;
        nextsp(token, line);
        p->value = atoi(token);
        nextsp(token, line);
        p->value2 = atoi(token);
    } else if (strcmp(token, "aud20Bit") == 0) {
        p->compress = 20;
        nextsp(token, line);
        p->value = atoi(token);
        nextsp(token, line);
        p->value2 = atoi(token);
    } else if (strcmp(token, "aud24Bit") == 0) {
        p->compress = 24;
        nextsp(token, line);
        p->value = atoi(token);
        nextsp(token, line);
        p->value2 = atoi(token);
    } else if (strcmp(token, "aud32Bit") == 0) {
        p->compress = 32;
        nextsp(token, line);
        p->value = atoi(token);
        nextsp(token, line);
        p->value2 = atoi(token);
    } else if (strcmp(token, "jp2k") == 0) {
        p->compress = FORMAT_JPEG2000;
        nextsp(token, line);
        p->value = atoi(token);
    }

    nextsp(token, line);

    p->rtptime = (uint32_t)atoll(token);

    return RTSP_SUCCESS;
}

int rtsp_parse_uint32(char* line, uint32_t* p)
{
    *p = (uint32_t)atol(line);
    return RTSP_SUCCESS;
}

int rtsp_parse_transport(char* line, t_rtsp_transport* p)
{
    char* token;
    int ret;

    // transport spec
    next(token, line);
    if ((ret = rtsp_parse_transport_protocol(token, p)) != RTSP_SUCCESS) {
        report(" ? unknown transport protocol: %s", token);
        return ret;
    }
    // "unicast" or "multicast"
    next(token, line);
    if (!strcmp(token, "unicast")) {
        p->multicast = false;
    } else if (!strcmp(token, "multicast")) {
        p->multicast = true;
    } else return RTSP_PARSE_ERROR;


    while (*(token = str_next_token(&line, ";%0"))) {

        // port
        if (str_starts_with(&token, "port=")) rtsp_parse_port(token, &p->port);
        else if (str_starts_with(&token, "client_port=")) rtsp_parse_port(token, &p->client_port);
        else if (str_starts_with(&token, "server_port=")) rtsp_parse_port(token, &p->server_port);
        else if (str_starts_with(&token, "destination=")) {
          rtsp_parse_ip(token, &p->destination);
          strncpy(p->destination_str, token, 49);
        }
    }

    return RTSP_SUCCESS;
}


int rtsp_parse_transport_protocol(char *line, t_rtsp_transport* p)
{
    char *pos = NULL;
    char *token = line;
    size_t length = 0;

    if (line == NULL)
        return RTSP_PARSE_ERROR;
  
    pos = strchrnul(line, '/');
    if (pos == NULL || *pos == '\0')
        return RTSP_PARSE_ERROR;
    
    length = pos - line;
    if (strncmp(token, "RTP", length) == 0) {
        p->protocol = PROTOCOL_RTP;
        p->profile = PROFILE_AVP;
        p->lower = LOWER_UDP;
    }
    else if (strncmp(token, "USB", length) == 0) {
        p->protocol = PROTOCOL_USB;
        p->profile = PROFILE_RAW;
        p->lower = LOWER_TCP;
    }
    else {
        report(" ? unknown transport protocol");
        return RTSP_PARSE_ERROR;
    }

    token = pos + 1;
    pos = strchrnul(token, '/');
    if (pos == NULL)
        return RTSP_PARSE_ERROR;

    length = pos - token;
    if (strncmp(token, "AVP", length) == 0)
        p->protocol = PROFILE_AVP;
    else if (strncmp(token, "RAW", length) == 0)
        p->profile = PROFILE_RAW;
    else {
        report(" ? unknown transport profile");
        return RTSP_PARSE_ERROR;
    }

    // the lower protocol part is optional
    if (*pos == '\0')
        return RTSP_SUCCESS;

    token = pos + 1;
    pos = strchrnul(token, '/');
    // this must be the last part of the protocol definition
    if (pos == NULL || *pos != '\0')
        return RTSP_PARSE_ERROR;

    length = pos - token;
    if (strncmp(token, "UDP", length) == 0)
        p->lower = LOWER_UDP;
    else if (strncmp(token, "TCP", length) == 0)
        p->lower = LOWER_TCP;
    else {
        report(" ? unknown transport lower");
        return RTSP_PARSE_ERROR;
    }

    return RTSP_SUCCESS;
}


int rtsp_parse_rtp_info(char UNUSED *line, t_rtsp_rtp_info UNUSED *p)
{
    return RTSP_SUCCESS;
}


int rtsp_parse_ui32(char* line, uint32_t* p)
{
    *p = atoi(line);
    return RTSP_SUCCESS;
}


int rtsp_parse_str(char* line, char* p)
{
    memcpy(p, line, strlen(line)+1);
    return RTSP_SUCCESS;
}

int rtsp_parse_edid(char* line, t_rtsp_edid *edid)
{
    edid->segment = nextbyte(&line[0]);
    for (int i=0;i<256;i++) {
        edid->edid[i] = nextbyte(&line[2*i+2]);
    }
    return RTSP_SUCCESS;
}

int rtsp_parse_hdcp(char* line, t_rtsp_hdcp *hdcp)
{
	int a;
	char* token;

	next(token, line);
	a = str_hdcp(token);
	hdcp->hdcp_on = a;

    return RTSP_SUCCESS;
}

int rtsp_parse_usb(char* line, t_rtsp_usb *usb)
{
    char* token;

    strcpy(usb->mouse, "");
    strcpy(usb->keyboard, "");
    strcpy(usb->storage, "");

    while (*(token = str_next_token(&line, ";%0"))) {

        if      (str_starts_with(&token, "device-mouse="))      strcpy(usb->mouse,    token);
        else if (str_starts_with(&token, "device-keyboard="))   strcpy(usb->keyboard, token);
        else if (str_starts_with(&token, "device-storage="))    strcpy(usb->storage,  token);
    }

    return RTSP_SUCCESS;
}

int rtsp_ommit_message(t_rtsp_connection* con, int timeout)
{
  t_rtsp_header_common header;
  int n = RTSP_SUCCESS;
  
  n = rtsp_parse_header(con, NULL, NULL, &header, timeout);
  if (header.content_length > 0)
      n = rtsp_ommit_body(con, timeout, header.content_length);
  
  return n;
}

int rtsp_ommit_header(t_rtsp_connection* con, int timeout)
{
    int n = RTSP_SUCCESS;
    char *line;
    size_t read;

    while ((n = rtsp_receive(con, &line, timeout, 0, &read)) == RTSP_SUCCESS) {
        // RTSP Header ends with empty line
        if (*line == '\0') break;
    }

    return n;
}

int rtsp_ommit_body(t_rtsp_connection* con, int timeout, size_t length)
{
    int n = RTSP_SUCCESS;
    char *buf;
    size_t read;

    if (length <= 0)
        return RTSP_SUCCESS;
    
    return rtsp_receive(con, &buf, timeout, length, &read);
}

int rtsp_parse_header(t_rtsp_connection* con, const t_map_fnc attr[], void* base, t_rtsp_header_common* common, int timeout)
{
    int n;
    char *line, *attrstr;
    size_t read;

    if (common) {
        common->session[0] = 0;
        common->content_type[0] = 0;
        common->content_length = 0;
    }
    
    while ((n = rtsp_receive(con, &line, timeout, 0, &read)) == RTSP_SUCCESS) {
        // RTSP Header ends with empty line
        if (*line == '\0') break;

        if ((attrstr = str_next_token(&line, "%:: ;"))) {
            if (common) {
                if (strcmp(attrstr, "Session") == 0) {
                    strcpy(common->session, line);
                } else if (strcmp(attrstr, "Content-Type") == 0) {
                    strcpy(common->content_type, line);
                } else if (strcmp(attrstr, "Content-Length") == 0) {
                    common->content_length = atoi(line);
                }
            }
            if (base && attr)
            {
              n = map_call_fnc(attr, attrstr, line, base);
              if (n == RTSP_UNKNOWN_HEADER) {
                  report(" ? unknown header: %s", attrstr);
              } else if (n) {
                  report(" ? parse header error: %s: %s", attrstr, line);
              }
            }
        } else {
            report(" ? defect header-line: %s", line);
        }
    }

    return n;
}

void rtsp_common_default(t_rtsp_header_common* common)
{
    common->content_length = 0;
    common->content_type[0] = 0;
    common->session[0] = 0;
}

int rtsp_parse_response(t_rtsp_connection* con, const t_map_fnc attr[], void* base, t_rtsp_header_common* common, int timeout)
{
    t_str_response_line rsp;
    int n;
    char* line;
    size_t read;

    rtsp_common_default(common);
    
    // Response Header
    if ((n = rtsp_receive(con, &line, timeout, 0, &read)) != RTSP_SUCCESS) {
        if(n != RTSP_CLOSE) {
            report("rtsp response receive header failed");
        } else {
            report("rtsp response receive unexpected close");
        }
        return n;
    }

    //strcpy(common->line, line);
    if (!str_split_response_line(&rsp, line)) {
        report("rtsp response split error (%s)", line);
        return RTSP_SERVER_ERROR;
    }

    if (strcmp(rsp.code, "200") != 0) {
        con->ecode = atoi(rsp.code);
        con->ereason = rsp.reason;
        report("rtsp error (%s): %s", rsp.code, rsp.reason);
        rtsp_ommit_message(con, timeout);
        return RTSP_RESPONSE_ERROR;
    }

    if (strcmp(rsp.version, RTSP_VERSION) != 0) {
        report("unsupported version: %s", rsp.version);
        rtsp_ommit_message(con, timeout);
        return RTSP_VERSION_ERROR;
    }

    if((n = rtsp_parse_header(con, attr, base, common, timeout))) {
        report("rtsp response receive header fields failed");
    }

    return n;
}

int rtsp_parse_request(t_rtsp_connection* con, const t_map_set srv_method[], const t_map_set** method, void* req, t_rtsp_header_common* common)
{
    char* line;
    int n;
    size_t read;

    rtsp_common_default(common);
    
    // receive request line
    if ((n = rtsp_receive(con, &line, 0, 0, &read)) != RTSP_SUCCESS) {
        return n;
    }
    strcpy(common->line, line);

    // Request: (METHOD URI VERSION)
    if (!str_split_request_line(&common->rq, common->line)) {
        rtsp_ommit_message(con, 0);
        rtsp_response_error(con, RTSP_STATUS_BAD_REQUEST, "Bad Request Line");
        report(" ? unknown request line (%s)", common->line);
        return RTSP_HANDLED;
    }
    if (strcmp(common->rq.version, RTSP_VERSION) != 0) {
        rtsp_ommit_message(con, 0);
        rtsp_response_error(con, RTSP_STATUS_RTSP_VERSION_NOT_SUPPORTED, "Version not supported");
        report(" ? mismatching RTSP version (%s)", common->rq.version);
        return RTSP_HANDLED;
    }

    // test uri
    if (!str_split_uri(&common->uri, common->rq.uri)) {
        rtsp_ommit_message(con, 0);
        rtsp_response_error(con, RTSP_STATUS_NOT_FOUND, NULL);
        report(" ? unknown URI (%s)", common->rq.uri);
        return RTSP_HANDLED;
    }
    // the URI's schema must either be "rtsp://" or
    // "*" but then uri.server must be empty
    if (strcmp(common->uri.scheme, RTSP_SCHEME) != 0 &&
       (strcmp(common->uri.scheme, "*") != 0 || common->uri.server)) {
        rtsp_ommit_message(con, 0);
        rtsp_response_error(con, RTSP_STATUS_BAD_REQUEST, "Bad URI Schema");
        report(" ? mismatching URI schema (%s)", common->uri.scheme);
        return RTSP_HANDLED;
    }

    // find method
    *method = map_find_set(srv_method, common->rq.method);

    if (!*method) {
        rtsp_ommit_message(con, 0);
        rtsp_response_error(con, RTSP_STATUS_NOT_IMPLEMENTED, NULL);
        report(" ? unsupported method (%s)", common->rq.method);
        return RTSP_HANDLED;
    }

    if ((n = rtsp_parse_header(con, (*method)->rec, req, common, 0))) {
        rtsp_ommit_body(con, 0, common->content_length);
        rtsp_response_error(con, RTSP_STATUS_BAD_REQUEST, "Bad Request Header");
        report(" ? parse request-header error (%d)", n);
        return RTSP_HANDLED;
    }
    
#ifdef REPORT_RTSP
    report("rtsp request of method %s to URI %s (media \"%s\"; control \"%s\")",
          common->rq.method, common->rq.uri, common->uri.name, common->uri.control != NULL ? common->uri.control : "0");
#endif

    return RTSP_SUCCESS;
}

