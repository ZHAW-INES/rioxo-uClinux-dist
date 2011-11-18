/*
 * rscp_connection.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 *
 *  Functions to send/receive data
 */

#include "rscp.h"
#include "rscp_net.h"
#include "rscp_error.h"
#include "rscp_string.h"
#include "rscp_connection.h"

#include "hdoipd.h"


void rscp_coninit(t_rscp_connection* m, int fd, uint32_t addr)
{
    m->fdw = fd;
    m->fdr = fd;
    m->sequence = 1;
    m->in.eol = m->in.sol = m->in.buf;
    m->out.eol = m->out.sol = m->out.buf;
    m->in.buf[0] = 0;
    m->out.buf[0] = 0;
    m->address = addr;
    m->doc = RSCP_CON_DOC_RX | RSCP_CON_DOC_TX;
}


bool rscp_receive_crlf(char** s, char* eol)
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
/** Not yet used, only header of protocol is used
 * */
int rscp_receive_body(t_rscp_connection* con, char* buf, size_t length)
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
        if ((i = recv(con->fdr, buf + n, length - n, 0)) <= 0) return i;
        n += i;
        buf[n] = 0;
    }

    return length;
}

/** Read the received chars in the socket
 *
 * */
int rscp_receive(t_rscp_connection* con, char** line, int timeout)
{
    int ret = 0;
    size_t s;
    char* sol = con->in.sol;
    char* eol = sol;
    fd_set rfds;
    struct timeval tv;

    *line = 0;

    while (!rscp_receive_crlf(&eol, con->in.eol)) {
        s = (eol - sol);
        memmove(con->in.buf, sol, s);
        sol = con->in.buf;
        con->in.sol = sol;
        eol = sol + s;
        if (timeout) {
            FD_ZERO(&rfds);
            FD_SET(con->fdr, &rfds);
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            ret = select(con->fdr+1, &rfds, NULL, NULL, &tv);
            if (ret == -1) {
                report(ERROR "rscp_receive() select failed");
                return -1;
            } else if (ret == 0) {
                report(ERROR "rscp_receive() timeout");
                return RSCP_TIMEOUT;
            }
        }

        // read is blocking -> so thread can be cancelled at this position
        pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
        ret = read(con->fdr, eol, RSCP_MSG_MAX_LENGTH - 1 - s);
        pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);

        if (ret == 0) {
            return RSCP_CLOSE;
        } else if (ret == -1) {
            return -1;
        }
        con->in.eol = eol + ret;
        if (eol > sol) eol--;
        *(eol + ret) = 0;
    }

    con->in.sol = eol;

    *line = sol;

#ifdef REPORT_RSCP_PACKETS
    if (con->doc & RSCP_CON_DOC_RX) {
        struct in_addr addr = { .s_addr = con->address };
        report_rscp("[%15s] < %s", inet_ntoa(addr), *line);
    }
#endif

    return RSCP_SUCCESS;
}

/** Sends message to receiver, function equal to rscp_write
 *
 * */
void rscp_send(t_rscp_connection* con)
{
    if (send(con->fdw, con->out.buf, con->out.eol - con->out.buf, MSG_NOSIGNAL) == -1) {
    //if (write(con->fdw, con->out.buf, con->out.eol - con->out.buf) == -1) {
        perrno("rscp_send() send failed");
    }

#ifdef REPORT_RSCP_PACKETS
    if (con->doc & RSCP_CON_DOC_TX) {
        struct in_addr addr = { .s_addr = con->address };
        char* sol = con->out.buf;
        while (*sol) {
            report_rscp("[%15s] > %s", inet_ntoa(addr), str_next_token(&sol, "%:\r\n;%0"));
        }
    }
#endif

    con->sequence++;
    con->out.eol = con->out.sol = con->out.buf;
    con->out.buf[0] = 0;
}

/** Sends message to receiver, function equal to rscp_send
 *
 * */
void rscp_write(t_rscp_connection* con)
{
    //if (send(con->fdw, con->out.buf, con->out.eol - con->out.buf, 0) == -1) {
    if (write(con->fdw, con->out.buf, con->out.eol - con->out.buf) == -1) {
        perrno("rscp_send() write failed");
    }

#ifdef REPORT_RSCP_PACKETS
    if (con->doc & RSCP_CON_DOC_TX) {
        struct in_addr addr = { .s_addr = con->address };
        char* sol = con->out.buf;
        while (*sol) {
            report_rscp("[%15s] > %s", inet_ntoa(addr), str_next_token(&sol, "%:\r\n;%0"));
        }
    }
#endif
    con->sequence++;  //increment sequence nr.
    con->out.eol = con->out.sol = con->out.buf;
    con->out.buf[0] = 0;
}

/** Creates the two pipes to write in the responses/requests ??
 *
 * */
int rscp_split(t_rscp_connection* con, t_rscp_connection* con1, t_rscp_connection* con2)
{
    int pfd1[2], pfd2[2]; // 0=read, 1=write

    if (pipe(pfd1) == -1) return -1;
    if (pipe(pfd2) == -1) return -1;

    rscp_coninit(con1, 0, con->address);
    con1->doc = 0;
    con1->fdr = con->fdr;
    con1->fdw = pfd1[1];
    con->fdr  = pfd1[0];

    rscp_coninit(con2, 0, con->address);
    con2->doc = RSCP_CON_DOC_RX;
    con2->fdw = pfd2[1];
    con2->fdr = pfd2[0];

    return RSCP_SUCCESS;
}

