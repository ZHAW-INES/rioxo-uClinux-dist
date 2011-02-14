/*
 * rscp_connection.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RSCP_CONNECTION_H_
#define RSCP_CONNECTION_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RSCP_MSG_MAX_LENGTH         (4096)
#define RSCP_CON_DOC_RX             1
#define RSCP_CON_DOC_TX             2

typedef struct {
    uint8_t         buf[RSCP_MSG_MAX_LENGTH];
    uint8_t*        sol;
    uint8_t*        eol;
} t_rscp_buffer;

typedef struct {
    int             fdw, fdr;
    int             sequence;
    t_rscp_buffer   in;
    t_rscp_buffer   out;
    uint32_t        address;
    int             ecode;
    char*           ereason;
    int             doc;
} t_rscp_connection;

// TODO: send content if not enough size and retry snprintf?
#define msgprintf(c, ...) \
{ \
    t_rscp_connection * _c_ = (c); \
    t_rscp_buffer* _m_ = &_c_->out; \
    if (_m_->eol < (_m_->buf + RSCP_MSG_MAX_LENGTH)) { \
        int n = snprintf(_m_->eol, RSCP_MSG_MAX_LENGTH - (_m_->eol - _m_->buf), __VA_ARGS__); \
        _m_->eol += n; \
    } \
}

int rscp_receive_body(t_rscp_connection* con, char* buf, size_t length);
int rscp_receive(t_rscp_connection* con, char** line);
void rscp_send(t_rscp_connection* con);
void rscp_coninit(t_rscp_connection* m, int fd, uint32_t addr);
void rscp_msginit(t_rscp_connection* m);
int rscp_split(t_rscp_connection* con, t_rscp_connection* mux, t_rscp_connection* con2);

#endif /* RSCP_CONNECTION_H_ */
