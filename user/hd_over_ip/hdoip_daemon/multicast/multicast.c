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
#include "rtsp_parse_header.h"
#include "rtsp_net.h"
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
    uint32_t                            number_of_clients;
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

int multicast_group_join(uint32_t multicast_ip)
{
    struct  sockaddr_in  addr;
    struct  ip_mreq mreq;
    char    loopback = 0;

    if (multicast.group_joined == true)
        return MULTICAST_ABORT;

    // set interface address
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = reg_get_int("alive-check-port");
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

int multicast_group_leave(uint32_t multicast_ip)
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

void multicast_client_add(int audio_video, t_rtsp_server* server)
{
    if (audio_video == MEDIA_IS_AUDIO) {
        if(!search_client_in_list(server->con.address, &multicast.client_list_audio)) {
            add_client_to_list(server->con.address, &multicast.client_list_audio, NULL);
        }
    }
    if (audio_video == MEDIA_IS_VIDEO) {
        if(!search_client_in_list(server->con.address, &multicast.client_list_video)) {
            add_client_to_list(server->con.address, &multicast.client_list_video, NULL);
        }
    }
}

void multicast_client_remove(int audio_video, t_rtsp_server* server)
{

    if (audio_video == MEDIA_IS_AUDIO) {
        remove_client_from_list(server->con.address, &multicast.client_list_audio);
    }
    if (audio_video == MEDIA_IS_VIDEO) {
        remove_client_from_list(server->con.address, &multicast.client_list_video);
    }
}

int multicast_client_check_availability(int audio_video)
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

void multicast_client_report()
{
    report("\n --- client-count-video: %i ---", count_client_list(&multicast.client_list_video));
    report_client_list(&multicast.client_list_video);
    report("\n --- client-count-audio: %i ---", count_client_list(&multicast.client_list_audio));
    report_client_list(&multicast.client_list_audio);
    report("\n --- client-count-edid:  %i ---", count_client_list(&multicast.client_list_edid));
    report_client_list(&multicast.client_list_edid);
}

bool multicast_set_enabled(bool enable)
{
    multicast.enable = enable;
    
    return multicast.enable;
}

bool multicast_get_enabled()
{
    return multicast.enable;
}

void multicast_connection_add(t_rtsp_server* server)
{
    //report(INFO "\nMulticast put %s into start-list", client_ip_string);

    // put client into start list if its not already there
    if(!search_client_in_list(server->con.address, &multicast.client_list_start)) {
        add_client_to_list(server->con.address, &multicast.client_list_start, NULL);
        if (count_client_list(&multicast.client_list_start) == 1) {      // first connection = wait time "alive-check-interval" to capture other clients until start first connection
            if (multicast.start_timer < reg_get_int("alive-check-interval")) {
                multicast.start_timer = reg_get_int("alive-check-interval");
            }
        }
    }
}

void multicast_handler()
{
    int         ret;
    uint32_t    client_ip;
    char        client_ip_string[30];
    t_edid      edid, edid_old;
    uint32_t    number_of_clients;

    if (multicast.start_timer > 0) {
        multicast.start_timer --;
    } else {
        if (count_client_list(&multicast.client_list_start) > 0) {

            // EDID handling
            if(!hdoipd_rsc(RSC_VIDEO_IN_SDI)) {
                ret = edid_read_file(&edid_old, EDID_PATH_VIDEO_IN);
                if (ret != -1) {

                    if (!multicast_edid_merge(&edid)) {

                        if (ret == -2) {        // No previous E-EDID exists
                            edid_write_function(&edid, "multicast first edid");
                        } else if (multicast_edid_compare(&edid, &edid_old)) {
                            // teardown all connections so that connection with new resolution can be started
                            rtsp_listener_teardown_all(&hdoipd.listener);
                            rtsp_listener_close_all(&hdoipd.listener);

                            // clear start list
                            while (count_client_list(&multicast.client_list_start)) {
                                get_first_client_and_remove_it_from_list(&multicast.client_list_start);
                            }
                            // clear edid list
                            while (count_client_list(&multicast.client_list_edid)) {
                                get_first_client_and_remove_it_from_list(&multicast.client_list_edid);
                            }

                            edid_write_function(&edid, "multicast edid changed");
                        }
                    }
                } else {
                    report(ERROR "Failed to read file: %s", EDID_PATH_VIDEO_IN);
                }
            }
        }

        // start connection
        if (count_client_list(&multicast.client_list_start) > 0) {
            if (hdoipd_rsc(RSC_VIDEO_IN)) {

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
                alive_check_response_vrb_alive(client_ip_string);
            }
        }

        // check if client is lost -> edid must be merged new
        number_of_clients = count_client_list(&multicast.client_list_edid);

        if ((multicast.number_of_clients > number_of_clients) && (number_of_clients > 0)) {
            ret = edid_read_file(&edid_old, EDID_PATH_VIDEO_IN);
            if (ret != -1) {
                if (!multicast_edid_merge(&edid)) {
                    if (multicast_edid_compare(&edid, &edid_old)) {
                        // teardown all connections so that connection with new resolution can be started
                        rtsp_listener_teardown_all(&hdoipd.listener);
                        rtsp_listener_close_all(&hdoipd.listener);
                        edid_write_function(&edid, "multicast client lost");
                    }
                }
            } else {
                report(ERROR "Failed to read file: %s", EDID_PATH_VIDEO_IN);
            }
        }
        multicast.number_of_clients = number_of_clients;
    }
}

void multicast_edid_add(t_edid* new_edid, t_rtsp_server* server)
{ 
    t_edid*     edid;

    // add edid to list if its not already there
    if(!search_client_in_list(server->con.address, &multicast.client_list_edid)) {
        if ((edid = (t_edid*) malloc(sizeof(t_edid))) == NULL) {
            report(ERROR "Cant allocate memory for EDID");
        } else {
            memcpy(edid, new_edid, sizeof(t_edid));
            add_client_to_list(server->con.address, &multicast.client_list_edid, edid);
        }
    }
}

void multicast_edid_remove(t_rtsp_server* server)
{
    remove_client_from_list(server->con.address, &multicast.client_list_edid);
}

int multicast_edid_merge(t_edid* edid)
{
    return merge_edid_list(&multicast.client_list_edid, edid);
}

bool multicast_edid_compare(t_edid* edid1, t_edid* edid2)
{
    int i;

    for (i=0;i<256;i++) {
        if (((uint8_t*)edid1)[i] != ((uint8_t*)edid2)[i]) {
            return true;
        }
    }
    return false;
}

void multicast_edid_report()
{
    report_client_list(&multicast.client_list_edid);
}

