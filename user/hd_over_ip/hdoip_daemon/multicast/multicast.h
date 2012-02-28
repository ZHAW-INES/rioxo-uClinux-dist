/*
 * multicast.h
 *
 *  Created on: 12.05.2011
 *      Author: buan
 */
#ifndef MULTICAST_H_
#define MULTICAST_H_

#define MULTICAST_SUCCESS           0
#define MULTICAST_ERROR             1
#define MULTICAST_ABORT             2

#define CLIENT_IS_NOT_IN_LIST       0
#define CLIENT_IS_IN_LIST           1

#define CLIENT_NOT_AVAILABLE        0
#define CLIENT_AVAILABLE_ONLY_ONE   1
#define CLIENT_AVAILABLE_MULTIPLE   2

#define MEDIA_IS_AUDIO              0
#define MEDIA_IS_VIDEO              1

#include "edid.h"


void convert_ip_to_multicast_mac(uint32_t ip, char* mac);
int join_multicast_group(uint32_t multicast_ip);
int leave_multicast_group(uint32_t multicast_ip);

void add_client_to_vtb(int audio_video, uint32_t client_ip);
void remove_client_from_vtb(int audio_video, uint32_t client_ip);
int  check_client_availability(int audio_video);
void report_available_clients();
bool set_multicast_enable(bool enable);
bool get_multicast_enable();

void add_client_to_start_list(char* client_ip_string);
void multicast_handler();
void multicast_add_edid(t_edid* new_edid, char* ip_string);
void multicast_merge_edid(t_edid* edid);
bool multicast_compare_edid(t_edid* edid1, t_edid* edid2);

#endif /* MULTICAST_H_ */
