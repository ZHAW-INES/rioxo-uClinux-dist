/*
 * box_sys.c
 *
 *  Created on: 14.12.2010
 *      Author: alda
 */
#include <stdint.h>
#include "hoi_drv_user.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "box_sys.h"
#include "rtsp_string.h"
#include "vrb_video.h"

struct {
    uint32_t address;
} box = {
    .address = 0
};

int box_sys_hello(t_rtsp_media UNUSED *media, intptr_t UNUSED m, t_rtsp_connection* rsp)
{
    if ((rsp->address == box.address) && box.address) {
        report(INFO "hello received from remote device");

        // new connection = new hdcp-key exchange
        hdoipd.hdcp.ske_executed = 0;

        if(hdoipd.auto_stream) {
            hdoipd_set_task_start_vrb();
        }
    }
    return 0;
}

int box_sys_options(t_rtsp_media UNUSED *media, t_map_set* methods, t_rtsp_connection* rsp)
{
    int index = 0;
    rtsp_response_line(rsp, RTSP_STATUS_OK, "OK");
    msgprintf(rsp, "Public: ");
    while (methods->name != NULL)
    {
        if (index > 0) msgprintf(rsp, ", ");
        msgprintf(rsp, methods->name);
        methods++;
        index++;
    }
    rtsp_eoh(rsp); // for the "Public: " line
    rtsp_eoh(rsp); // to end the header
    rtsp_send(rsp);
  
    return 0;
}

int box_sys_set_remote(char* address)
{
    char buf[200];
    t_str_uri uri;
    struct hostent* host;

    if (!address) return -1;

    box.address = 0;

    strcpy(buf, address);

    if (!str_split_uri(&uri, buf)) {
        report("uri error: scheme://server[:port]/name");
        return -1;
    }

    host = gethostbyname(uri.server);

    if (!host) {
        //herror("gethostbyname");
        return -1;
    }

    box.address = *((uint32_t*)host->h_addr_list[0]);

    struct in_addr addr; addr.s_addr = box.address;
    report(INFO "box_sys_set_remote(%s)", inet_ntoa(addr));

    return 0;
}

t_rtsp_media box_sys = {
    .name = "",
    .owner = 0,
    .cookie = 0,
    .hello = (frtspm*)box_sys_hello,
    .options = (frtspm*)box_sys_options
};
