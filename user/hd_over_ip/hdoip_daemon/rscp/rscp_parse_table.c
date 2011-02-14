/*
 * rscp_parse_table.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

/*
 * rscp_table.c
 *
 *  Created on: 17.11.2010
 *      Author: alda
 */

#include "rscp_server.h"
#include "rscp_parse_header.h"
#include "rscp.h"
#include "rscp_media.h"


// OPTION attributes
const t_map_fnc tab_request_option[] ={
        { "CSeq", rscp_parse_ui32, offsetof(t_rscp_req_option, cseq) },
        MAP_FNC_NULL
};

// SETUP attributes
const t_map_fnc tab_request_setup[] ={
        { "CSeq", rscp_parse_ui32, offsetof(t_rscp_req_setup, cseq) },
        { "Transport", rscp_parse_transport, offsetof(t_rscp_req_setup, transport) },
        { "EDID-Segment", rscp_parse_edid,  offsetof(t_rscp_req_setup, edid) },
        MAP_FNC_NULL
};

// PLAY attributes
const t_map_fnc tab_request_play[] ={
        { "CSeq", rscp_parse_ui32, offsetof(t_rscp_req_play, cseq) },
        { "Session", rscp_parse_str, offsetof(t_rscp_req_play, session) },
        { "RTP-Format", rscp_parse_rtp_format, offsetof(t_rscp_req_play, format) },
        MAP_FNC_NULL
};

// TEARDOWN attributes
const t_map_fnc tab_request_teardown[] ={
        { "CSeq", rscp_parse_ui32, offsetof(t_rscp_req_teardown, cseq) },
        { "Session", rscp_parse_str, offsetof(t_rscp_req_teardown, session) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_setup[] ={
        { "CSeq", rscp_parse_ui32, offsetof(t_rscp_rsp_setup, cseq) },
        { "Session", rscp_parse_str, offsetof(t_rscp_rsp_setup, session) },
        { "Transport", rscp_parse_transport, offsetof(t_rscp_rsp_setup, transport) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_play[] ={
        { "CSeq", rscp_parse_ui32, offsetof(t_rscp_rsp_play, cseq) },
        { "Session", rscp_parse_str, offsetof(t_rscp_rsp_play, session) },
        { "Timing", rscp_parse_timing, offsetof(t_rscp_rsp_play, timing) },
        { "RTP-Format", rscp_parse_rtp_format, offsetof(t_rscp_rsp_play, format) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_teardown[] ={
        { "CSeq", rscp_parse_ui32, offsetof(t_rscp_rsp_teardown, cseq) },
        { "Session", rscp_parse_str, offsetof(t_rscp_rsp_teardown, session) },
        MAP_FNC_NULL
};

// UPDATE attributes
const t_map_fnc tab_request_update[] ={
        { "CSeq", rscp_parse_ui32, offsetof(t_rscp_req_update, cseq) },
        { "Session", rscp_parse_str, offsetof(t_rscp_req_update, session) },
        { "Event", rscp_parse_uint32, offsetof(t_rscp_req_update, event) },
        MAP_FNC_NULL
};

// PAUSE attributes
const t_map_fnc tab_request_pause[] ={
        { "CSeq", rscp_parse_ui32, offsetof(t_rscp_req_update, cseq) },
        { "Session", rscp_parse_str, offsetof(t_rscp_req_update, session) },
        MAP_FNC_NULL
};

// HELLO attributes
const t_map_fnc tab_request_hello[] ={
        { "CSeq", rscp_parse_ui32, offsetof(t_rscp_req_update, cseq) },
        { "Session", rscp_parse_str, offsetof(t_rscp_req_update, session) },
        MAP_FNC_NULL
};

// the methodes
const t_map_set srv_method[] = {
    { "SETUP", (void*)tab_request_setup, (void*)rmsq_setup },
    { "PLAY", (void*)tab_request_play, (void*)rmsq_play },
    { "TEARDOWN", (void*)tab_request_teardown, (void*)rmsq_teardown },
    { "HELLO", (void*)tab_request_hello, (void*)rmsq_hello },
    MAP_SET_NULL
};

// the methodes
const t_map_set client_method[] = {
    { "UPDATE", (void*)tab_request_update, (void*)rmcq_update },
    { "PAUSE", (void*)tab_request_pause, (void*)rmcq_pause },
    { "TEARDOWN", (void*)tab_request_teardown, (void*)rmcq_teardown },
    MAP_SET_NULL
};
