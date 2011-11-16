/*
 * rscp.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RSCP_H_
#define RSCP_H_

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "hoi_msg.h"


//------------------------------------------------------------------------------
// Constants

#define RSCP_VERSION                "RSCP/1.0"
#define RSCP_SCHEME                 "rscp"


//------------------------------------------------------------------------------
// Status Codes

#define RSCP_SC_CONTINUE                (100)
#define RSCP_SC_OK                      (200)
#define RSCP_SC_MULTIPLE_CHOICES        (300)
#define RSCP_SC_BAD_REQUEST             (400)
#define RSCP_SC_METHOD_NVIS             (455)
#define RSCP_SC_SERVER_ERROR            (500)

//------------------------------------------------------------------------------
// Macros

#define PORT_RANGE(x, y)            (((x)&0xffff)|(((y)&0xffff)<<16))
#define PORT_RANGE_START(x)         ((x)&0xffff)
#define PORT_RANGE_STOP(x)          (((x)>>16)&0xffff)


//------------------------------------------------------------------------------
// Enumeration

typedef enum {
    PROTOCOL_RTP = 0
} e_rscp_protocol;

// profile values
typedef enum {
    PROFILE_AVP = 0
} e_rscp_profile;

// lower transport values
typedef enum {
    LOWER_UDP = 0
} e_rscp_lower;

// lower transport values
typedef enum {
    FORMAT_PLAIN = 0,
    FORMAT_JPEG2000
} e_rscp_rtp_format;

//------------------------------------------------------------------------------
// Set

typedef uint32_t s_rscp_method;
#define RSCP_METHOD_OPTIONS         (0x0001)
#define RSCP_METHOD_SETUP           (0x0002)
#define RSCP_METHOD_PLAY            (0x0004)
#define RSCP_METHOD_TEARDOWN        (0x0008)


//------------------------------------------------------------------------------
// Header fields

// transport field
typedef struct {
    uint32_t            protocol;                   //!< "RTP"
    uint32_t            profile;                    //!< "AVP"
    uint32_t            lower;                      //!< "UDP"
    bool                multicast;
    uint32_t            multicast_group;
    uint32_t            destination;
    bool                append;
    uint32_t            ttl;
    uint32_t            layers;
    uint32_t            port;                       //!< low 16 Bit: port range start; high 16 Bit: port range stop
    uint32_t            client_port;
    uint32_t            server_port;
    uint32_t            ssrc;
    uint32_t            mode;
} t_rscp_transport;

typedef struct {
    char                url[200];
    uint32_t            seq;
    uint32_t            rtptime;
} t_rscp_rtp_info;

// own header...
typedef struct {
    uint32_t            compress;
    uint32_t            value;
    uint32_t            value2;
    uint32_t            rtptime;
} t_rscp_rtp_format;

typedef struct {
    uint8_t             segment;
    uint8_t             edid[256];
} t_rscp_edid;

// hdcp status and port number
typedef struct {
    uint32_t            hdcp_on; //on=1, off=0
} t_rscp_hdcp;

//------------------------------------------------------------------------------
// Requests

typedef struct {
    uint32_t            cseq;
} t_rscp_req_option;

typedef struct {
    uint32_t            cseq;
    t_rscp_transport    transport;
    t_rscp_edid         edid;
    t_rscp_hdcp			hdcp;
} t_rscp_req_setup;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    char				id[10];
    char 				content[1200];
} t_rscp_req_hdcp;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    t_rscp_rtp_format   format;
} t_rscp_req_play;

typedef struct {
    uint32_t            cseq;
    char                session[50];
} t_rscp_req_teardown;

typedef struct {
    uint32_t            cseq;
    char                session[50];
} t_rscp_req_hello;

typedef struct {
    uint32_t            cseq;
    char				device[50];
    char                type[50];
    char                session[50];
} t_rscp_req_usb;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    uint32_t            event;
} t_rscp_req_update;

typedef struct {
    uint32_t            cseq;
    char                session[50];
} t_rscp_req_pause;


//------------------------------------------------------------------------------
// Response

typedef struct {
    uint32_t            cseq;
} t_rscp_rsp_option;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    t_rscp_transport    transport;
    t_rscp_hdcp			hdcp;
} t_rscp_rsp_setup;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    char				id[10];
    char 				content[1200];
} t_rscp_rsp_hdcp;

typedef struct {
    uint32_t            cseq;
    char                session[50];
    t_video_timing      timing;
    t_rscp_rtp_format   format;
} t_rscp_rsp_play;

typedef struct {
    uint32_t            cseq;
    char                session[50];
} t_rscp_rsp_teardown;


typedef union {
    t_rscp_req_option   req_option;
    t_rscp_req_setup    req_setup;
    t_rscp_req_hdcp     req_hdcp;
    t_rscp_req_play     req_play;
    t_rscp_req_teardown req_teardown;
    t_rscp_req_hello    req_hello;
    t_rscp_req_usb      req_usb;
    t_rscp_req_update   req_update;
    t_rscp_req_update   req_pause;
    t_rscp_rsp_option   rsp_option;
    t_rscp_rsp_setup    rsp_setup;
    t_rscp_rsp_play     rsp_play;
    t_rscp_rsp_teardown rsp_teardown;
} u_rscp_header;


#endif /* RSCP_H_ */
