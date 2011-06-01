/*
 * alive_check.c
 *
 *  Created on: Apr 26, 2011
 *      Author: stth, buan
 */

#include <unistd.h>
#include <string.h>

#include "alive_check.h"
#include "rscp_net.h"
#include "debug.h"
#include "hdoipd.h"
#include "rscp_string.h"

int alive_check_client_open(t_alive_check *handle, bool enable, int interval, uint32_t ip, uint16_t port, int broadcast, bool load_new_config)
{
    handle->socket                          = 0;
    handle->enable                          = enable;
    handle->interval_cnt                    = 0;
    handle->addr_in.sin_family              = AF_INET;

    if (load_new_config == true) {
        handle->interval                    = interval;
        handle->broadcast                   = broadcast;
        handle->addr_in.sin_port            = htons(port);
        handle->addr_in.sin_addr.s_addr     = ip;
    }

    if (handle->enable) {
        if ((handle->socket = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
            report(ERROR "alive_check: server socket error: %s", strerror(errno));
            handle->socket = 0;
            return ALIVE_CHECK_ERROR;
        }

        if ((setsockopt(handle->socket, SOL_SOCKET, SO_BROADCAST, &handle->broadcast, sizeof(handle->broadcast))) == -1) {
            report(ERROR "alive_check: client socket broadcast error: %s", strerror(errno));
            handle->socket = 0;
            return ALIVE_CHECK_ERROR;
        }

        if ((connect(handle->socket, (struct sockaddr*)&(handle->addr_in), sizeof(struct sockaddr_in))) == -1) {
            report(ERROR "alive_check: client socket connect error: %s", strerror(errno));
            handle->socket = 0;
            return ALIVE_CHECK_ERROR;
        }
    }
    return ALIVE_CHECK_SUCCESS;
}

int alive_check_client_close(t_alive_check *handle)
{
    if(handle->socket != 0) {
        if (shutdown(handle->socket, SHUT_RDWR) == -1) {
            report(ERROR "alive_check: client socket shutdown error: %s", strerror(errno));
            return ALIVE_CHECK_ERROR;
        }
        if (close(handle->socket) == -1) {
            report(ERROR "alive_check: client socket close error: %s", strerror(errno));
            return ALIVE_CHECK_ERROR;
        }
        handle->socket = 0;
    }
    return ALIVE_CHECK_SUCCESS;
}

int alive_check_client_update(t_alive_check *handle, bool enable, int interval, uint32_t ip, uint16_t port, int broadcast, bool load_new_config)
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
            if (handle->socket) {
                if (write(handle->socket, hello_msg, strlen(hello_msg)) == -1) {
                    report(ERROR "alive_check: client write error: %s", strerror(errno));
                    return ALIVE_CHECK_ERROR;
                }
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

int alive_check_server_open(t_alive_check *handle, bool enable, uint16_t port, bool load_new_config)
{
    int socket_flags;

    handle->socket                          = 0;
    handle->enable                          = enable;
    handle->interval                        = 0;
    handle->interval_cnt                    = 0;
    handle->addr_in.sin_family              = AF_INET;
    handle->addr_in.sin_addr.s_addr         = htonl(INADDR_ANY);

    if (load_new_config == true) {
        handle->addr_in.sin_port            = htons(port);
    }

    if(handle->enable) {
        if ((handle->socket = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
            report(ERROR "alive_check: server socket error: %s", strerror(errno));
            handle->socket = 0;
            return ALIVE_CHECK_ERROR;
        }

        if ((bind(handle->socket, (struct sockaddr*)&handle->addr_in, sizeof(struct sockaddr_in))) == -1) {
            report(ERROR "alive_check: server bind error: %s", strerror(errno));
            handle->socket = 0;
            return ALIVE_CHECK_ERROR;
        }

        if (handle->socket) {
            socket_flags = fcntl(handle->socket, F_GETFL, 0);
            fcntl(handle->socket, F_SETFL, (socket_flags | O_NONBLOCK));
        }
    }
    return ALIVE_CHECK_SUCCESS;
}

int alive_check_server_close(t_alive_check *handle)
{
    if(handle->socket != 0) {
        if ((shutdown(handle->socket, SHUT_RDWR)) == -1 ) {
            report(ERROR "alive_check: server socket shutdown error: %s", strerror(errno));
            return ALIVE_CHECK_ERROR;
        }
        if ((close(handle->socket)) == -1 ) {
            report(ERROR "alive_check: server socket close error: %s", strerror(errno));
            return ALIVE_CHECK_ERROR;
        }
        handle->socket = 0;
    }
    return ALIVE_CHECK_SUCCESS;
}

int alive_check_server_handler(t_alive_check *handle, char *hello_msg, int msg_length)
{
    int msg_size;

    if ((msg_size = read(handle->socket, hello_msg, msg_length)) > 0) {
        return ALIVE_CHECK_SUCCESS;
    }
    return ALIVE_CHECK_ERROR;
}

int alive_check_server_update(t_alive_check *handle, bool enable, uint16_t port, bool load_new_config)
{
    if (alive_check_server_close(handle) == -1) {
        return ALIVE_CHECK_ERROR;
    }

    if (alive_check_server_open(handle, enable, port, load_new_config) == -1) {
        return ALIVE_CHECK_ERROR;
    }
    return ALIVE_CHECK_SUCCESS;
}


//--------------------------------------------
// specific functions to check if vtb is ready


void alive_check_init_msg_vrb_alive()
{
    uint32_t  hello_uri_length = 200;
    char *s;
    char hello_uri[hello_uri_length];
    t_str_uri uri;
    struct hostent* host;

    s = reg_get("hello-uri");
    if (strlen(s) < hello_uri_length)
        memcpy(hello_uri, s, strlen(s)+1);
    else {
        memcpy(hello_uri, s, hello_uri_length);
        perror("[ALIVE] hello-uri size too long");
    }

    str_split_uri(&uri, hello_uri);

    if (!(host = gethostbyname(uri.server))) {
        perror("[ALIVE] get host by name failed");
    }

    if reg_test("mode-start", "vrb") {
        if (alive_check_client_open(&hdoipd.alive_check, reg_test("alive-check", "true"), reg_get_int("alive-check-interval"), *((uint32_t*)host->h_addr_list[0]), reg_get_int("alive-check-port"), 0, true)) {
            perror("[ALIVE] alive_check_client_open() failed");
        }
    }

    if reg_test("mode-start", "vtb") {
        if (alive_check_server_open(&hdoipd.alive_check, true, reg_get_int("alive-check-port"), true)) {
            perror("[ALIVE] alive_check_server_open() failed");
        }
    }
}

void alive_check_handle_msg_vrb_alive(t_alive_check *handle)
{
    int  msg_length = 30;
    char hello_msg[msg_length];    
    char client_ip[30];

    if reg_test("mode-start", "vrb") {
        memset(&hello_msg, 0, msg_length);
        sprintf(hello_msg, "%s/%s/%s","HELLO", "VRB", reg_get("system-ip"));
        alive_check_client_handler(handle, hello_msg);
    }

    if reg_test("mode-start", "vtb") {
        memset(&hello_msg, 0, msg_length);
        if (!alive_check_server_handler(handle, hello_msg, msg_length)) {
            if (!alive_check_test_msg_vrb_alive(hello_msg, client_ip)) {
                alive_check_response_vrb_alive(client_ip);
            }
        }
    }
}

int alive_check_test_msg_vrb_alive(char *hello_msg, char *client_ip)
{
    char type[30];
    char msg[30]; 
    
    strcpy(msg, ((char*) str_next_token(&hello_msg, "/")));
    strcpy(type, ((char*) str_next_token(&hello_msg, "/")));

    strcpy(client_ip, hello_msg);

    if (!strcmp(msg, "HELLO")) {
        if (!strcmp(type, "VRB")) {
            return ALIVE_CHECK_SUCCESS;
        }
    }
    return ALIVE_CHECK_ERROR;
}

int alive_check_response_vrb_alive(char *client_ip) 
{
    t_rscp_client *client;
    char uri[40];

    sprintf(uri, "%s://%s","rscp", client_ip);

    if (client_ip) {
        client = rscp_client_open(hdoipd.client, 0, uri);
        if (client) {
            report(INFO "alive check: say hello to %s", uri);
            rscp_client_hello(client);
            rscp_client_close(client);
            return ALIVE_CHECK_SUCCESS;
        } else {
            report(ERROR "alive check: could not say hello to %s", uri);
        }
    }
    return ALIVE_CHECK_ERROR;
}

void alive_check_start_vrb_alive()
{
    if(hdoipd.auto_stream) {
        if (alive_check_client_update(&hdoipd.alive_check, true, 0, 0, 0, 0, false)) {
            perror("[ALIVE] alive_check_client_update() failed");
        }
    }
}

void alive_check_stop_vrb_alive()
{
    alive_check_client_update(&hdoipd.alive_check, false, 0, 0, 0, 0, false);
}
