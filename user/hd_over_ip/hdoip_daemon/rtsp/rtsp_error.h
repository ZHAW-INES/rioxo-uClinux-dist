/*
 * rtsp_error.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_ERROR_H_
#define RTSP_ERROR_H_

typedef enum {
    RTSP_ERRORNO =-1,           //!< general system error -> see errorno
    RTSP_SUCCESS = 0,
    RTSP_CLIENT_ERROR,
    RTSP_SERVER_ERROR,
    RTSP_RESOURCE_ERROR,
    RTSP_PARSE_ERROR,
    RTSP_VERSION_ERROR,
    RTSP_RESPONSE_ERROR,
    RTSP_SHUTDOWN
} e_error;

#endif /* RTSP_ERROR_H_ */
