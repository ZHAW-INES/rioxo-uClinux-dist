/*
 * hdoipd_callback.h
 *
 *  Created on: 20.01.2011
 *      Author: alda
 */

#ifndef HDOIPD_CALLBACK_H_
#define HDOIPD_CALLBACK_H_

#include "rtsp_media.h"

int hdoipd_media_callback(t_rtsp_media* media, int (*f)(t_rtsp_media*, void*), void* d);

#endif /* HDOIPD_CALLBACK_H_ */
