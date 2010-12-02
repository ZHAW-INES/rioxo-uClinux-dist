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

#define RTSP_MSG_MAX_LENGTH         (2048)

typedef struct {
    uint8_t         buf[RTSP_MSG_MAX_LENGTH];
    uint8_t*        sol;
    uint8_t*        eol;
} t_rtsp_buffer;

typedef struct {
    int             fd;
    int             sequence;
    t_rtsp_buffer   in;
    t_rtsp_buffer   out;
    uint32_t        address;
    int             ecode;
    char*           ereason;
} t_rtsp_connection;

// TODO: send content if not enough size and retry snprintf?
#define msgprintf(c, ...) \
{ \
    t_rtsp_connection * _c_ = (c); \
    t_rtsp_buffer* _m_ = &_c_->out; \
    if (_m_->eol < (_m_->buf + RTSP_MSG_MAX_LENGTH)) { \
        int n = snprintf(_m_->eol, RTSP_MSG_MAX_LENGTH - (_m_->eol - _m_->buf), __VA_ARGS__); \
        _m_->eol += n; \
    } \
}

int rtsp_receive_body(t_rtsp_connection* con, char* buf, size_t length);
int rtsp_receive(t_rtsp_connection* con, char** line);
void rtsp_send(t_rtsp_connection* con);
void rtsp_coninit(t_rtsp_connection* m, int fd, uint32_t addr);
void rtsp_msginit(t_rtsp_connection* m);

#endif /* RTSP_CONNECTION_H_ */
