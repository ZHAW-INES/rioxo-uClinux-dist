/*
 * rtsp.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_H_
#define RTSP_H_

#include <stdio.h>
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

#define RTSP_SC_CONTINUE                (100)
#define RTSP_SC_OK                      (200)
#define RTSP_SC_MULTIPLE_CHOICES        (300)
#define RTSP_SC_BAD_REQUEST             (400)
#define RTSP_SC_METHOD_NVIS             (455)
#define RTSP_SC_SERVER_ERROR            (500)

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
// Set

typedef uint32_t s_rtsp_method;
#define RTSP_METHOD_OPTIONS         (0x0001)
#define RTSP_METHOD_SETUP           (0x0002)
#define RTSP_METHOD_PLAY            (0x0004)
#define RTSP_METHOD_TEARDOWN        (0x0008)


//------------------------------------------------------------------------------
// Header fields

// transport field
typedef struct {
    uint32_t            protocol;                   //!< "RTP"
    uint32_t            profile;                    //!< "AVP"
    uint32_t            lower;                      //!< "UDP"
    bool                multicast;
    uint32_t            destination;
    char                destination_str[50];
    bool                append;
    uint32_t            ttl;
    uint32_t            layers;
    uint32_t            port;                       //!< low 16 Bit: port range start; high 16 Bit: port range stop
    uint32_t            client_port;
    uint32_t            server_port;
    uint32_t            ssrc;
    uint32_t            mode;
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
} t_rtsp_edid;

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
} t_rtsp_req_option;

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
    char				device[50];
    char                type[50];
    char                session[50];
} t_rtsp_req_usb;

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
} t_rtsp_rsp_option;

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
    t_rtsp_req_option   req_option;
    t_rtsp_req_setup    req_setup;
    t_rtsp_req_hdcp     req_hdcp;
    t_rtsp_req_play     req_play;
    t_rtsp_req_teardown req_teardown;
    t_rtsp_req_hello    req_hello;
    t_rtsp_req_usb      req_usb;
    t_rtsp_req_update   req_update;
    t_rtsp_req_update   req_pause;
    t_rtsp_rsp_option   rsp_option;
    t_rtsp_rsp_setup    rsp_setup;
    t_rtsp_rsp_play     rsp_play;
    t_rtsp_rsp_pause    rsp_pause;
    t_rtsp_rsp_teardown rsp_teardown;
} u_rtsp_header;


#endif /* RTSP_H_ */
