/*
 * hdcp_hal.c
 *
 *  Created on: May 17, 2011
 *      Author: stth
 */

#include "hdcp_hal.h"

void hdcp_get_master_key(void *p, uint32_t master_key[4])
{
    master_key[0] = hdcp_get_master_key0(p);
    master_key[1] = hdcp_get_master_key1(p);
    master_key[2] = hdcp_get_master_key2(p);
    master_key[3] = hdcp_get_master_key3(p);
}

