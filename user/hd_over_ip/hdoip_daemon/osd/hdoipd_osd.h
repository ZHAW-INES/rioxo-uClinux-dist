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

void hdoipd_osd_init(t_hdoipd* handle);
void hdoipd_osd_activate(t_hdoipd* handle);

#define hdoipd_report(h, ...) \
{ \
    t_hdoipd* _h_ = (h); \
    hdoipd_osd_activate(_h_); \
    fprintf(_h_->drv, __VA_ARGS__); \
}


#endif /* HDOIPD_OSD_H_ */
