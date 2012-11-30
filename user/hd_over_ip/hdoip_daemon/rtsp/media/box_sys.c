/*
 * box_sys.c
 *
 *  Created on: 14.12.2010
 *      Author: alda
 */

#include <netdb.h>
#include <stdint.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "box_sys.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hoi_drv_user.h"
#include "rtsp_command.h"
#include "rtsp_error.h"
#include "rtsp_parameter.h"
#include "rtsp_string.h"
#include "vrb_video.h"

static struct {
    uint32_t address;
} box = {
    .address = 0
};

static int box_sys_hello(t_rtsp_media *media UNUSED, void *data UNUSED,
                         t_rtsp_connection *rsp UNUSED)
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

static int box_sys_options(t_rtsp_media *media UNUSED, void *data, t_rtsp_connection *rsp)
{
    t_map_set *methods = data;
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

static int box_sys_get_parameter(t_rtsp_media *media, void *data,
                                 t_rtsp_connection *rsp)
{
    char *line;
    int n;
    size_t rem, sz;
    int ret;

    rtsp_response_line(rsp, RTSP_STATUS_OK, "OK");
    // End of header
    rtsp_eoh(rsp);

    rem = rsp->common.content_length;
    sz = 0;

    while (rem > 0) {
        /* TODO: handle multiple lines properly in rtsp_receive */
        n = rtsp_receive(rsp, &line, 2, rem, &sz);
        if (n != RTSP_SUCCESS) {
            report(">>> failed to read body: %d", n);
            break;
        }

        rem -= sz;

#ifdef REPORT_RTSP_PACKETS
        report(">>> got line '%s' (%zu)", line, strlen(line));
#endif
        ret = rtsp_get_parameter(media, rsp, line);
        if (ret != RTSP_SUCCESS)
            report(ERROR "Failed to get parameter %s", line); // TODO: Anything else?
    }

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
    .name           = "",
    .owner          = 0,
    .cookie         = NULL,
    .hello          = box_sys_hello,
    .options        = box_sys_options,
    .get_parameter  = box_sys_get_parameter,
};
