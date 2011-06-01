/*
 * hoi_cfg.h
 *
 *  Created on: 25.11.2010
 *      Author: stth
 */

#ifndef HOI_CFG_H_
#define HOI_CFG_H_

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <linux/types.h>
#include <stdint.h>
#include <string.h>

void hoi_cfg_system();
void hoi_cfg_static_ip();
void hoi_cfg_dynamic_ip();
void hoi_cfg_mac_addr(char *ifname, char *mac);
void hoi_cfg_default_gw(char *gw);
void hoi_cfg_ip_addr(char *ifname, char *ip, char *netmask);

int hoi_cfg_write(char* filename);
int hoi_cfg_read(char* filename);
int hoi_cfg_get_dns_server(char* dns1, char* dns2);
int hoi_cfg_set_dns_server(char* dns1, char* dns2);
int hoi_cfg_get_default_gw(char *gw);
int hoi_cfg_get_ip(char *ip);
int hoi_cfg_get_subnet(char *subnet);


#endif /* HOI_CFG_H_ */
