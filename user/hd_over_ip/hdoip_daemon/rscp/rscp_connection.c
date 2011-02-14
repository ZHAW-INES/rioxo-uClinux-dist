/*
 * rscp_connection.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "rscp.h"
#include "rscp_net.h"
#include "rscp_error.h"
#include "rscp_string.h"
#include "rscp_connection.h"

#include "hdoipd.h"

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

int rscp_receive(t_rscp_connection* con, char** line)
{
    int n = 0;
    char* sol = con->in.sol;
    char* eol = sol;

    *line = 0;

    while (!rscp_receive_crlf(&eol, con->in.eol)) {
        n = (eol - sol);
        memmove(con->in.buf, sol, n);
        sol = con->in.buf;
        con->in.sol = sol;
        eol = sol + n;
        //if ((n = recv(con->fdr, eol, RSCP_MSG_MAX_LENGTH - 1 - n, 0)) <= 0) return -1;
        if ((n = read(con->fdr, eol, RSCP_MSG_MAX_LENGTH - 1 - n)) <= 0) return -1;
        con->in.eol = eol + n;
        if (eol > sol) eol--;
        *(eol + n) = 0;
    }

    con->in.sol = eol;

    *line = sol;

    if (con->doc & RSCP_CON_DOC_RX) {
        struct in_addr addr = { .s_addr = con->address };
        fprintf(rscp_fd, "[%15s] < %s\n", inet_ntoa(addr), *line);
        fflush(rscp_fd);
    }

    return RSCP_SUCCESS;
}

void rscp_send(t_rscp_connection* con)
{
    //send(con->fdw, con->out.buf, con->out.eol - con->out.buf, 0);
    if (write(con->fdw, con->out.buf, con->out.eol - con->out.buf) == -1) {
        perrno("rscp_send() write failed");
    }

    if (con->doc & RSCP_CON_DOC_TX) {
        struct in_addr addr = { .s_addr = con->address };
        char* sol = con->out.buf;
        while (*sol) {
            fprintf(rscp_fd, "[%15s] > %s\n", inet_ntoa(addr), str_next_token(&sol, "%:\r\n;%0"));
        }
        fflush(rscp_fd);
    }

    con->out.eol = con->out.buf;
}

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

void rscp_msginit(t_rscp_connection* m)
{
    m->sequence++;
    m->out.eol = m->out.sol = m->out.buf;
    m->out.buf[0] = 0;
}

int rscp_split(t_rscp_connection* con, t_rscp_connection* mux, t_rscp_connection* con2)
{
    int pfd[2], pfd2[2]; // 0=read, 1=write

    if (pipe(pfd) == -1) return -1;
    rscp_coninit(mux, 0, con->address);
    mux->doc = 0;
    mux->fdr = con->fdr;
    mux->fdw = pfd[1];
    con->fdr = pfd[0];

    if (pipe(pfd2) == -1) return -1;
    rscp_coninit(con2, 0, con->address);
    con2->doc = RSCP_CON_DOC_RX;
    con2->fdw = pfd2[1];
    con2->fdr = pfd2[0];

    return RSCP_SUCCESS;
}

