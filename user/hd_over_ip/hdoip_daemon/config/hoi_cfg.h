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

int hoi_cfg_write(char* filename);
int hoi_cfg_read(char* filename);
int hoi_cfg_get_dns_server(char* dns1, char* dns2);
int hoi_cfg_set_dns_server(char* dns1, char* dns2);

#endif /* HOI_CFG_H_ */
