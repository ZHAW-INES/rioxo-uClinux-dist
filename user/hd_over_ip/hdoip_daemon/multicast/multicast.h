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
#include "rtsp_server.h"

void convert_ip_to_multicast_mac(uint32_t ip, char* mac);

int  multicast_group_join(uint32_t multicast_ip);
int  multicast_group_leave(uint32_t multicast_ip);

void multicast_client_add(int audio_video, t_rtsp_server* server);
void multicast_client_remove(int audio_video, t_rtsp_server* server);
int  multicast_client_check_availability(int audio_video);
void multicast_client_report();

bool multicast_set_enabled(bool enable);
bool multicast_get_enabled();

void multicast_initialize();
bool multicast_edid_compare(t_edid* edid1, t_edid* edid2);

#endif /* MULTICAST_H_ */
