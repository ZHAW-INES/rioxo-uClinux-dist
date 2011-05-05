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

#include "hdoipd.h"
#include "hoi_drv_user.h"
#include "version.h"

int hoi_msg(void* param)
{
    int ret;
    if ((ret = ioctl(hdoipd.drv, HDOIP_IOCPARAM, param))) {
        perrno("hoi_msg(%d, %08x:%d Byte) failed = %d", hdoipd.drv, *(uint32_t*)param, ((uint32_t*)param)[1], ret);
    }
    return ret;
}


//------------------------------------------------------------------------------
// additional driver load/unload command

int hoi_drv_ldrv(uint32_t drivers)
{
    t_hoi_msg_ldrv msg;

    hoi_msg_ldrv_init(&msg);
    msg.drivers = drivers;

    return hoi_msg(&msg);
}

int hoi_drv_buf(void* ar, size_t arl, void* vr, size_t vrl, void* at, size_t atl, void* vt, size_t vtl)
{
    t_hoi_msg_buf msg;

    hoi_msg_buf_init(&msg);
    msg.aud_rx_buf = ar; msg.aud_rx_len = arl; memset(ar, 0, arl);
    msg.vid_rx_buf = vr; msg.vid_rx_len = vrl; memset(vr, 0, vrl);
    msg.aud_tx_buf = at; msg.aud_tx_len = atl; memset(at, 0, atl);
    msg.vid_tx_buf = vt; msg.vid_tx_len = vtl; memset(vt, 0, vtl);

    return hoi_msg(&msg);
}
//-----------------------------------------------------------------------------
// HDCP
//send hdcp keys to kernel
int hoi_drv_hdcp(uint32_t hdcp_keys[])
{
    t_hoi_msg_hdcp_init msg;

    hoi_msg_hdcp_init(&msg);
    msg.key0 = hdcp_keys[0];
    msg.key1 = hdcp_keys[1];
    msg.key2 = hdcp_keys[2];
    msg.key3 = hdcp_keys[3];
    msg.riv0 = hdcp_keys[4];
    msg.riv1 = hdcp_keys[5];

    return hoi_msg(&msg);
}

//get hdcp status
int hoi_drv_hdcpstat(uint32_t *eto_hdcp_audio,uint32_t *eto_hdcp_video,uint32_t *eti_hdcp_audio, uint32_t *eti_hdcp_video)
{
    int ret;
    t_hoi_msg_hdcpstat msg;

    hoi_msg_hdcpstat_init(&msg);
    ret = hoi_msg(&msg);
    *eto_hdcp_audio = msg.status_eto_audio;
    *eto_hdcp_video = msg.status_eto_video;
    *eti_hdcp_audio = msg.status_eti_audio;
    *eti_hdcp_video = msg.status_eti_video;

    return ret;
}
//-----------------------------------------------------------------------------
int hoi_drv_eti(uint32_t addr_dst, uint32_t addr_src, uint32_t vid, uint32_t aud)
{
    t_hoi_msg_eti msg;

    hoi_msg_eti_init(&msg);
    msg.ip_address_dst = addr_dst;
    msg.ip_address_src = addr_src;
    msg.udp_port_aud = aud;
    msg.udp_port_vid = vid;

    return hoi_msg(&msg);

}

int hoi_drv_info(t_video_timing* timing, uint32_t *advcnt)
{
    int ret;
    t_hoi_msg_info msg;

    hoi_msg_info_init(&msg);
    msg.advcnt = 0;
    if (advcnt) msg.advcnt = *advcnt;
    ret = hoi_msg(&msg);
    if (advcnt) *advcnt = msg.advcnt;
    memcpy(timing, &msg.timing, sizeof(t_video_timing));

    return ret;
}

int hoi_drv_info_all(t_hoi_msg_info** nfo)
{
    int ret;
    static t_hoi_msg_info msg;

    hoi_msg_info_init(&msg);
    msg.advcnt = 0;
    ret = hoi_msg(&msg);
    *nfo = &msg;

    return ret;
}

#define HOI_GET_STAT(T)                         \
    int hoi_drv_##T(t_hoi_msg_##T **stat)       \
    {                                           \
        int ret;                                \
        static t_hoi_msg_##T msg;               \
        hoi_msg_##T##_init(&msg);               \
        ret = hoi_msg(&msg);                    \
        if (stat) *stat = &msg;                 \
        return ret;                             \
    }

HOI_GET_STAT(vsostat);
HOI_GET_STAT(ethstat);
HOI_GET_STAT(viostat);
HOI_GET_STAT(asoreg);


//------------------------------------------------------------------------------
// setup/read video format for input/output

#define HOI_WRITE(T, L)                         \
    int hoi_drv_##T(uint32_t p)                 \
    {                                           \
        t_hoi_msg_param msg;                    \
        hoi_msg_write_init(&msg, L);            \
        msg.value = p;                          \
        return hoi_msg(&msg);                   \
    }

#define HOI_READ(T, L)                          \
    int hoi_drv_##T(uint32_t *p)                \
    {                                           \
        int ret;                                \
        t_hoi_msg_param msg;                    \
        hoi_msg_read_init(&msg, L);             \
        ret = hoi_msg(&msg);                    \
        *p = msg.value;                         \
        return ret;                             \
    }

HOI_WRITE(ifmt, HOI_MSG_IFMT);
HOI_WRITE(ofmt, HOI_MSG_OFMT);
HOI_WRITE(pfmt, HOI_MSG_PFMT);
HOI_WRITE(set_mtime, HOI_MSG_SETMTIME);
HOI_WRITE(set_stime, HOI_MSG_SETSTIME);
HOI_WRITE(timer, HOI_MSG_TIMER);
HOI_WRITE(bw, HOI_MSG_BW);
HOI_WRITE(reset, HOI_MSG_OFF);

HOI_READ(get_mtime, HOI_MSG_GETMTIME);
HOI_READ(get_stime, HOI_MSG_GETSTIME);
HOI_READ(get_syncdelay, HOI_MSG_SYNCDELAY);


//------------------------------------------------------------------------------
// capture/show image command

int hoi_drv_vsi(uint32_t compress, uint32_t encrypt, int bandwidth, hdoip_eth_params* eth, t_video_timing* timing, uint32_t* advcnt)
{
    int ret;
    t_hoi_msg_vsi msg;

    hoi_msg_vsi_init(&msg);
    msg.bandwidth = bandwidth;
    msg.compress = compress;
    msg.encrypt = encrypt;
    if (advcnt) msg.advcnt = *advcnt;
    memcpy(&msg.eth, eth, sizeof(struct hdoip_eth_params));
    ret = hoi_msg(&msg);
    memcpy(timing, &msg.timing, sizeof(t_video_timing));
    if (advcnt) *advcnt = msg.advcnt;

    return ret;
}

int hoi_drv_vso(uint32_t compress, uint32_t encrypt, t_video_timing* timing, uint32_t advcnt, uint32_t delay_ms)
{
    int ret;
    t_hoi_msg_vso msg;

    hoi_msg_vso_init(&msg);
    msg.compress = compress;
    msg.advcnt = advcnt;
    msg.delay_ms = delay_ms;
    msg.encrypt = encrypt;
    memcpy(&msg.timing, timing, sizeof(t_video_timing));
    ret = hoi_msg(&msg);

    return ret;
}


int hoi_drv_asi(uint32_t cfg, hdoip_eth_params* eth, uint32_t fs, uint32_t width, uint32_t cnt, uint8_t* sel)
{
    int ret;
    t_hoi_msg_asi msg;

    hoi_msg_asi_init(&msg);
    msg.cfg = cfg;
    memcpy(&msg.channel, sel, 16);
    msg.channel_cnt = cnt;
    msg.fs = fs;
    msg.width = width;
    memcpy(&msg.eth, eth, sizeof(hdoip_eth_params));
    ret = hoi_msg(&msg);

    return ret;
}


int hoi_drv_aso(uint32_t fs, uint32_t fs_tol, uint32_t width, uint32_t cnt, uint8_t* sel, uint32_t delay_ms, uint32_t cfg)
{
    int ret;
    t_hoi_msg_aso msg;

    hoi_msg_aso_init(&msg);
    msg.cfg = cfg;
    msg.fs_tol = fs_tol;
    memcpy(&msg.channel, sel, 16);
    msg.channel_cnt = cnt;
    msg.fs = fs;
    msg.width = width;
    msg.delay_ms = delay_ms;
    ret = hoi_msg(&msg);

    return ret;
}


int hoi_drv_capture(bool compress, void* buffer, size_t size, t_video_timing* timing, uint32_t* advcnt)
{
    int ret;
    t_hoi_msg_image msg;

    hoi_msg_capture_init(&msg);
    msg.compress = compress;
    msg.buffer = buffer;
    msg.size = size;
    ret = hoi_msg(&msg);
    memcpy(timing, &msg.timing, sizeof(t_video_timing));
    if (compress && advcnt) *advcnt = msg.advcnt;

    return ret;
}

int hoi_drv_show(bool compress, void* buffer, t_video_timing* timing, uint32_t advcnt)
{
    int ret;
    t_hoi_msg_image msg;

    hoi_msg_show_init(&msg);
    msg.compress = compress;
    msg.buffer = buffer;
    memcpy(&msg.timing, timing, sizeof(t_video_timing));
    msg.advcnt = advcnt;
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_debug(t_video_timing* timing)
{
    int ret;
    t_hoi_msg_image msg;

    hoi_msg_debug_init(&msg);
    memcpy(&msg.timing, timing, sizeof(t_video_timing));
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_getversion(t_hoic_getversion* cmd)
{
    int ret;
    t_hoi_msg_version msg;

    hoi_msg_getversion_init(&msg);
    ret = hoi_msg(&msg);
    cmd->fpga_date = msg.fpga_date;
    cmd->fpga_svn = msg.fpga_svn;
    cmd->sysid_date = msg.sysid_date;
    cmd->sysid_id = msg.sysid_id;
    cmd->sw_version = VERSION_SOFTWARE;

    return ret;
}

//------------------------------------------------------------------------------
// EDID I/O

int hoi_drv_rdedid(void* buffer)
{
    int ret;
    t_hoi_msg_edid msg;

    hoi_msg_rdedid_init(&msg);
    ret = hoi_msg(&msg);
    memcpy(buffer, msg.edid, 256);

    return ret;
}

int hoi_drv_wredid(void* buffer)
{
    int ret;
    t_hoi_msg_edid msg;

    hoi_msg_wredid_init(&msg);
    memcpy(msg.edid, buffer, 256);
    ret = hoi_msg(&msg);

    return ret;
}


//------------------------------------------------------------------------------
// TAG I/O

int hoi_drv_rdvidtag(void* buffer, bool* available)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_rdvidtag_init(&msg);
    ret = hoi_msg(&msg);
    memcpy(buffer, msg.tag, 256);
    *available = msg.available;

    return ret;
}

int hoi_drv_wrvidtag(void* buffer)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_wrvidtag_init(&msg);
    memcpy(msg.tag, buffer, 256);
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_rdaudtag(void* buffer, bool* available)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_rdaudtag_init(&msg);
    ret = hoi_msg(&msg);
    memcpy(buffer, msg.tag, 256);
    *available = msg.available;

    return ret;
}

int hoi_drv_wraudtag(void* buffer)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_wraudtag_init(&msg);
    ret = hoi_msg(&msg);

    return ret;
}

//------------------------------------------------------------------------------
// command

#define HOI_CMDSW(T)                            \
    int hoi_drv_##T()                           \
    {                                           \
        t_hoi_msg msg;                          \
        hoi_msg_##T##_init(&msg);               \
        return hoi_msg(&msg);                   \
    }

HOI_CMDSW(poll);
HOI_CMDSW(loop);
HOI_CMDSW(osdon);
HOI_CMDSW(osdoff);
HOI_CMDSW(hpdon);
HOI_CMDSW(hpdoff);
HOI_CMDSW(repair);

HOI_CMDSW(hdcp_viden);       //enable hdcp video encryption
HOI_CMDSW(hdcp_auden);       //enable hdcp audio encryption
HOI_CMDSW(hdcp_viddis);      //disable hdcp video encryption
HOI_CMDSW(hdcp_auddis);      //disable hdcp audio encryption
HOI_CMDSW(hdcp_adv9889dis);  //disable AD9889 hdcp encryption
HOI_CMDSW(hdcp_adv9889en);   //enable AD9889 hdcp encryption

