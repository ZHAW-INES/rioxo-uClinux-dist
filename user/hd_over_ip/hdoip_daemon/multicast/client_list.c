/*
 * client_list.c
 *
 *  Created on: 31.05.2011
 *      Author: buan
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "multicast.h"
#include "client_list.h"
#include "debug.h"


int add_client_to_list(uint32_t client_ip, t_client_list* first_client)
{
    t_client_list*   tmp = first_client;
    t_client_list*   new_client;

    // search last client
    while (tmp->next) {
        tmp = tmp->next;
    }

    if ((new_client = (t_client_list*) malloc(sizeof(t_client_list))) == NULL)
        return MULTICAST_ERROR;

    new_client->ip = client_ip;
    new_client->next = NULL;
    tmp->next = new_client;
    return MULTICAST_SUCCESS;
}

int remove_client_from_list(uint32_t client_ip, t_client_list* first_client)
{
    t_client_list*  client = first_client;
    t_client_list*  found_client = NULL;

    // search if client_ip is in list
    while (client->next) {
        client = client->next;
        if (client->ip == client_ip) {
            found_client = client;
            break;
        }
    }

    // when it is in list -> delete item
    if (found_client) {
        client = first_client;
        while (client->next) {
            if (client->next == found_client) {
                client->next = found_client->next;
                break;
            }
            client = client->next;
        }
        free(found_client);
        return MULTICAST_SUCCESS;
    }
    return MULTICAST_ERROR;
}

int search_client_in_list(uint32_t client_ip, t_client_list* first_client)
{
    t_client_list*  client = first_client;

    while (client->next) {
        client = client->next;
        if (client->ip == client_ip)
            return CLIENT_IS_IN_LIST;
    }
    return CLIENT_IS_NOT_IN_LIST;
}

int report_client_list(t_client_list* first_client)
{
    t_client_list*  client = first_client;
    int             client_count = 0;

    while (client->next) {
        client_count++;
        client = client->next;
        report("| Nr: %02i - %03i.%03i.%03i.%03i |", client_count, ((client->ip & 0x000000FF)>>0), ((client->ip & 0x0000FF00)>>8), ((client->ip & 0x00FF0000)>>16), ((client->ip & 0xFF000000)>>24));
        if (client_count == 10)
            break;
    }
    return client_count;
}

int count_client_list(t_client_list* first_client)
{
    t_client_list*  client = first_client;
    int             client_count = 0;

    while (client->next) {
        client = client->next;
        client_count++;
    }
    return client_count;
}
