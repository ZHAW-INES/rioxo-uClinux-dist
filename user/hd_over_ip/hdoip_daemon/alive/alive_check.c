/*
 * alive_check.c
 *
 *  Created on: Apr 26, 2011
 *      Author: stth, buan
 */

#include <string.h>
#include <unistd.h>

#include "alive_check.h"
#include "box_sys.h"
#include "edid.h"
#include "edid_merge.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hoi_drv_user.h"
#include "multicast.h"
#include "rtsp_client.h"
#include "rtsp_net.h"
#include "rtsp_string.h"

int alive_check_client_open(t_alive_check *handle, bool enable, int interval, char *dest, uint16_t port, int broadcast, bool load_new_config)
{
    struct hostent* host;

    handle->socket                          = -1;
    handle->enable                          = enable;
    handle->interval_cnt                    = 0;
    handle->addr_in.sin_family              = AF_INET;
    handle->edid_stored                     = false;

    if (load_new_config == true) {
        handle->interval                    = interval;
        handle->broadcast                   = broadcast;
        handle->addr_in.sin_port            = htons(port);

        if(strlen(dest) > ALIVE_DEST_LEN) {
            return ALIVE_CHECK_ERROR;
        }
        strcpy(handle->dest, dest);

        if ((host = gethostbyname(handle->dest)) == NULL) {
            handle->addr_in.sin_addr.s_addr = 0;
            return ALIVE_CHECK_ERROR;
        }
        handle->addr_in.sin_addr.s_addr     = *((uint32_t*)host->h_addr_list[0]);
    }

    if (handle->enable) {
        if ((handle->socket = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
            report(ERROR "alive_check: server socket error: %s", strerror(errno));
            return ALIVE_CHECK_ERROR;
        }

        if ((setsockopt(handle->socket, SOL_SOCKET, SO_BROADCAST, &handle->broadcast, sizeof(handle->broadcast))) == -1) {
            report(ERROR "alive_check: client socket broadcast error: %s", strerror(errno));
            handle->socket = -1;
            return ALIVE_CHECK_ERROR;
        }

        if ((connect(handle->socket, (struct sockaddr*)&(handle->addr_in), sizeof(struct sockaddr_in))) == -1) {
            // report(ERROR "alive_check: client socket connect error: %s", strerror(errno));
            handle->socket = -1;
            return ALIVE_CHECK_ERROR;
        }
    }

    return ALIVE_CHECK_SUCCESS;
}

int alive_check_client_close(t_alive_check *handle)
{
    if(handle->socket != -1) {
        shutdown(handle->socket, SHUT_RDWR);
        close(handle->socket);

        handle->socket = -1;
    }
    return ALIVE_CHECK_SUCCESS;
}

int alive_check_client_update(t_alive_check *handle, bool enable, int interval, char *ip, uint16_t port, int broadcast, bool load_new_config)
{
    if (alive_check_client_close(handle) == -1) {
        return ALIVE_CHECK_ERROR;
    }
    if (alive_check_client_open(handle, enable, interval, ip, port, broadcast, load_new_config) == -1) {
        return ALIVE_CHECK_ERROR;
    }
    return ALIVE_CHECK_SUCCESS;
}

int alive_check_client_handler(t_alive_check *handle, char *hello_msg)
{
    if (handle->enable) {
        if (handle->interval_cnt == 0) {
            if ((handle->socket != -1) && (handle->addr_in.sin_addr.s_addr != 0)) {
                if (write(handle->socket, hello_msg, strlen(hello_msg)) == -1) {
                    report(ERROR "alive_check: client write error: %s", strerror(errno));
                    return ALIVE_CHECK_ERROR;
                }
            } else {
                alive_check_client_update(handle,true,handle->interval,handle->dest,ntohs(handle->addr_in.sin_port),handle->broadcast,true);
            }
            if (handle->interval > 1) {
                handle->interval_cnt = handle->interval - 1;
            }
            else {
                handle->interval_cnt = 0;
            }
        }
        else {
            handle->interval_cnt--;
        }
    }

    return ALIVE_CHECK_SUCCESS;
}

uint8_t return_next_byte(char* s)
{
    uint8_t r = 0;

    if ((s[0] >= '0') && (s[0] <= '9'))
        r = r + (s[0] - '0');
    else if ((s[0] >= 'a') && (s[0] <= 'f'))
        r = r + (s[0] - 'a' + 10);
    else if ((s[0] >= 'A') && (s[0] <= 'F'))
        r = r + (s[0] - 'A' + 10);
    r = r << 4;

    if ((s[1] >= '0') && (s[1] <= '9'))
        r = r + (s[1] - '0');
    else if ((s[1] >= 'a') && (s[1] <= 'f'))
        r = r + (s[1] - 'a' + 10);
    else if ((s[1] >= 'A') && (s[1] <= 'F'))
        r = r + (s[1] - 'A' + 10);
    return r;
}
