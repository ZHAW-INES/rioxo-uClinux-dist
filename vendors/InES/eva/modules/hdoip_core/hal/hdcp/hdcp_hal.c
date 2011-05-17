/*
 * hdcp_hal.c
 *
 *  Created on: May 17, 2011
 *      Author: stth
 */

#include <stdio.h>
#include "hdcp_hal.h"

void hdcp_get_master_key(void *p, char master_key[32])
{
    snprintf(master_key, sizeof(master_key), "%08x%08x%08x%08x",hdcp_get_master_key0(p),hdcp_get_master_key1(p),hdcp_get_master_key2(p),hdcp_get_master_key3(p));
}

