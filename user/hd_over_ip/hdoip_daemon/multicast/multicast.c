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
    int                                 start_timer;
    uint32_t                            number_of_clients;
    pthread_mutex_t mutex_list;         // protect access to the client lists
} multicast;

/*
 * Locks the mutex protecting the list of connections
 */
static void list_lock(const char* s MAYBE_UNUSED)
{
  pthread_mutex_lock(&multicast.mutex_list);
  report2("multicast:list:pthread_mutex_lock(%d, %s)", pthread_self(), s);
}

/*
 * Unlocks the mutex protecting the list of connections
 */
static void list_unlock(const char* s MAYBE_UNUSED)
{
  report2("multicast:list:pthread_mutex_unlock(%d, %s)", pthread_self(), s);
  pthread_mutex_unlock(&multicast.mutex_list);
}

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
    list_lock("multicast_client_add");
    if (audio_video == MEDIA_IS_AUDIO) {
        if (search_client_in_list(server->con.address, server->con.common.session, &multicast.client_list_audio) == CLIENT_IS_NOT_IN_LIST)
            add_client_to_list(server->con.address, server->con.common.session, &multicast.client_list_audio, NULL);
    }
    else if (audio_video == MEDIA_IS_VIDEO) {
        if (search_client_in_list(server->con.address, server->con.common.session, &multicast.client_list_video) == CLIENT_IS_NOT_IN_LIST)
            add_client_to_list(server->con.address, server->con.common.session, &multicast.client_list_video, NULL);
    }
    list_unlock("multicast_client_add");
}

void multicast_client_remove(int audio_video, t_rtsp_server* server)
{
    list_lock("multicast_client_remove");
    if (audio_video == MEDIA_IS_AUDIO)
        remove_client_from_list(server->con.address, &multicast.client_list_audio);
    else if (audio_video == MEDIA_IS_VIDEO)
        remove_client_from_list(server->con.address, &multicast.client_list_video);
    list_unlock("multicast_client_remove");
}

int multicast_client_check_availability(int audio_video)
{
    int ret = CLIENT_NOT_AVAILABLE;
    
    list_lock("multicast_client_check_availability");
    if (audio_video == MEDIA_IS_AUDIO) {
        switch (count_client_list(&multicast.client_list_audio)) {
            case 0:
                ret = CLIENT_NOT_AVAILABLE;
                break;
            case 1:
                ret = CLIENT_AVAILABLE_ONLY_ONE;
                break;
            default:
                ret = CLIENT_AVAILABLE_MULTIPLE;
                break;
        }
    }
    else if (audio_video == MEDIA_IS_VIDEO) {
        switch (count_client_list(&multicast.client_list_video)) {
            case 0:
                ret = CLIENT_NOT_AVAILABLE;
                break;
            case 1:
                ret = CLIENT_AVAILABLE_ONLY_ONE;
                break;
            default:
                ret = CLIENT_AVAILABLE_MULTIPLE;
                break;
        }
    }
    
    list_unlock("multicast_client_check_availability");
    return ret;
}

void multicast_client_report()
{
    list_lock("multicast_client_report");
    report("\n --- client-count-video: %i ---", count_client_list(&multicast.client_list_video));
    report_client_list(&multicast.client_list_video);
    report("\n --- client-count-audio: %i ---", count_client_list(&multicast.client_list_audio));
    report_client_list(&multicast.client_list_audio);
    list_unlock("multicast_client_report");
}

/*
 * Enable or disable multicast.
 *
 * @param enable Whether to enable or disable multicast
 *
 * @return True if multicast is enabled otherwise false
 */
bool multicast_set_enabled(bool enable)
{
    multicast.enable = enable;
    return multicast.enable;
}

/*
 * Check whether multicast is enabled or not.
 *
 * @return True if multicast is enabled otherwise false
 */
bool multicast_get_enabled()
{
    return multicast.enable;
}

/*
 * Initialize the mutexes and conditional variables needed for the multicast
 * connection setup handling.
 */
void multicast_initialize()
{
    // this mutex is needed to protect the lists of clients
    pthread_mutex_init(&multicast.mutex_list, NULL);

    multicast.enable = false;
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
