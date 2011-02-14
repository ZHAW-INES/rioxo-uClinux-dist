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
#include "rscp_string.h"
#include "vrb_video.h"

struct {
    uint32_t address;
} box = {
    .address = 0
};

int box_sys_hello(t_rscp_media UNUSED *media, intptr_t UNUSED m, t_rscp_connection* rsp)
{
    if ((rsp->address == box.address) && box.address) {
        unlock("box_sys_hello");
            hdoipd_launch(hdoipd_start_vrb, 0, 50, 3, 1000);
        lock("box_sys_hello");
    }
    return 0;
}

void box_sys_set_remote(char* address)
{
    char buf[200];
    t_str_uri uri;
    struct hostent* host;

    box.address = 0;

    strcpy(buf, address);

    if (!str_split_uri(&uri, buf)) {
        report("uri error: scheme://server[:port]/name");
        return;
    }

    host = gethostbyname(uri.server);

    if (!host) {
        report("gethostbyname failed");
        return;
    }

    box.address = *((uint32_t*)host->h_addr_list[0]);
}

t_rscp_media box_sys = {
    .name = "",
    .owner = 0,
    .cookie = 0,
    .hello = (frscpm*)box_sys_hello
};
