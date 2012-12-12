/*
 * rtsp.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_H_
#define RTSP_H_

#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "hoi_msg.h"

//------------------------------------------------------------------------------
// Constants

#define RTSP_VERSION                "RTSP/1.0"
#define RTSP_SCHEME                 "rtsp"


//------------------------------------------------------------------------------
// Status Codes

typedef enum {
  RTSP_STATUS_UNKNOWN                           =   0,
  RTSP_STATUS_CONTINUE                          = 100,
  RTSP_STATUS_OK                                = 200,
  RTSP_STATUS_CREATED                           = 201,
  RTSP_STATUS_LOW_ON_STORAGE_SPACE              = 250,
  RTSP_STATUS_MULTIPLE_CHOICES                  = 300,
  RTSP_STATUS_MOVED_PERMANENTLY                 = 301,
  RTSP_STATUS_MOVED_TEMPORARILY                 = 302,
  RTSP_STATUS_SEE_OTHER                         = 303,
  RTSP_STATUS_USE_PROXY                         = 305,
  RTSP_STATUS_BAD_REQUEST                       = 400,
  RTSP_STATUS_UNAUTHORIZED                      = 401,
  RTSP_STATUS_PAYMENT_REQUIRED                  = 402,
  RTSP_STATUS_FORBIDDEN                         = 403,
  RTSP_STATUS_NOT_FOUND                         = 404,
  RTSP_STATUS_METHOD_NOT_ALLOWED                = 405,
  RTSP_STATUS_NOT_ACCEPTABLE                    = 406,
  RTSP_STATUS_PROXY_AUTHENTICATION_REQUIRED     = 407,
  RTSP_STATUS_REQUEST_TIMEOUT                   = 408,
  RTSP_STATUS_GONE                              = 410,
  RTSP_STATUS_LENGTH_REQUIRED                   = 411,
  RTSP_STATUS_PRECONDITION_FAILED               = 412,
  RTSP_STATUS_REQUEST_ENTITY_TOO_LARGE          = 413,
  RTSP_STATUS_REQUEST_URI_TOO_LONG              = 414,
  RTSP_STATUS_UNSUPPORTED_MEDIA_TYPE            = 415,
  RTSP_STATUS_INVALID_PARAMETER                 = 451,
  RTSP_STATUS_ILLEGAL_CONFERENCE_IDENTIFIER     = 452,
  RTSP_STATUS_NOT_ENOUGH_BANDWIDTH              = 453,
  RTSP_STATUS_SESSION_NOT_FOUND                 = 454,
  RTSP_STATUS_METHOD_NOT_VALID_IN_THIS_STATE    = 455,
  RTSP_STATUS_HEADER_FIELD_NOT_VALID            = 456,
  RTSP_STATUS_INVALID_RANGE                     = 457,
  RTSP_STATUS_PARAMETER_IS_READ_ONLY            = 458,
  RTSP_STATUS_AGGREGATE_OPERATION_NOT_ALLOWED   = 459,
  RTSP_STATUS_ONLY_AGGREGATE_OPERATION_ALLOWED  = 460,
  RTSP_STATUS_UNSUPPORTED_TRANSPORT             = 461,
  RTSP_STATUS_DESTINATION_UNREACHABLE           = 462,
  RTSP_STATUS_INTERNAL_SERVER_ERROR             = 500,
  RTSP_STATUS_NOT_IMPLEMENTED                   = 501,
  RTSP_STATUS_BAD_GATEWAY                       = 502,
  RTSP_STATUS_SERVICE_UNAVAILABLE               = 503,
  RTSP_STATUS_GATEWAY_TIMEOUT                   = 504,
  RTSP_STATUS_RTSP_VERSION_NOT_SUPPORTED        = 505,
  RTSP_STATUS_OPTION_NOT_SUPPORTED              = 551
} e_rtsp_status_code;

//------------------------------------------------------------------------------
// Macros

#define PORT_RANGE(x, y)            (((x)&0xffff)|(((y)&0xffff)<<16))
#define PORT_RANGE_START(x)         ((x)&0xffff)
#define PORT_RANGE_STOP(x)          (((x)>>16)&0xffff)


//------------------------------------------------------------------------------
// Enumeration

typedef enum {
    PROTOCOL_RTP = 0,
    PROTOCOL_USB
} e_rtsp_protocol;

// profile values
typedef enum {
    PROFILE_RAW = 0,
    PROFILE_AVP
} e_rtsp_profile;

// lower transport values
typedef enum {
    LOWER_UDP = 0,
    LOWER_TCP
} e_rtsp_lower;

// transport format values
typedef enum {
    FORMAT_PLAIN = 0,
    FORMAT_JPEG2000
} e_rtsp_rtp_format;


//------------------------------------------------------------------------------
// Header fields

// transport field
typedef struct {
    uint32_t            protocol;                   //!< "RTP" or "USB"
    uint32_t            profile;                    //!< "AVP" or "RAW"
    uint32_t            lower;                      //!< "UDP" or "TCP"
    bool                multicast;
    uint32_t            destination;
    char                destination_str[50];
    uint32_t            port;                       //!< low 16 Bit: port range start; high 16 Bit: port range stop
    uint32_t            client_port;
    uint32_t            server_port;
} t_rtsp_transport;

typedef struct {
    char                url[200];
    uint32_t            seq;
    uint32_t            rtptime;
} t_rtsp_rtp_info;

// own header...
typedef struct {
    uint32_t            compress;
    uint32_t            value;
    uint32_t            value2;
    uint32_t            rtptime;
} t_rtsp_rtp_format;

typedef struct {
    uint8_t             segment;
    uint8_t             edid[256];
    bool                from_header;
} t_rtsp_edid;

typedef struct {
  uint32_t            timeout;
  int                 alive_ping;
} t_rtsp_timeout;

// hdcp status and port number
typedef struct {
    uint32_t            hdcp_on; //on=1, off=0
} t_rtsp_hdcp;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    char                keyboard[20];
    char                mouse[20];
    char                storage[20];
} t_rtsp_usb;

//------------------------------------------------------------------------------
// Requests

typedef struct {
    uint32_t            cseq;
} t_rtsp_req_options;

typedef struct {
    uint32_t            cseq;
} t_rtsp_req_get_parameter;

typedef struct {
    uint32_t            cseq;
    t_rtsp_transport    transport;
    t_rtsp_edid         edid;
    t_rtsp_hdcp         hdcp;
    t_rtsp_usb          usb;
} t_rtsp_req_setup;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    char				id[10];
    char 				content[1200];
} t_rtsp_req_hdcp;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    t_rtsp_rtp_format   format;
    t_rtsp_usb          usb;
} t_rtsp_req_play;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    t_rtsp_usb          usb;
} t_rtsp_req_teardown;

typedef struct {
    uint32_t            cseq;
    char                session[50];
} t_rtsp_req_hello;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    uint32_t            event;
    t_rtsp_rtp_format   format;
} t_rtsp_req_update;

typedef struct {
    uint32_t            cseq;
    char                session[50];
} t_rtsp_req_pause;


//------------------------------------------------------------------------------
// Response

typedef struct {
    uint32_t            cseq;
    char                session[50];
    t_rtsp_transport    transport;
    t_rtsp_hdcp			hdcp;
} t_rtsp_rsp_setup;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    char				id[10];
    char 				content[1200];
} t_rtsp_rsp_hdcp;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    t_video_timing      timing;
    t_rtsp_rtp_format   format;
} t_rtsp_rsp_play;

typedef struct {
    uint32_t            cseq;
    char                session[50];
} t_rtsp_rsp_teardown;

typedef struct {
    uint32_t            cseq;
    char                session[50];
} t_rtsp_rsp_pause;

typedef union {
    t_rtsp_req_options  req_options;
    t_rtsp_req_setup    req_setup;
    t_rtsp_req_hdcp     req_hdcp;
    t_rtsp_req_play     req_play;
    t_rtsp_req_teardown req_teardown;
    t_rtsp_req_hello    req_hello;
    t_rtsp_req_update   req_update;
    t_rtsp_req_update   req_pause;
    t_rtsp_rsp_setup    rsp_setup;
    t_rtsp_rsp_play     rsp_play;
    t_rtsp_rsp_pause    rsp_pause;
    t_rtsp_rsp_teardown rsp_teardown;
} u_rtsp_header;

#endif /* RTSP_H_ */
