/*
 * rtsp_connection.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "rtsp.h"
#include "rtsp_error.h"
#include "rtsp_connection.h"
#include "rtsp_net.h"

bool rtsp_receive_crlf(char** s, char* eol)
{
    char* p = *s;
    for (;(p<(eol-1)) && !((p[0]=='\r')&&(p[1]=='\n'));p++);
    if (p<(eol-1)) {
        p[0] = 0;
        p[1] = 0;
        *s = p + 2;
        return true;
    }
    *s = eol;
    return false;
}

int rtsp_receive_body(t_rtsp_connection* con, char* buf, size_t length)
{
    int i;
    char* sol = con->in.sol;
    char* eol = con->in.eol;
    size_t n = eol - sol;

    if (n > length) {
        // buffer not empty
        n = length;
        con->in.sol += n;
    } else {
        // buffer now empty
        con->in.sol = con->in.buf;
        con->in.eol = con->in.buf;
    }

    memcpy(buf, sol, n);

    while (n < length) {
        if ((i = recv(con->fd, buf + n, length - n, 0)) <= 0) return i;
        n += i;
        buf[n] = 0;
    }

    return length;
}

int rtsp_receive(t_rtsp_connection* con, char** line)
{
    int n = 0;
    char* sol = con->in.sol;
    char* eol = sol;

    while (!rtsp_receive_crlf(&eol, con->in.eol)) {
        n = (eol - sol);
        memmove(con->in.buf, sol, n);
        sol = con->in.buf;
        con->in.sol = sol;
        eol = sol + n;
        if ((n = recv(con->fd, eol, RTSP_MSG_MAX_LENGTH - 1 - n, 0)) <= 0) return RTSP_SHUTDOWN;
        con->in.eol = eol + n;
        if (eol > sol) eol--;
        *(eol + n) = 0;
    }

    con->in.sol = eol;

    *line = sol;

    struct in_addr addr = { .s_addr = con->address };
    printf("[%15s] < %s\n", inet_ntoa(addr), *line);

    return RTSP_SUCCESS;
}

void rtsp_send(t_rtsp_connection* con)
{
    send(con->fd, con->out.buf, con->out.eol - con->out.buf, 0);

    struct in_addr addr = { .s_addr = con->address };
    char* sol = con->out.buf;
    while (*sol) {
        printf("[%15s] > %s\n", inet_ntoa(addr), str_next_token(&sol, "%:\r\n;%0"));
    }

    con->out.eol = con->out.buf;
}

void rtsp_coninit(t_rtsp_connection* m, int fd, uint32_t addr)
{
    m->fd = fd;
    m->sequence = 1;
    m->in.eol = m->in.sol = m->in.buf;
    m->out.eol = m->out.sol = m->out.buf;
    m->in.buf[0] = 0;
    m->out.buf[0] = 0;
    m->address = addr;
}

void rtsp_msginit(t_rtsp_connection* m)
{
    m->sequence++;
    m->out.eol = m->out.sol = m->out.buf;
    m->out.buf[0] = 0;
}
