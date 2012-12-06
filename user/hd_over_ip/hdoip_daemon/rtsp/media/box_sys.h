/*
 * box_sys.h
 *
 *  Created on: 14.12.2010
 *      Author: alda
 */

#ifndef BOX_SYS_H_
#define BOX_SYS_H_

#include "debug.h"
#include "rtsp_connection.h"
#include "rtsp_media.h"

int box_sys_hello(t_rtsp_media *media UNUSED, void *data UNUSED, t_rtsp_connection *rsp UNUSED);
int box_sys_options(t_rtsp_media *media UNUSED, void *data, t_rtsp_connection *rsp);
int box_sys_get_parameter(t_rtsp_media *media, void *data, t_rtsp_connection *rsp);

int box_sys_set_remote(char* address);

#define REPORT_RTX(dir, s, dir2, d, __av)                   \
{                                                           \
    struct in_addr a1, a2;                                  \
    a1.s_addr = s.address;                                  \
    a2.s_addr = d.address;                                  \
    reportn(INFO dir ": %s:%d %s " dir2,                    \
            inet_ntoa(a1),                                  \
            ntohs(s. __av##_port),                          \
            mac_ntoa(s.mac));                               \
    report(" %s:%d %s",                                     \
            inet_ntoa(a2),                                  \
            ntohs(d. __av##_port),                          \
            mac_ntoa(d.mac));                               \
}

#endif /* BOX_SYS_H_ */
