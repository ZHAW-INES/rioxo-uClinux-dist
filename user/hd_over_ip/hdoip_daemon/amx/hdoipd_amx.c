/*
 * hdoipd_amx.c
 *
 *  Created on: Apr 26, 2011
 *      Author: stth
 */

#include <unistd.h>
#include <string.h>

#include "hdoipd_amx.h"
#include "debug.h"

int hdoipd_amx_open(t_hdoip_amx *handle, bool enable, int interval, uint32_t ip, uint16_t port)
{
    int broadcast = 1, ret = 0;

    handle->socket                          = 0;
    handle->enable                          = enable;
    handle->interval                        = interval;
    handle->interval_cnt                    = 0;
    handle->addr_in.sin_family              = AF_INET;
    handle->addr_in.sin_port                = port;
    handle->addr_in.sin_addr.s_addr         = ip;

    if(handle->enable) {
        handle->socket = socket(PF_INET, SOCK_DGRAM, 0);
        if(handle->socket) {
            ret = setsockopt(handle->socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
            if(ret == -1) {
                handle->socket = 0;
                return ret;
            }
            ret = connect(handle->socket, (struct sockaddr*)&(handle->addr_in), sizeof(struct sockaddr_in));
            if(ret == -1) {
                handle->socket = 0;
                return ret;
            }
        } else {
            return handle->socket;
        }
    }

    return ret;
}

int hdoipd_amx_close(t_hdoip_amx *handle)
{
    int ret = 0;

    if(handle->socket != 0) {
        ret = shutdown(handle->socket, SHUT_RDWR);
        close(handle->socket);
    }
    return ret;
}

int hdoipd_amx_update(t_hdoip_amx *handle, bool enable, int interval, uint32_t ip, uint16_t port)
{
    int ret = 0;

    if((ret = hdoipd_amx_close(handle)) == 0) {
        ret = hdoipd_amx_open(handle, enable, interval, ip, port);
    }

    return ret;
}

int hdoipd_amx_handler(t_hdoip_amx *handle, char *hello_msg)
{
    int ret = 0;
    if(handle->enable) {
        if(handle->interval_cnt == 0) {
            if(handle->socket) {
                ret = write(handle->socket, hello_msg, strlen(hello_msg));
                if(ret == -1) {
                    return ret;
                }
            }

            if(handle->interval > 1) {
                handle->interval_cnt = handle->interval - 1;
            } else {
                handle->interval_cnt = 0;
            }
        } else {
            handle->interval_cnt--;
        }
    }

    return 0;
}


