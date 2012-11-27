#ifndef RTSP_PARAMETER_H_
#define RTSP_PARAMETER_H_

#include <stdlib.h>

#include "rtsp_connection.h"

typedef int (f_rtsp_param_get)(t_rtsp_connection *con);
typedef int (f_rtsp_param_set)(t_rtsp_connection *con, char *buf, size_t n);

typedef struct rtsp_parameter {
    const char *name;       // name
    f_rtsp_param_get *get;  // getter
    f_rtsp_param_set *set;  // setter, NULL for readonly parameters
} t_rtsp_parameter;

t_rtsp_parameter *rtsp_parameter_lookup(const char *name);

#endif /* RTSP_PARAMETER_H_ */
