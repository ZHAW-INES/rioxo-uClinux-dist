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
        MAP_FNC_NULL
};

// PLAY attributes
const t_map_fnc tab_request_play[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_play, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_req_play, session) },
        { "RTP-Format", rtsp_parse_rtp_format, offsetof(t_rtsp_req_play, format) },
        MAP_FNC_NULL
};

// TEARDOWN attributes
const t_map_fnc tab_request_teardown[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_req_teardown, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_req_teardown, session) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_setup[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_rsp_setup, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_rsp_setup, session) },
        { "Transport", rtsp_parse_transport, offsetof(t_rtsp_rsp_setup, transport) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_play[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_rsp_play, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_rsp_play, session) },
        { "Timing", rtsp_parse_timing, offsetof(t_rtsp_rsp_play, timing) },
        { "RTP-Format", rtsp_parse_rtp_format, offsetof(t_rtsp_rsp_play, format) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_teardown[] ={
        { "CSeq", rtsp_parse_ui32, offsetof(t_rtsp_rsp_teardown, cseq) },
        { "Session", rtsp_parse_str, offsetof(t_rtsp_rsp_teardown, session) },
        MAP_FNC_NULL
};

// the methodes
const t_map_set srv_method[] = {
    { "SETUP", tab_request_setup, offsetof(u_rtsp_request, setup), rms_setup },
    { "PLAY", tab_request_play, offsetof(u_rtsp_request, play), rms_play },
    { "TEARDOWN", tab_request_teardown, offsetof(u_rtsp_request, teardown), rms_teardown },
    { 0, 0, 0, 0 }
};
