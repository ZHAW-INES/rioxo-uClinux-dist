/*
 * rtsp_connection.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 *
 *  Functions to send/receive data
 */

#include "rtsp.h"
#include "rtsp_net.h"
#include "rtsp_error.h"
#include "rtsp_string.h"
#include "rtsp_connection.h"

#include "hdoipd.h"


void rtsp_coninit(t_rtsp_connection* m, int fd, uint32_t addr)
{
    m->fdw = fd;
    m->fdr = fd;
    m->sequence = 1;
    m->in.eol = m->in.sol = m->in.buf;
    m->out.eol = m->out.sol = m->out.buf;
    m->in.buf[0] = 0;
    m->out.buf[0] = 0;
    m->address = addr;
    m->doc = RTSP_CON_DOC_RX | RTSP_CON_DOC_TX;
}


static bool rtsp_receive_crlf(char **s, char *eol, size_t len, size_t *readBytes)
{
    char *p = *s;
    size_t n = 0;

    /* shortcut */
    if (*s == eol)
        return false;
    if (readBytes == NULL)
        return false;

    *readBytes = 0;

    if (len == 0) {
        /* Read as much as we can until \n or eol */
        for ( ; p < eol && *p != '\n'; p++)
            n++;
    } else {
        /* Read of fixed size (DOES ANYBODY EVER USE THIS?) */
        for ( ; n < len && p < eol; p++)
            n++;
    }

    *readBytes = n;

    if (len == 0 && *p == '\n') {
        /* omit \n and (optionally) \r */
        *p = '\0';
        p--;
        if (p >= *s && *p == '\r')
            *p = '\0';
        *s = p + 2;
        return true;
    } else {
        *s = p;
        return (p <= eol && n >= len);
    }

    *s = eol;
    return false;
}

/** Read the received chars in the socket
 *
 * */
int rtsp_receive(t_rtsp_connection* con, char** line, int timeout, size_t length, size_t* readBytes)
{
    int ret = 0;
    size_t s, readLength;
    char* sol = con->in.sol;
    char* eol = sol;
    fd_set rfds;
    struct timeval tv;

    *line = NULL;

    if (readBytes == NULL)
        return false;

    while (!rtsp_receive_crlf(&eol, con->in.eol, length, readBytes)) {
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
                report(ERROR "rtsp_receive() select failed");
                return -1;
            } else if (ret == 0) {
                report(ERROR "rtsp_receive() timeout");
                return RTSP_TIMEOUT;
            }
        }

        readLength = RTSP_MSG_MAX_LENGTH - 1 - s;
        if (length > 0 && length < readLength)
          readLength = length;

        // read is blocking -> so thread can be cancelled at this position
        pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
        ret = read(con->fdr, eol, readLength);
        pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);

        if (ret == 0) {
            return RTSP_CLOSE;
        } else if (ret == -1) {
            return -1;
        }
        con->in.eol = eol + ret;
        if (eol > sol) eol--;
        *(eol + ret) = 0;

        if (length > 0 && *readBytes <= length)
            length -= *readBytes;
    }

    con->in.sol = eol;

    *line = sol;

#ifdef REPORT_RTSP_PACKETS
    if (con->doc & RTSP_CON_DOC_RX) {
        struct in_addr addr = { .s_addr = con->address };
        report_rtsp("[%15s] < %s (%zu)", inet_ntoa(addr), *line, *readBytes);
    }
#endif

    return RTSP_SUCCESS;
}

/** Sends message to receiver, function equal to rtsp_write
 *
 * */
void rtsp_send(t_rtsp_connection* con)
{
    if (send(con->fdw, con->out.buf, con->out.eol - con->out.buf, MSG_NOSIGNAL) == -1) {
    //if (write(con->fdw, con->out.buf, con->out.eol - con->out.buf) == -1) {
        perrno("rtsp_send() send failed");
    }

#ifdef REPORT_RTSP_PACKETS
    if (con->doc & RTSP_CON_DOC_TX) {
        struct in_addr addr = { .s_addr = con->address };
        char* sol = con->out.buf;
        while (*sol) {
            report_rtsp("[%15s] > %s", inet_ntoa(addr), str_next_token(&sol, "%:\r\n;%0"));
        }
    }
#endif

    con->sequence++;
    con->out.eol = con->out.sol = con->out.buf;
    con->out.buf[0] = 0;
}

/** Sends message to receiver, function equal to rtsp_send
 *
 * */
void rtsp_write(t_rtsp_connection* con)
{
    //if (send(con->fdw, con->out.buf, con->out.eol - con->out.buf, 0) == -1) {
    if (write(con->fdw, con->out.buf, con->out.eol - con->out.buf) == -1) {
        perrno("rtsp_send() write failed");
    }

#ifdef REPORT_RTSP_PACKETS
    if (con->doc & RTSP_CON_DOC_TX) {
        struct in_addr addr = { .s_addr = con->address };
        char* sol = con->out.buf;
        while (*sol) {
            report_rtsp("[%15s] > %s", inet_ntoa(addr), str_next_token(&sol, "%:\r\n;%0"));
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
int rtsp_split(t_rtsp_connection* con, t_rtsp_connection* con1, t_rtsp_connection* con2)
{
    int pfd1[2], pfd2[2]; // 0=read, 1=write

    if (pipe(pfd1) == -1) return -1;
    if (pipe(pfd2) == -1) return -1;

    rtsp_coninit(con1, 0, con->address);
    con1->doc = 0;
    con1->fdr = con->fdr;
    con1->fdw = pfd1[1];
    con->fdr  = pfd1[0];

    rtsp_coninit(con2, 0, con->address);
    con2->doc = RTSP_CON_DOC_RX;
    con2->fdw = pfd2[1];
    con2->fdr = pfd2[0];

    return RTSP_SUCCESS;
}

