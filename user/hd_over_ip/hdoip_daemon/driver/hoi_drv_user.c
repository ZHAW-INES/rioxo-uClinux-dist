/*
 * hoi_drv_user.c
 * This function do no range testing!
 *
 *  Created on: 18.10.2010
 *      Author: alda
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "hoi_drv_user.h"


inline int hoi_msg(int f, void* param)
{
    int ret;
    if ((ret = ioctl(f, HDOIP_IOCPARAM, param))) {
        printf("hoi_msg failed. (error code = %d)", ret);
    }
    return ret;
}


//------------------------------------------------------------------------------
// additional driver load/unload command

int hoi_drv_ldrv(int f, uint32_t drivers, pid_t pid)
{
    t_hoi_msg_ldrv msg;

    hoi_msg_ldrv_init(&msg);
    msg.drivers = drivers;
    msg.pid = pid;

    return hoi_msg(f, &msg);
}


//------------------------------------------------------------------------------
// setup/read video format for input/output


int hoi_drv_write(int f, uint32_t a, uint32_t v)
{
    t_hoi_msg_param msg;

    hoi_msg_write_init(&msg, a);
    msg.value = v;

    return hoi_msg(f, &msg);
}

int hoi_drv_read(int f, uint32_t a, uint32_t* v)
{
    int ret;
    t_hoi_msg_param msg;

    hoi_msg_read_init(&msg, a);
    ret = hoi_msg(f, &msg);
    *v = msg.value;

    return ret;
}

int hoi_drv_ifmt(int f, t_video_format fmt)
{
    return hoi_drv_write(f, HOI_MSG_IFMT, fmt);
}

int hoi_drv_ofmt(int f, t_video_format fmt)
{
    return hoi_drv_write(f, HOI_MSG_OFMT, fmt);
}

int hoi_drv_pfmt(int f, t_video_format fmt)
{
    return hoi_drv_write(f, HOI_MSG_PFMT, fmt);
}

int hoi_drv_getevent(int f, uint32_t* p)
{
    return hoi_drv_read(f, HOI_MSG_GETEVENT, p);
}

int hoi_drv_getstate(int f, uint32_t* p)
{
    return hoi_drv_read(f, HOI_MSG_GETSTATE, p);
}



//------------------------------------------------------------------------------
// capture/show image command

int hoi_drv_capture(int f, bool compress, void* buffer, size_t size, t_video_timing* timing, uint32_t* advcnt)
{
    int ret;
    t_hoi_msg_image msg;

    hoi_msg_capture_init(&msg);
    msg.compress = compress;
    msg.buffer = buffer;
    msg.size = size;
    ret = hoi_msg(f, &msg);
    memcpy(timing, &msg.timing, sizeof(t_video_timing));
    if (compress && advcnt) *advcnt = msg.advcnt;

    return ret;
}

int hoi_drv_show(int f, bool compress, void* buffer, t_video_timing* timing, uint32_t advcnt)
{
    int ret;
    t_hoi_msg_image msg;

    hoi_msg_show_init(&msg);
    msg.compress = compress;
    msg.buffer = buffer;
    memcpy(&msg.timing, timing, sizeof(t_video_timing));
    msg.advcnt = advcnt;
    ret = hoi_msg(f, &msg);

    return ret;
}


//------------------------------------------------------------------------------
// EDID I/O

int hoi_drv_rdedid(int f, void* buffer)
{
    int ret;
    t_hoi_msg_edid msg;

    hoi_msg_rdedid_init(&msg);
    ret = hoi_msg(f, &msg);
    memcpy(buffer, msg.edid, 256);

    return ret;
}

int hoi_drv_wredid(int f, void* buffer)
{
    int ret;
    t_hoi_msg_edid msg;

    hoi_msg_wredid_init(&msg);
    memcpy(msg.edid, buffer, 256);
    ret = hoi_msg(f, &msg);

    return ret;
}


//------------------------------------------------------------------------------
// TAG I/O

int hoi_drv_rdvidtag(int f, void* buffer, bool* available)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_rdvidtag_init(&msg);
    ret = hoi_msg(f, &msg);
    memcpy(buffer, msg.tag, 256);
    *available = msg.available;

    return ret;
}

int hoi_drv_wrvidtag(int f, void* buffer)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_wrvidtag_init(&msg);
    memcpy(msg.tag, buffer, 256);
    ret = hoi_msg(f, &msg);

    return ret;
}

int hoi_drv_rdaudtag(int f, void* buffer, bool* available)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_rdaudtag_init(&msg);
    ret = hoi_msg(f, &msg);
    memcpy(buffer, msg.tag, 256);
    *available = msg.available;

    return ret;
}

int hoi_drv_wraudtag(int f, void* buffer)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_wraudtag_init(&msg);
    memcpy(msg.tag, buffer, 256);
    ret = hoi_msg(f, &msg);

    return ret;
}


//------------------------------------------------------------------------------
// command

int hoi_drv_loop(int f)
{
    t_hoi_msg msg;
    hoi_msg_loop_init(&msg);
    return hoi_msg(f, &msg);
}

int hoi_drv_osdon(int f)
{
    t_hoi_msg msg;
    hoi_msg_osdon_init(&msg);
    return hoi_msg(f, &msg);
}

int hoi_drv_osdoff(int f)
{
    t_hoi_msg msg;
    hoi_msg_osdoff_init(&msg);
    return hoi_msg(f, &msg);
}

