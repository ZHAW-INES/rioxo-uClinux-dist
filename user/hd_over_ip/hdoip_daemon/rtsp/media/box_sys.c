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

int box_sys_options(t_rtsp_media *media UNUSED, void *data, t_rtsp_connection *rsp)
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

int box_sys_get_parameter(t_rtsp_media *media, t_rtsp_req_get_parameter *data, t_rtsp_connection *rsp)
{
    // if the request contains a sessionid, copy it to the response
    if (data != NULL && rsp != NULL && strlen(data->session) > 0)
      strcpy(data->session, rsp->common.session);
    rtsp_handle_get_parameter(media, rsp);
    rtsp_send(rsp);

    return 0;
}

int box_sys_set_parameter(t_rtsp_media *media, void *data UNUSED, t_rtsp_connection *rsp)
{
    rtsp_handle_set_parameter(media, rsp);
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
