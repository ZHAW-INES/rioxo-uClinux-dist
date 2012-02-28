/*
 * multicast.c
 *
 *  Created on: 12.05.2011
 *      Author: buan
 */

#include <stdint.h>
#include <stdbool.h>
#include "multicast.h"
#include "client_list.h"
#include "rscp_parse_header.h"
#include "rscp_net.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"


static struct {
    bool                                group_joined;
    int                                 sock;
    bool                                enable;
    t_client_list                       client_list_audio;
    t_client_list                       client_list_video;
    t_client_list                       client_list_start;
    t_client_list                       client_list_edid;
    int                                 start_timer;
    int                                 number_of_clients;
} multicast;


void convert_ip_to_multicast_mac(uint32_t ip, char* mac)
{
    mac[0] = 0x01;
    mac[1] = 0x00;
    mac[2] = 0x5E;
    mac[3] = (ip & 0x00007F00) >> 8;
    mac[4] = (ip & 0x00FF0000) >> 16;
    mac[5] = (ip & 0xFF000000) >> 24;
}

int join_multicast_group(uint32_t multicast_ip)
{
    struct  sockaddr_in  addr;
    struct  ip_mreq mreq;
    char    loopback = 0;

    if (multicast.group_joined == true)
        return MULTICAST_ABORT;

    // set interface address
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(554);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // open UDP socket
    if ((multicast.sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        report(ERROR "Multicast: socket error: %s", strerror(errno));
        return MULTICAST_ERROR;
    }

    // bind socket to interface
    if (bind(multicast.sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1 ) {
        report(ERROR "Multicast: bind error: %s", strerror(errno));
        return MULTICAST_ERROR;
    }

    // disable loopback (is enabled on default)
    if (setsockopt(multicast.sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loopback, sizeof(loopback)) == -1 ) {
        report(ERROR "Multicast: loopback error: %s", strerror(errno));
        return MULTICAST_ERROR;
    }

    // Join multicast group
    mreq.imr_multiaddr.s_addr = multicast_ip;
    mreq.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(multicast.sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1) {
        report(ERROR "Multicast: join group error: %s", strerror(errno));
        return MULTICAST_ERROR;
    }
    multicast.group_joined = true;

    return MULTICAST_SUCCESS;
}

int leave_multicast_group(uint32_t multicast_ip)
{
    struct ip_mreq mreq;

    if (multicast.group_joined == false)
        return MULTICAST_ERROR;

    // leave multicast group
    mreq.imr_multiaddr.s_addr = multicast_ip;
    mreq.imr_interface.s_addr = INADDR_ANY;
    if ((setsockopt(multicast.sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq))) == -1) {
        report(ERROR "Multicast: leave group error: %s", strerror(errno));
        return MULTICAST_ERROR;
    }
    if (close(multicast.sock) == -1) {
        report(ERROR "Multicast: socket close error: %s", strerror(errno));
        return MULTICAST_ERROR;
    }
    multicast.group_joined = false;

    return MULTICAST_SUCCESS;
}

void add_client_to_vtb(int audio_video, uint32_t client_ip)
{
    if (audio_video == MEDIA_IS_AUDIO) {
        if(!search_client_in_list(client_ip, &multicast.client_list_audio)) {
            add_client_to_list(client_ip, &multicast.client_list_audio, NULL);
        }
    }
    if (audio_video == MEDIA_IS_VIDEO) {
        if(!search_client_in_list(client_ip, &multicast.client_list_video)) {
            add_client_to_list(client_ip, &multicast.client_list_video, NULL);
        }
    }
}

void remove_client_from_vtb(int audio_video, uint32_t client_ip)
{

    if (audio_video == MEDIA_IS_AUDIO) {
        remove_client_from_list(client_ip, &multicast.client_list_audio);
    }
    if (audio_video == MEDIA_IS_VIDEO) {
        remove_client_from_list(client_ip, &multicast.client_list_video);
    }
}

int check_client_availability(int audio_video)
{

    if (audio_video == MEDIA_IS_AUDIO) {
        switch (count_client_list(&multicast.client_list_audio)) {
            case 0:     return CLIENT_NOT_AVAILABLE;
            case 1:     return CLIENT_AVAILABLE_ONLY_ONE;
            default:    return CLIENT_AVAILABLE_MULTIPLE;
        }
    }
    if (audio_video == MEDIA_IS_VIDEO) {
        switch (count_client_list(&multicast.client_list_video)) {
            case 0:     return CLIENT_NOT_AVAILABLE;
            case 1:     return CLIENT_AVAILABLE_ONLY_ONE;
            default:    return CLIENT_AVAILABLE_MULTIPLE;
        }
    }
    return MULTICAST_ERROR;
}

void report_available_clients()
{
    report("\n --- client-count-video: %i ---", count_client_list(&multicast.client_list_video));
    report_client_list(&multicast.client_list_video);
    report("\n --- client-count-audio: %i ---", count_client_list(&multicast.client_list_audio));
    report_client_list(&multicast.client_list_audio);
}

bool set_multicast_enable(bool enable)
{
    multicast.enable = enable;
    
    return multicast.enable;
}

bool get_multicast_enable()
{
    return multicast.enable;
}

void add_client_to_start_list(char* client_ip_string)
{
    uint32_t    client_ip;
    char        client_ip_string_copy[30];

    if (strlen(client_ip_string) < 30) {
        strcpy(client_ip_string_copy, client_ip_string);
        report(INFO "\nMulticast put %s into start-list", client_ip_string_copy);

        // convert string into integer
        rscp_parse_ip(client_ip_string_copy, &client_ip);

        // put client into start list if its not already there
        if(!search_client_in_list(client_ip, &multicast.client_list_start)) {
            add_client_to_list(client_ip, &multicast.client_list_start, NULL);
            if (count_client_list(&multicast.client_list_start) == 1) {      // first connection = wait time "alive-check-interval" to capture other clients until start first connection
                if (multicast.start_timer < reg_get_int("alive-check-interval")) {
                    multicast.start_timer = reg_get_int("alive-check-interval");
                }
            }
        }
    } else {
        report(ERROR "Cant put %s into start-list", client_ip_string_copy);
    }
}

void multicast_handler()
{
    uint32_t    client_ip;
    char        client_ip_string[30];

    if (multicast.start_timer > 0) {
        multicast.start_timer --;
    } else {
        if (count_client_list(&multicast.client_list_start) > 0) {

            if (hdoipd.hdcp.enc_state == 0) {
                multicast.start_timer = 3;
            } else {
                multicast.start_timer = 7;
            }

            // get first client in list, delete it from list and start connection
            client_ip = get_first_client_and_remove_it_from_list(&multicast.client_list_start);

            // convert integer to string
            sprintf(client_ip_string, "%i.%i.%i.%i", ((client_ip >> 0) & 0xFF), ((client_ip >> 8) & 0xFF), ((client_ip >> 16) & 0xFF), ((client_ip >> 24) & 0xFF));

            // response to client
            if (hdoipd_rsc(RSC_VIDEO_IN)) {
                alive_check_response_vrb_alive(client_ip_string);
            }
        }

        // check if client is lost -> edid must be merged new
        if (multicast.number_of_clients != count_client_list(&multicast.client_list_video)) {
            //TODO: write new edid
        }
        multicast.number_of_clients = count_client_list(&multicast.client_list_video);
    }
}

void multicast_add_edid(t_edid* new_edid, char* ip_string)
{ 
    t_edid *edid;
    uint32_t ip;
    char ip_string_copy[30];

    if (strlen(ip_string) < 30) {
        strcpy(ip_string_copy, ip_string);

        // convert string into integer
        rscp_parse_ip(ip_string_copy, &ip);

        // add edid to list if its not already there
        if(!search_client_in_list(ip, &multicast.client_list_edid)) {        
            if ((edid = (t_edid*) malloc(sizeof(t_edid))) == NULL) {
                report(ERROR "Cant allocate memory for EDID of %s", ip_string);
            } else {
                report(INFO "Add edid from %s to list", ip_string);
                memcpy(edid, new_edid, sizeof(t_edid));
                add_client_to_list(ip, &multicast.client_list_edid, edid);
            }
        }
    } else {
        report(ERROR "Cant add edid from %s into start-list", ip_string);
    }
}

void multicast_merge_edid(t_edid* edid)
{
    merge_edid_list(&multicast.client_list_edid, edid);
}

bool multicast_compare_edid(t_edid* edid1, t_edid* edid2)
{
    int i;

    for (i=0;i<256;i++) {
        if (((uint8_t*)edid1)[i] != ((uint8_t*)edid2)[i]) {
            return true;
        }
    }
    return false;
}

