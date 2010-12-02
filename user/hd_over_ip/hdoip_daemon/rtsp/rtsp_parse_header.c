/*
 * rtsp_parse_header.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "rtsp_parse_header.h"
#include "rtsp_error.h"
#include "rtsp_string.h"
#include "map.h"
#include "hoi_msg.h"

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

int rtsp_parse_port(char* str, uint32_t* p)
{
    char* s2;

    if ((s2 = str_next_token(&str, "-"))) {
        *p = PORT_RANGE(atoi(s2), atoi(str));
    } else {
        *p = atoi(str);
    }

    return RTSP_SUCCESS;
}

int rtsp_parse_timing(char* line, t_video_timing* timing)
{
    char* token;
    // Timing: pfreq width fp p bp pol height fp p bp pol

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
    timing->hfront = atoi(token);
    nextsp(token, line);
    timing->hpulse = atoi(token);
    nextsp(token, line);
    timing->hback = atoi(token);
    nextsp(token, line);
    if (strcmp(token, "N") == 0) timing->vpolarity = 0;
    else timing->vpolarity = 1;

    return RTSP_SUCCESS;
}

int rtsp_parse_rtp_format(char* line, t_rtsp_rtp_format* p)
{
    char* token;
    // RTP-Format: compress[ value] rtptime

    nextsp(token, line);

    if (strcmp(token, "plain") == 0) {
        p->compress = FORMAT_PLAIN;
    } else if (strcmp(token, "jp2k") == 0) {
        p->compress = FORMAT_JPEG2000;
        nextsp(token, line);
        p->value = atoi(token);
    }

    nextsp(token, line);
    p->rtptime = atoi(token);

    return RTSP_SUCCESS;
}

int rtsp_parse_transport(char* line, t_rtsp_transport* p)
{
    char* token;

    // transport spec (for now ignore) -> TODO
    next(token, line);

    // "unicast" or "multicast"
    next(token, line);
    if (strcmp(token, "unicast")) {
        p->multicast = false;
    } else if (strcmp(token, "multicast")) {
        p->multicast = true;
    } else return RTSP_PARSE_ERROR;


    while (*(token = str_next_token(&line, ";%0"))) {

        // port
        if (str_starts_with(&token, "port=")) rtsp_parse_port(token, &p->port);
        else if (str_starts_with(&token, "client_port=")) rtsp_parse_port(token, &p->client_port);
        else if (str_starts_with(&token, "server_port=")) rtsp_parse_port(token, &p->server_port);

    }
}


int rtsp_parse_rtp_info(char* line, t_rtsp_rtp_info* p)
{

}


int rtsp_parse_ui32(char* line, uint32_t* p)
{
    *p = atoi(line);
}


int rtsp_parse_str(char* line, char* p)
{
    memcpy(p, line, strlen(line)+1);
}


int rtsp_ommit_header(t_rtsp_connection* con)
{
    int n;
    char *line;

    while ((n = rtsp_receive(con, &line)) >= 0) {
        // RTSP Header ends with empty line
        if (!*line) break;
    }

    // ommit body

    return n;
}

int rtsp_parse_header(t_rtsp_connection* con, t_map_fnc* attr, void* base, char* session)
{
    int n;
    char *line, *attrstr;

    *session = 0;

    while ((n = rtsp_receive(con, &line)) >= 0) {
        // RTSP Header ends with empty line
        if (!*line) break;

        if ((attrstr = str_next_token(&line, "%:: ;"))) {
            if (strcmp(attrstr, "Session") == 0) {
                memcpy(session, line, strlen(line));
            }
            map_call_fnc(attr, attrstr, line, base);
        } else {
            printf(" ? defect line: %s\n", line);
        }
    }

    return n;
}


int rtsp_parse_response(t_rtsp_connection* con, t_map_fnc* attr, void* base, char* session)
{
    t_str_response_line rsp;
    int n;
    char* line;
    char sessionrx[50];

    // Response Header
    n = rtsp_receive(con, &line);
    if (n != RTSP_SUCCESS) return n;

    str_split_response_line(&rsp, line);

    if (strcmp(rsp.code, "200") != 0) {
        con->ecode = atoi(rsp.code);
        con->ereason = rsp.reason;
        printf("rtsp error (%s): %s\n", rsp.code, rsp.reason);
        rtsp_ommit_header(con);
        return RTSP_RESPONSE_ERROR;
    }

    if (strcmp(rsp.version, RTSP_VERSION) != 0) {
        printf("unsupported version: %s\n", rsp.version);
        rtsp_ommit_header(con);
        return RTSP_VERSION_ERROR;
    }

    *sessionrx = 0;

    n = rtsp_parse_header(con, attr, base, sessionrx);
    if (n != RTSP_SUCCESS) return n;

    if (sessionrx && session && *sessionrx && *session) {
        if (strcmp(session, sessionrx) != 0) {
            printf("wrong session: %s | own:%s\n", sessionrx, session);
            return RTSP_SERVER_ERROR;
        }
    }

    return RTSP_SUCCESS;
}

