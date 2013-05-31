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
    volatile bool                                setup_succeeded;

    pthread_mutex_t mutex_list;         // protect access to the client lists
    pthread_mutex_t mutex_timer;        // protect access to the timer condition
    pthread_cond_t  cond_timer;         // timer condition used during connection setup
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

/*
 * Locks the mutex protecting the setup timer
 */
static void timer_lock(const char* s MAYBE_UNUSED)
{
  pthread_mutex_lock(&multicast.mutex_timer);
  report2("multicast:timer:pthread_mutex_lock(%d, %s)", pthread_self(), s);
}

/*
 * Unlocks the mutex protecting the setup timer
 */
static void timer_unlock(const char* s MAYBE_UNUSED)
{
  report2("multicast:timer:pthread_mutex_unlock(%d, %s)", pthread_self(), s);
  pthread_mutex_unlock(&multicast.mutex_timer);
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

int merge_edids(t_edid* edid)
{
    return merge_edid_list(&multicast.client_list_edid, edid);
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
    report("\n --- client-count-edid:  %i ---", count_client_list(&multicast.client_list_edid));
    report_client_list(&multicast.client_list_edid);
    report("\n --- client-count-start:  %i ---", count_client_list(&multicast.client_list_start));
    report_client_list(&multicast.client_list_start);
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
//    if (multicast.enable != enable)
//        multicast.setup_succeeded = false;

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
    // this mutex is needed to protect the lists of start clients, edid etc
    pthread_mutex_init(&multicast.mutex_list, NULL);
    // this mutex is needed to protect the conditional variable used during the setup of connections
    pthread_mutex_init(&multicast.mutex_timer, NULL);
    // this conditional variable is used to setup the connections
    // when a new connection is added in multicast_connection_add() it kicks off the
    // timer which is handled in multicast_handler(). once the timer has expired the
    // setup routine is executed, the result is stored in setup_succeeded and the conditional
    // variable is triggered to wake up any new connections waiting for the setup to finish
    pthread_cond_init(&multicast.cond_timer, NULL);

//    multicast.setup_succeeded = false;
    multicast.enable = false;
}

/*
 * Adds the given connection to the list of connections which need to be setup.
 * This must be called AFTER calling multicast_edid_add(). If it's the first
 * connection it starts the setup timer handled in multicast_handler().
 * This call blocks until the setup procedure has been finished.
 *
 * @param server Connection to setup
 * @return 0 on success otherwise non-zero
 */
int multicast_connection_add(t_rtsp_server* server)
{
    int ret = 0;
    int temp = 0;
    list_lock("multicast_connection_add");

    // put client into start list if its not already there
    if (search_client_in_list(server->con.address, server->con.common.session, &multicast.client_list_start) == CLIENT_IS_NOT_IN_LIST) {
        add_client_to_list(server->con.address, server->con.common.session, &multicast.client_list_start, NULL);
        // check if this is the first connection
        // => wait time "rtsp-timeout" to capture other clients before starting to stream
        if (count_client_list(&multicast.client_list_edid) == 1) {
            if (multicast.start_timer < reg_get_int("rtsp-timeout")) {
                multicast.start_timer = reg_get_int("rtsp-timeout");
            }
        }
        // we need to unlock here before we try to grab the lock that keeps us waiting until
        //list_unlock("multicast_connection_add");

        // get the timer lock because it needs to be held when checking the conditional variable
        //timer_lock("multicast_connection_add");

        // we need to unlock the global lock here because otherwise hdoipd_osd_timer won't be able to run
        //unlock("multicast_connection_add");
        // wait for the condition variable to be triggered
        //pthread_cond_wait(&multicast.cond_timer, &multicast.mutex_timer);
        // check if setup succeeded or not
        //while(!multicast.setup_succeeded){}
//        while(1){
//            if(multicast.setup_succeeded){
                ret = 0;
//                break;
//            }
//            else {
//                ret = -1;
//               usleep(10000);
//                if(++temp>300){
//                    report("multicast_connection_add timed out");
//                    break;
//                }
//            }
//        }


        // get the global lock (which we previously unlocked) back
        //lock("multicast_connection_add");

        //timer_unlock("multicast_connection_add");
    }
    //else
        list_unlock("multicast_connection_add");

    return ret;
}

/*
 * This method is called periodically. It checks if there are any new connections
 * which need to be setup. If it's the first multicast connection a timer is used
 * to give other connections the chance to also join the multicast setup procedure.
 * This is done to avoid having to re-configure the EDID with every new connection
 * which would lead to a change in resolution and therefore requires the connection
 * to be closed and re-opened. If a new connection arrives while other connections
 * are already setup it is tested whether the EDID of the new connection is compatible
 * with the currently used EDID. If that's the case the setup procedure is successful.
 * Otherwise a new EDID has to be merged and configured and all the existing connections
 * have to be closed and re-opened. In that case the setup procedure of the new connection
 * will fail which will automatically lead to closing the connection as well.
 *
 * This method hold both the list and the timer lock.
 */
void multicast_handler()
{
    int         ret;
    uint32_t    client_ip;
    t_edid      edid, edid_old;
    uint32_t    number_of_clients;
    t_client_list* client;

    // nothing to do here for SDI
    if (hdoipd_rsc(RSC_VIDEO_IN_SDI))
        return;

    list_lock("multicast_handler");
    timer_lock("multicast_handler");
    if (multicast.start_timer > 0)
        multicast.start_timer--;
    else {
//        multicast.setup_succeeded = true;

        // EDID handling
        if (count_client_list(&multicast.client_list_start) > 0) {
            // read old EDID if it exists
            ret = edid_read_file(&edid_old, EDID_PATH_VIDEO_IN);
            if (ret != -1) {
                // try to merge all the EDIDs
                if (!merge_edids(&edid)) {
                    // no previous EDID exists => use the one we just merged
                    if (ret == -2)
                        edid_write_function(&edid, "multicast first edid");
                    // there's a previous EDID, so compare it to the old one
                    else if (multicast_edid_compare(&edid, &edid_old)) {
                        // the EDID has changed, we have to close all existing connections
                        // so that they can be re-started with the new resolution
                        while (count_client_list(&multicast.client_list_edid)) {
                            client = get_first_client_and_remove_it_from_list(&multicast.client_list_edid);
                            // only TEARDOWN and close the clients that are not in the start list
                            // as for those the SETUP will fail anyway
                            report("closing connection to client: %s", client->sessionid);
                            if (search_client_in_list(client->ip, client->sessionid, &multicast.client_list_start) == CLIENT_IS_NOT_IN_LIST){
                                rtsp_listener_close_connection(&hdoipd.listener, client_ip, true);
                            }
                        }
                        // use the new EDID
                        edid_write_function(&edid, "multicast edid changed");

                        //multicast.setup_succeeded = false;
                    }
                }
            }
            else {
                report(ERROR "Failed to read file: %s", EDID_PATH_VIDEO_IN);
//                multicast.setup_succeeded = false;
            }

            // remove all remaining clients from the start list
            while (count_client_list(&multicast.client_list_start))
                get_first_client_and_remove_it_from_list(&multicast.client_list_start);

            // make sure the EDID change was successful
//            if (check_input_after_edid_changed() != 0)
//                multicast.setup_succeeded = false;
        }

        // check if client is lost -> edid must be merged new
        number_of_clients = count_client_list(&multicast.client_list_edid);

        if (multicast.number_of_clients > number_of_clients && number_of_clients > 0) {
            ret = edid_read_file(&edid_old, EDID_PATH_VIDEO_IN);
            if (ret != -1) {
                if (!merge_edids(&edid)) {
                    if (multicast_edid_compare(&edid, &edid_old)) {
                        // teardown all connections so that connection with new resolution can be started
                        rtsp_listener_teardown_all(&hdoipd.listener);
                        rtsp_listener_close_all(&hdoipd.listener);
                        edid_write_function(&edid, "multicast client lost");

//                        multicast.setup_succeeded = false;
                    }
                }
            }
            else
                report(ERROR "Failed to read file: %s", EDID_PATH_VIDEO_IN);
        }
        multicast.number_of_clients = number_of_clients;
        pthread_cond_broadcast(&multicast.cond_timer);
    }
    timer_unlock("multicast_handler");
    list_unlock("multicast_handler");
}

/*
 * Adds the given EDID together with the given connection to the list of managed EDIDs.
 * This method must be called BEFORE calling multicast_connection_add().
 *
 * @param new_edid EDID to be added
 * @param server connection to be added
 */
void multicast_edid_add(t_edid* new_edid, t_rtsp_server* server)
{ 
    t_edid* edid = NULL;

    list_lock("multicast_edid_add");
    // add edid to list if its not already there
    if (search_client_in_list(server->con.address, server->con.common.session, &multicast.client_list_edid) == CLIENT_IS_NOT_IN_LIST) {
        if ((edid = (t_edid*)malloc(sizeof(t_edid))) == NULL){
            report(ERROR "Cant allocate memory for EDID");
        } else {
            memcpy(edid, new_edid, sizeof(t_edid));
            add_client_to_list(server->con.address, server->con.common.session, &multicast.client_list_edid, edid);
        }
    }
    list_unlock("multicast_edid_add");
}

/*
 * Removes the EDID stored together with the given connection from the list of managed EDIDs.
 *
 * @param server connection to be removed
 */
void multicast_edid_remove(t_rtsp_server* server)
{
    list_lock("multicast_edid_remove");
    remove_client_from_list(server->con.address, &multicast.client_list_edid);
    list_unlock("multicast_edid_remove");
}

void multicast_start_remove(t_rtsp_server* server)
{
    list_lock("multicast_start_remove");
    remove_client_from_list(server->con.address, &multicast.client_list_start);
    list_unlock("multicast_start_remove");
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
    list_lock("multicast_edid_report");
    report_client_list(&multicast.client_list_edid);
    list_unlock("multicast_edid_report");
}


int multicast_check_client_list(uint32_t client_ip, char* sessionid){
    char temp[20] = "";

    list_lock("multicast_check_client_list");
    
    if(sessionid == NULL){
        if(search_client_in_list(client_ip, temp, &multicast.client_list_video)==CLIENT_IS_IN_LIST){
            list_unlock("multicast_check_client_list");
            return CLIENT_IS_IN_LIST;
        }
        if(search_client_in_list(client_ip, temp, &multicast.client_list_audio)==CLIENT_IS_IN_LIST){
            list_unlock("multicast_check_client_list");
            return CLIENT_IS_IN_LIST;
        }
    }
    else{
        if(search_client_in_list(client_ip, sessionid, &multicast.client_list_video)==CLIENT_IS_IN_LIST){
            list_unlock("multicast_check_client_list");
            return CLIENT_IS_IN_LIST;
        }
        if(search_client_in_list(client_ip, sessionid, &multicast.client_list_audio)==CLIENT_IS_IN_LIST){
            list_unlock("multicast_check_client_list");
            return CLIENT_IS_IN_LIST;
        }
    }

    list_unlock("multicast_check_client_list");

    return CLIENT_IS_NOT_IN_LIST;
}
