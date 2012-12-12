/*
 * alive_check.h
 *
 *  Created on: Apr 26, 2011
 *      Author: stth, buan
 */

#ifndef ALIVE_CHECK_H_
#define ALIVE_CHECK_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define ALIVE_CHECK_SUCCESS      0
#define ALIVE_CHECK_ERROR       -1

#define ALIVE_DEST_LEN          80

typedef struct {
    bool                enable;
    int                 interval;
    int                 broadcast;
    int                 socket;
    int                 interval_cnt;
    int                 init_done;
    char                dest[80];
    bool                edid_stored;
    struct sockaddr_in  addr_in;
    bool                vtb;
} t_alive_check;

int  alive_check_client_open(t_alive_check *handle, bool enable, int interval, char *dest, uint16_t port, int broadcast, bool load_new_config);
int  alive_check_client_close(t_alive_check *handle);
int  alive_check_client_handler(t_alive_check *handle, char *hello_msg);
int  alive_check_client_update(t_alive_check *handle, bool enable, int interval, char *ip, uint16_t port, int broadcast, bool load_new_config);

uint8_t return_next_byte(char* s);

#endif /* ALIVE_CHECK_H_ */
