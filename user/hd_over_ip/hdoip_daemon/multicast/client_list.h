/*
 * client_list.h
 *
 *  Created on: 31.05.2011
 *      Author: buan
 */
#ifndef CLIENT_LIST_H_
#define CLIENT_LIST_H_

#include "edid.h"

typedef struct {
    uint32_t    ip;
    void*       edid;
    void*       next;
} __attribute__((packed)) t_client_list;

int add_client_to_list(uint32_t client_ip, t_client_list* first_client, t_edid* edid);
int remove_client_from_list(uint32_t client_ip, t_client_list* first_client);
int search_client_in_list(uint32_t client_ip, t_client_list* first_client);
int report_client_list(t_client_list* first_client);
int count_client_list(t_client_list* first_client);
uint32_t get_first_client_and_remove_it_from_list(t_client_list* first_client);
int merge_edid_list(t_client_list* first_client, t_edid* edid);

#endif /* CLIENT_LIST_H_ */
