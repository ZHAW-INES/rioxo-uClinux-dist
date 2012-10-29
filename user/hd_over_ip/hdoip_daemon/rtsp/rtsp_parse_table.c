/*
 * rtsp_parse_table.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

/*
 * rtsp_table.c
 *
 *  Created on: 17.11.2010
 *      Author: alda
 */

#include "rtsp_server.h"
#include "rtsp_parse_header.h"
#include "rtsp.h"
#include "rtsp_media.h"


// OPTION attributes
const t_map_fnc tab_request_option[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_option, cseq) },
        MAP_FNC_NULL
};

// SETUP attributes
const t_map_fnc tab_request_setup[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_setup, cseq) },
        { "Transport", rtsp_parse_transport, offsetof(t_rtsp_req_setup, transport) },
        { "EDID-Segment", rtsp_parse_edid,  offsetof(t_rtsp_req_setup, edid) },
        { "HDCP", rtsp_parse_hdcp,  offsetof(t_rtsp_req_setup, hdcp) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_setup[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_rsp_setup, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_rsp_setup, session) },
        { "Transport", rtsp_parse_transport, offsetof(t_rtsp_rsp_setup, transport) },
        { "HDCP", rtsp_parse_hdcp,  offsetof(t_rtsp_rsp_setup, hdcp) },
        MAP_FNC_NULL
};

// HDCP attributes
const t_map_fnc tab_request_hdcp[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_hdcp, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_req_hdcp, session) },
        { "ID", rtsp_parse_str, offsetof(t_rtsp_req_hdcp, id) },
        { "Content", rtsp_parse_str, offsetof(t_rtsp_req_hdcp, content) },
        MAP_FNC_NULL
};

// PLAY attributes
const t_map_fnc tab_request_play[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_play, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_req_play, session) },
        { "RTP-Format", rtsp_parse_rtp_format, offsetof(t_rtsp_req_play, format) },
        { "USB", rtsp_parse_usb,  offsetof(t_rtsp_req_play, usb) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_play[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_rsp_play, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_rsp_play, session) },
        { "Timing", rtsp_parse_timing, offsetof(t_rtsp_rsp_play, timing) },
        { "RTP-Format", rtsp_parse_rtp_format, offsetof(t_rtsp_rsp_play, format) },
        MAP_FNC_NULL
};

// TEARDOWN attributes
const t_map_fnc tab_request_teardown[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_teardown, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_req_teardown, session) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_teardown[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_rsp_teardown, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_rsp_teardown, session) },
        MAP_FNC_NULL
};

// HDCP attributes
const t_map_fnc tab_response_hdcp[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_rsp_hdcp, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_rsp_hdcp, session) },
        { "ID", rtsp_parse_str, offsetof(t_rtsp_rsp_hdcp, id) },
        { "Content", rtsp_parse_str, offsetof(t_rtsp_rsp_hdcp, content) },
        MAP_FNC_NULL
};


// UPDATE attributes
const t_map_fnc tab_request_update[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_update, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_req_update, session) },
        { "Event", rtsp_parse_uint32, offsetof(t_rtsp_req_update, event) },
        { "RTP-Format", rtsp_parse_rtp_format, offsetof(t_rtsp_req_update, format) },
        MAP_FNC_NULL
};

// PAUSE attributes
const t_map_fnc tab_request_pause[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_update, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_req_update, session) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_pause[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_pause, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_req_pause, session) },
        MAP_FNC_NULL
};

// HELLO attributes
const t_map_fnc tab_request_hello[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_update, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_req_update, session) },
        MAP_FNC_NULL
};

// the methodes
const t_map_set srv_method[] = {
    { "SETUP", (void*)tab_request_setup, (void*)rmsq_setup },
    { "HDCP", (void*)tab_request_hdcp, (void*)rmsq_hdcp },
    { "PLAY", (void*)tab_request_play, (void*)rmsq_play },
    { "PAUSE", (void*)tab_request_pause, (void*)rmsq_pause },
    { "TEARDOWN", (void*)tab_request_teardown, (void*)rmsq_teardown },
    { "HELLO", (void*)tab_request_hello, (void*)rmsq_hello },
    { "UPDATE", (void*)tab_request_update, (void*)rmsq_update },
    MAP_SET_NULL
};

// the methodes
const t_map_set client_method[] = {
    { "UPDATE", (void*)tab_request_update, (void*)rmcq_update },
    { "PAUSE", (void*)tab_request_pause, (void*)rmcq_pause },
    { "TEARDOWN", (void*)tab_request_teardown, (void*)rmcq_teardown },
    MAP_SET_NULL
};
