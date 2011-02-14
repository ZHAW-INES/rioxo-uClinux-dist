/*
 * rscp_error.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RSCP_ERROR_H_
#define RSCP_ERROR_H_

typedef enum {
    RSCP_ERRORNO =-1,           //!< general system error -> see errorno
    RSCP_SUCCESS = 0,
    RSCP_CLIENT_ERROR,
    RSCP_SERVER_ERROR,
    RSCP_PARSE_ERROR,
    RSCP_VERSION_ERROR,
    RSCP_RESPONSE_ERROR,
    RSCP_REQUEST_ERROR,
    RSCP_UNSUPPORTED_METHOD,
    RSCP_UNKNOWN_HEADER,
    RSCP_NULL_POINTER
} e_error;

#endif /* RSCP_ERROR_H_ */
