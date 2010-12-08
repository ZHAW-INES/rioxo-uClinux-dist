/*
 * hdoipd_osd.h
 *
 *  Created on: 26.11.2010
 *      Author: alda
 */

#ifndef HDOIPD_OSD_H_
#define HDOIPD_OSD_H_

#include <stdio.h>
#include "hdoipd.h"

void hdoipd_osd_init();
void hdoipd_osd_activate();

#define osd_printf(...) \
{ \
    hdoipd_osd_activate(); \
    fprintf(handle.drv, __VA_ARGS__); \
}


#endif /* HDOIPD_OSD_H_ */
