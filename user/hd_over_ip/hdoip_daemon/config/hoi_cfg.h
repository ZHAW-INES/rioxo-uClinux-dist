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


//------------------------------------------------------------------------------
// Config values

#define HOI_CFG_MODE_RX     0x00000001
#define HOI_CFG_MODE_TX     0x00000002

#define HOI_CFG_VERSION     0

//------------------------------------------------------------------------------
// Config struct

typedef struct {
    char                file_location[60];
} t_hoi_cfg;


//------------------------------------------------------------------------------
// Config function prototyps

int hoi_cfg_init(t_hoi_cfg* handle, char* file_location);
int hoi_cfg_save(t_hoi_cfg* handle);
int hoi_cfg_read(t_hoi_cfg* handle);

#endif /* HOI_CFG_H_ */
