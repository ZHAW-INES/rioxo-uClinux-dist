#ifndef RTSP_PARAMETER_H_
#define RTSP_PARAMETER_H_

#include <stdlib.h>

#include "rtsp_connection.h"
#include "rtsp_media.h"

int rtsp_get_parameter(t_rtsp_media *media, t_rtsp_connection *con, const char *name);

#endif /* RTSP_PARAMETER_H_ */
