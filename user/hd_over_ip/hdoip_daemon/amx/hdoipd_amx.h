/*
 * amx.h
 *
 *  Created on: Apr 26, 2011
 *      Author: stth
 */

#ifndef HDOIPD_AMX_H_
#define HDOIPD_AMX_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct {
    bool                enable;
    int                 socket;
    int                 interval;
    int                 interval_cnt;
    struct sockaddr_in  addr_in;
} t_hdoip_amx;

int hdoipd_amx_open(t_hdoip_amx *handle, bool enable, int interval, uint32_t ip, uint16_t port);
int hdoipd_amx_close(t_hdoip_amx *handle);
int hdoipd_amx_handler(t_hdoip_amx *handle, char *hello_msg);
int hdoipd_amx_update(t_hdoip_amx *handle, bool enable, int interval, uint32_t ip, uint16_t port);

#endif /* HDOIPD_AMX_H_ */
