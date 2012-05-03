/*
 * alive_check.c
 *
 *  Created on: Apr 26, 2011
 *      Author: stth, buan
 */

#include <unistd.h>
#include <string.h>

#include "alive_check.h"
#include "multicast.h"
#include "rscp_net.h"
#include "hdoipd.h"
#include "rscp_string.h"
#include "box_sys.h"
#include "hdoipd_fsm.h"
#include "edid.h"
#include "factory_edid.h"
#include "edid_merge.h"
#include "hoi_drv_user.h"

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

int alive_check_server_open(t_alive_check *handle, bool enable, uint16_t port, bool load_new_config)
{
    int socket_flags;

    handle->socket                          = -1;
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
            return ALIVE_CHECK_ERROR;
        }

        if ((bind(handle->socket, (struct sockaddr*)&handle->addr_in, sizeof(struct sockaddr_in))) == -1) {
            report(ERROR "alive_check: server bind error: %s", strerror(errno));
            handle->socket = -1;
            return ALIVE_CHECK_ERROR;
        }

        if (handle->socket != -1) {
            socket_flags = fcntl(handle->socket, F_GETFL, 0);
            fcntl(handle->socket, F_SETFL, (socket_flags | O_NONBLOCK));
        }
    }
    return ALIVE_CHECK_SUCCESS;
}

int alive_check_server_close(t_alive_check *handle)
{
    if(handle->socket != -1) {
        if ((shutdown(handle->socket, SHUT_RDWR)) == -1 ) {
            report(ERROR "alive_check: server socket shutdown error: %s", strerror(errno));
            return ALIVE_CHECK_ERROR;
        }
        if ((close(handle->socket)) == -1 ) {
            report(ERROR "alive_check: server socket close error: %s", strerror(errno));
            return ALIVE_CHECK_ERROR;
        }
        handle->socket = -1;
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

/*
 * Because of DHCP needs some time to get ip address, 
 * this init function is repeated until ip is available
 *
 */
int alive_check_init_msg_vrb_alive()
{
    uint32_t  hello_uri_length = 200;
    char *s;
    char hello_uri[hello_uri_length];
    t_str_uri uri;


    if (!(hdoipd.alive_check.init_done)) {

        if reg_test("mode-start", "vtb") {
            hdoipd.alive_check.vtb = true;
        } else {
            hdoipd.alive_check.vtb = false;
        }

        if (!hdoipd.alive_check.vtb) {
            // register remote for "hello"
            if (box_sys_set_remote(reg_get("hello-uri")) == -1) {
                return ALIVE_CHECK_ERROR;
            }
            s = reg_get("hello-uri");
            if (strlen(s) < hello_uri_length)
                memcpy(hello_uri, s, strlen(s)+1);
            else {
                memcpy(hello_uri, s, hello_uri_length);
                perrno("[ALIVE] hello-uri size too long");
            }
            str_split_uri(&uri, hello_uri);

            if (alive_check_client_open(&hdoipd.alive_check, reg_test("alive-check", "true"), reg_get_int("alive-check-interval"), uri.server, reg_get_int("alive-check-port"), 0, true)) {
                perrno("[ALIVE] alive_check_client_open() failed");
            }
        }

        if (hdoipd.alive_check.vtb) {
            if (alive_check_server_open(&hdoipd.alive_check, true, reg_get_int("alive-check-port"), true)) {
                perrno("[ALIVE] alive_check_server_open() failed");
            }
        }

    }
    hdoipd.alive_check.init_done = 1;

    return ALIVE_CHECK_SUCCESS;
}

void alive_check_handle_msg_vrb_alive(t_alive_check *handle)
{
    int  msg_length = 30;
    int  edid_length = 256;
    char hello_msg[msg_length+(edid_length*2)];    
    char client_ip[100];
    char edid_string[edid_length*2];
    char buff[2];
    uint8_t edid_table[edid_length];
    int i;
    t_rscp_edid edid;
	int ret;
    t_edid edid_old;
    char remote_uri[100];

    if (!hdoipd.alive_check.vtb) {
        if (hdoipd.alive_check.init_done) {
            memset(hello_msg, 0, (msg_length+(edid_length*2)));
            memset(edid_string, 0, (edid_length*2));

            //read edid and convert into string
            hoi_drv_rdedid(edid.edid);
            for (i=0; i<edid_length; i++) {
                sprintf(buff, "%02x", edid.edid[i]);
                strcat(edid_string, buff);
            }
        
            if (reg_test("system-dhcp", "true")) {
                sprintf(hello_msg, "%s/%s/%s/","HELLO", "VRB", reg_get("system-hostname"));
            } else {
                sprintf(hello_msg, "%s/%s/%s/","HELLO", "VRB", reg_get("system-ip"));
            }

            strcat(hello_msg, edid_string);

            alive_check_client_handler(handle, hello_msg);
        }
    }

    if (hdoipd.alive_check.vtb) {
    	memset(hello_msg, 0, (msg_length+(edid_length*2)));
        if (!alive_check_server_handler(handle, hello_msg, (msg_length+(edid_length*2)))) {
            if (!alive_check_test_msg_vrb_alive(hello_msg, client_ip, edid_table)) {

                report(INFO "HELLO received from: %s", client_ip);

                // use default edid if requested
                if (reg_test("edid-mode", "default")) {
                    memcpy(edid_table, factory_edid, edid_length);
                }

                set_multicast_enable(reg_test("multicast_en", "true"));
                if (get_multicast_enable()) { // multicast
                    // write client_ip to start list so that transmitter is able to start one connection after each other
                    add_client_to_start_list(client_ip);
                    multicast_add_edid((t_edid *)edid_table, client_ip);
                } else { // unicast

                    // only handle hello from remote uri
                    sprintf(remote_uri, "%s", reg_get("remote-uri"));
                    if (strcmp(&(remote_uri[7]), client_ip) == 0) {
                        if(!hdoipd_rsc(RSC_VIDEO_IN_SDI)) {
                            // read old edid
                            ret = edid_read_file(&edid_old, EDID_PATH_VIDEO_IN);
                            if (ret != -1) {
                                // write edid only when it has changed
                                if (ret == -2) { // No previous E-EDID exists
                                    edid_merge((t_edid *)edid_table, (t_edid *)edid_table); // modify edid
                                    edid_write_function((t_edid *)edid_table, "unicast first edid");
                                } else { // A previous E-EDID exists
                                    edid_merge((t_edid *)edid_table, (t_edid *)edid_table); // modify edid
                                    if (multicast_compare_edid(&edid_old, (t_edid *)edid_table)) { // edid has changed
                                        edid_write_function((t_edid *)edid_table, "unicast edid changed");
                                    }
                                }
                            } else {
                                report(ERROR "Failed to read file: %s", EDID_PATH_VIDEO_IN);
                            }
                        }

                        // response only when not already unicast is streaming
                        if (!hdoipd_tstate(VTB_VIDEO | VTB_AUDIO)) {
                            if (hdoipd_rsc(RSC_VIDEO_IN)) {
                                alive_check_response_vrb_alive(client_ip);
                            }
                        }
                    }
                }
            }
        }
    }
}

int alive_check_test_msg_vrb_alive(char *hello_msg, char *client_ip, uint8_t *edid)
{
    int i;
    char type[30];
    char msg[30]; 
    char buff[2];
    
    strcpy(msg, ((char*) str_next_token(&hello_msg, "/")));
    strcpy(type, ((char*) str_next_token(&hello_msg, "/")));
    strcpy(client_ip, ((char*) str_next_token(&hello_msg, "/")));

    for (i=0; i<256; i++) {
        strncpy(buff, (hello_msg+(i*2)), 2);
        edid[i] = return_next_byte(buff);
    }

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

    if (client_ip) {
        sprintf(uri, "%s://%s","rscp", client_ip);
        client = rscp_client_open(hdoipd.client, 0, uri);
        if (client) {
#ifdef REPORT_ALIVE_HELLO
            report(INFO "alive check: say hello to %s", uri);
#endif
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
    if (alive_check_client_update(&hdoipd.alive_check, true, 0, 0, 0, 0, false)) {
        perrno("[ALIVE] alive_check_client_update() failed");
    }
}

void alive_check_stop_vrb_alive()
{
    alive_check_client_update(&hdoipd.alive_check, false, 0, 0, 0, 0, false);
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
