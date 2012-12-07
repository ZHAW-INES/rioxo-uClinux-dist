/*
 * rtsp_connection.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_CONNECTION_H_
#define RTSP_CONNECTION_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "rtsp_string.h"

#define RTSP_MSG_MAX_LENGTH         (4096)
#define RTSP_CON_DOC_RX             1
#define RTSP_CON_DOC_TX             2

typedef struct {
    uint8_t         buf[RTSP_MSG_MAX_LENGTH];
    uint8_t*        sol;
    uint8_t*        eol;
} t_rtsp_buffer;

static inline void rtsp_buffer_init(t_rtsp_buffer *buf)
{
    buf->sol = buf->eol = buf->buf;
}

typedef struct {
    char        line[100];
    t_str_request_line rq;
    t_str_uri   uri;
    char        session[20];
    char        content_type[50];
    size_t      content_length;
} t_rtsp_header_common;

typedef struct rtsp_connection {
    int             fdw, fdr;       // TCP socket of this connection (read(RX) and write(TX))
    int             sequence;       // Count of sent messages
    t_rtsp_buffer   in;             // Message buffer
    t_rtsp_buffer   out;            // Message buffer
    uint32_t        address;        // Remote IP
    int             ecode;          // Error code
    char*           ereason;        // Error reason
    int             doc;            // plot flags (RTSP_CON_DOC_RX, RTSP_CON_DOC_TX)
    t_rtsp_header_common common;
} t_rtsp_connection;

// TODO: send content if not enough size and retry snprintf?

/** Print message to buffer
 *
 * */

#define rtsp_buffer_printf(_m_, ...)                        \
do {                                                        \
    if ((_m_)->eol < ((_m_)->buf + RTSP_MSG_MAX_LENGTH)) {  \
        int n = snprintf((_m_)->eol,                        \
                         RTSP_MSG_MAX_LENGTH - ((_m_)->eol - (_m_)->buf), \
                         __VA_ARGS__);                      \
        (_m_)->eol += n;                                    \
    } \
} while (0)

#define msgprintf(c, ...)                   \
do {                                        \
    t_rtsp_connection * _c_ = (c);          \
    t_rtsp_buffer* _m_ = &((_c_)->out);     \
    rtsp_buffer_printf(_m_, __VA_ARGS__);   \
} while (0)

void rtsp_coninit(t_rtsp_connection* m, int fd, uint32_t addr);
int rtsp_receive(t_rtsp_connection* con, char** line, int timeout, size_t length, size_t* readBytes);
void rtsp_send(t_rtsp_connection* con);
void rtsp_write(t_rtsp_connection* con);
int rtsp_split(t_rtsp_connection* con, t_rtsp_connection* con1, t_rtsp_connection* con2);

#endif /* RTSP_CONNECTION_H_ */
