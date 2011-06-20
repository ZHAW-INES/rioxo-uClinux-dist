/*
 * client_list.h
 *
 *  Created on: 31.05.2011
 *      Author: buan
 */
#ifndef CLIENT_LIST_H_
#define CLIENT_LIST_H_


int add_client_to_list(uint32_t client_ip, t_client_list* first_client);
int remove_client_from_list(uint32_t client_ip, t_client_list* first_client);
int search_client_in_list(uint32_t client_ip, t_client_list* first_client);
int report_client_list(t_client_list* first_client);
int count_client_list(t_client_list* first_client);

#endif /* CLIENT_LIST_H_ */
