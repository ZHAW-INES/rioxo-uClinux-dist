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
    RTSP_CLOSE = 1,
    RTSP_PAUSE,
    RTSP_CLIENT_ERROR,
    RTSP_SERVER_ERROR,
    RTSP_PARSE_ERROR,
    RTSP_VERSION_ERROR,
    RTSP_RESPONSE_ERROR,
    RTSP_REQUEST_ERROR,
    RTSP_UNSUPPORTED_METHOD,
    RTSP_UNKNOWN_HEADER,
    RTSP_NULL_POINTER,
    RTSP_TIMEOUT,
    RTSP_HDCP_ERROR,
    RTSP_WRONG_STATE
} e_error;

#endif /* RTSP_ERROR_H_ */
