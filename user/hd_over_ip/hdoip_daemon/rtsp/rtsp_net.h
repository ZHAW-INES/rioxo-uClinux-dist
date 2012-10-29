/*
 * rtsp_net.h
 *
 *  Created on: 01.12.2010
 *      Author: alda
 */

#ifndef RTSP_NET_H_
#define RTSP_NET_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <linux/sockios.h>
#include <linux/if.h>

char *mac_ntoa(unsigned char *ptr);
char* str_ntoa(uint32_t address);

int net_get_local_hwaddr(int sock, char* ifn, uint8_t* mac);
int net_get_local_addr(int sock, char* ifn, uint32_t* addr);
int net_get_remote_hwaddr(int sock, char* ifn, uint32_t address, uint8_t* mac);


#endif /* RTSP_NET_H_ */
