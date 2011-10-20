/*
 * rscp_parse_header.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 *
 *  Functions to parse the received message string back to the struct values
 *  Every line of the RSCP message has its own parsing function.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "rscp_parse_header.h"
#include "rscp_error.h"
#include "rscp_string.h"
#include "map.h"
#include "hoi_msg.h"
#include "hdoipd.h"

#define next(x, l) \
{ \
    x = str_next_token(&(l), ";%0"); \
    if (!x || !*x) return RSCP_PARSE_ERROR; \
}

#define nextsp(x, l) \
{ \
    x = str_next_token(&(l), " %0"); \
    if (!x || !*x) return RSCP_PARSE_ERROR; \
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

int rscp_parse_port(char* str, uint32_t* p)
{
    char* s2;

    if ((s2 = str_next_token(&str, "-"))) {
        *p = PORT_RANGE(htons(atoi(s2)), htons(atoi(str)));
    } else {
        *p = htons(atoi(str));
    }

    return RSCP_SUCCESS;
}

int rscp_parse_ip(char* str, uint32_t* p)
{
    char* s;
    uint32_t ip = 0;
    int i;    

    for (i=0;i<3;i++) {
        if (s = str_next_token(&str, "."))
            ip |= (uint32_t)(atoi(s) << (8*i));
    }
    ip |= (uint32_t)(atoi(str) << (8*3));

    *p = ip;

    return RSCP_SUCCESS;
}

int rscp_parse_timing(char* line, t_video_timing* timing)
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

    return RSCP_SUCCESS;
}

int rscp_parse_rtp_format(char* line, t_rscp_rtp_format* p)
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

    return RSCP_SUCCESS;
}

int rscp_parse_uint32(char* line, uint32_t* p)
{
    *p = (uint32_t)atol(line);
    return RSCP_SUCCESS;
}

int rscp_parse_transport(char* line, t_rscp_transport* p)
{
    char* token;

    // transport spec (for now ignore) -> TODO
    next(token, line);
    // "unicast" or "multicast"
    next(token, line);
    if (!strcmp(token, "unicast")) {
        p->multicast = false;
    } else if (!strcmp(token, "multicast")) {
        p->multicast = true;
    } else return RSCP_PARSE_ERROR;


    while (*(token = str_next_token(&line, ";%0"))) {

        // port
        if (str_starts_with(&token, "port=")) rscp_parse_port(token, &p->port);
        else if (str_starts_with(&token, "client_port=")) rscp_parse_port(token, &p->client_port);
        else if (str_starts_with(&token, "server_port=")) rscp_parse_port(token, &p->server_port);
        else if (str_starts_with(&token, "multicast_group=")) rscp_parse_ip(token, &p->multicast_group);
    }

    return RSCP_SUCCESS;
}


int rscp_parse_rtp_info(char UNUSED *line, t_rscp_rtp_info UNUSED *p)
{
    return RSCP_SUCCESS;
}


int rscp_parse_ui32(char* line, uint32_t* p)
{
    *p = atoi(line);
    return RSCP_SUCCESS;
}


int rscp_parse_str(char* line, char* p)
{
    memcpy(p, line, strlen(line)+1);
    return RSCP_SUCCESS;
}

int rscp_parse_edid(char* line, t_rscp_edid *edid)
{
    edid->segment = nextbyte(&line[0]);
    for (int i=0;i<256;i++) {
        edid->edid[i] = nextbyte(&line[2*i+2]);
    }
    return RSCP_SUCCESS;
}

int rscp_parse_hdcp(char* line, t_rscp_hdcp *hdcp)
{
	int a;
	char* token;

	next(token, line);
	a = str_hdcp(token);
	hdcp->hdcp_on = a;

    return RSCP_SUCCESS;
}

int rscp_ommit_header(t_rscp_connection* con, int timeout)
{
    int n = RSCP_SUCCESS;
    char *line;

    while (!(n = rscp_receive(con, &line, timeout)) ) {
        // RSCP Header ends with empty line
        if (!*line) break;
    }
    // ommit body

    return n;
}

int rscp_parse_header(t_rscp_connection* con, const t_map_fnc attr[], void* base, t_rscp_header_common* common, int timeout)
{
    int n;
    char *line, *attrstr;

    if (common) common->session[0] = 0;

    while (!(n = rscp_receive(con, &line, timeout))) {
        // RSCP Header ends with empty line
        if (!*line) break;

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
            n = map_call_fnc(attr, attrstr, line, base);
            if (n == RSCP_UNKNOWN_HEADER) {
                report(" ? unknown header: %s", attrstr);
            } else if (n) {
                report(" ? parse header error: %s: %s", attrstr, line);
            }
        } else {
            report(" ? defect header-line: %s", line);
        }
    }

    return n;
}

void rscp_common_default(t_rscp_header_common* common)
{
    common->content_length = 0;
    common->content_type[0] = 0;
    common->session[0] = 0;
}

int rscp_parse_response(t_rscp_connection* con, const t_map_fnc attr[], void* base, t_rscp_header_common* common, int timeout)
{
    t_str_response_line rsp;
    int n;
    char* line;

    rscp_common_default(common);

    // Response Header
    if ((n = rscp_receive(con, &line, timeout))) {
        if(n != RSCP_CLOSE) {
            report("rscp response receive header failed");
        } else {
            report("rscp response receive unexpected close");
        }
        return n;
    }

    //strcpy(common->line, line);
    if (!str_split_response_line(&rsp, line)) {
        report("rscp response split error (%s)", line);
        return RSCP_SERVER_ERROR;
    }

    if (strcmp(rsp.code, "200") != 0) {
        con->ecode = atoi(rsp.code);
        con->ereason = rsp.reason;
        report("rscp error (%s): %s", rsp.code, rsp.reason);
        rscp_ommit_header(con, timeout);
        return RSCP_RESPONSE_ERROR;
    }

    if (strcmp(rsp.version, RSCP_VERSION) != 0) {
        report("unsupported version: %s", rsp.version);
        rscp_ommit_header(con, timeout);
        return RSCP_VERSION_ERROR;
    }

    if(n = rscp_parse_header(con, attr, base, common, timeout)) {
        report("rscp response receive header fields failed");
    }

    return n;
}

int rscp_parse_request(t_rscp_connection* con, const t_map_set srv_method[], const t_map_set** method, void* req, t_rscp_header_common* common)
{
    char* line;
    int n;

    rscp_common_default(common);

    // receive request line
    if ((n = rscp_receive(con, &line, 0))) {
        return n;
    }
    strcpy(common->line, line);

    // Request: (METHOD URI VERSION)
    if (!str_split_request_line(&common->rq, common->line)) return RSCP_CLIENT_ERROR;
    if (strcmp(common->rq.version, RSCP_VERSION) != 0) return RSCP_VERSION_ERROR;

    // test uri
    if (!str_split_uri(&common->uri, common->rq.uri)) return RSCP_CLIENT_ERROR;
    if (strcmp(common->uri.scheme, RSCP_SCHEME) != 0) return RSCP_CLIENT_ERROR;

    // find method
    *method = map_find_set(srv_method, common->rq.method);

    if (!*method) {
        rscp_ommit_header(con, 0);
        rscp_response_error(con, 405, "Method not allowed");
        report(" ? unsupported method %s", common->rq.method);
        return RSCP_SUCCESS;
    }

    if ((n = rscp_parse_header(con, (*method)->rec, req, common, 0))) {
        report(" ? parse request-header error (%d)", n);
        return n;
    }

    return RSCP_SUCCESS;
}

