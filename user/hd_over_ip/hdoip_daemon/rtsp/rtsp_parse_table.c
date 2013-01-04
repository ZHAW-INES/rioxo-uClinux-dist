/*
 * rtsp_parse_table.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "rtsp.h"
#include "rtsp_media.h"
#include "rtsp_parse_header.h"
#include "rtsp_server.h"

// OPTIONS attributes
const t_map_fnc tab_request_options[] = {
    { "CSeq",   rtsp_parse_ui32,    true,   offsetof(t_rtsp_req_options, cseq) },
    MAP_FNC_NULL
};

// GET_PARAMETER attributes
const t_map_fnc tab_request_get_parameter[] = {
    { "CSeq",   rtsp_parse_ui32,    true,   offsetof(t_rtsp_req_get_parameter, cseq) },
    { "Session",rtsp_parse_str,     false,  offsetof(t_rtsp_req_get_parameter, session) },
    MAP_FNC_NULL
};

const t_map_fnc tab_response_get_parameter[] = {
    { "CSeq",   rtsp_parse_ui32,    true,   offsetof(t_rtsp_req_get_parameter, cseq) },
    { "Session",rtsp_parse_str,     false,  offsetof(t_rtsp_req_get_parameter, session) },
    MAP_FNC_NULL
};

// SET_PARAMETER attributes
const t_map_fnc tab_request_set_parameter[] = {
    { "CSeq",   rtsp_parse_ui32,    true,   offsetof(t_rtsp_req_set_parameter, cseq) },
    { "Session",rtsp_parse_str,     false,  offsetof(t_rtsp_req_set_parameter, session) },
    MAP_FNC_NULL
};

// SETUP attributes
const t_map_fnc tab_request_setup[] = {
	{ "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_req_setup, cseq) },
	{ "Transport",	rtsp_parse_transport,	true,	offsetof(t_rtsp_req_setup, transport) },
	{ "EDID-Segment",rtsp_parse_edid,	false,	offsetof(t_rtsp_req_setup, edid) },
    { "HDCP",	rtsp_parse_hdcp,	true,	offsetof(t_rtsp_req_setup, hdcp) },
	MAP_FNC_NULL
};

const t_map_fnc tab_response_setup[] = {
	{ "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_rsp_setup, cseq) },
	{ "Session",	rtsp_parse_str,		true,	offsetof(t_rtsp_rsp_setup, session) },
	{ "Transport",	rtsp_parse_transport,	true,	offsetof(t_rtsp_rsp_setup, transport) },
	{ "HDCP",	rtsp_parse_hdcp,	true,	offsetof(t_rtsp_rsp_setup, hdcp) },
	MAP_FNC_NULL
};

// HDCP attributes
const t_map_fnc tab_request_hdcp[] = {
	{ "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_req_hdcp, cseq) },
	{ "Session",	rtsp_parse_str,		true,	offsetof(t_rtsp_req_hdcp, session) },
	{ "ID",		rtsp_parse_str,		true,	offsetof(t_rtsp_req_hdcp, id) },
	{ "Content",	rtsp_parse_str,		true,	offsetof(t_rtsp_req_hdcp, content) },
	MAP_FNC_NULL
};

// PLAY attributes
const t_map_fnc tab_request_play[] = {
	{ "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_req_play, cseq) },
	{ "Session",	rtsp_parse_str,		true,	offsetof(t_rtsp_req_play, session) },
	{ "RTP-Format",	rtsp_parse_rtp_format,	true,	offsetof(t_rtsp_req_play, format) },
	{ "USB",	rtsp_parse_usb,		true,	offsetof(t_rtsp_req_play, usb) },
	MAP_FNC_NULL
};

const t_map_fnc tab_response_play[] = {
	{ "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_rsp_play, cseq) },
	{ "Session",	rtsp_parse_str,		true,	offsetof(t_rtsp_rsp_play, session) },
	{ "Timing",	rtsp_parse_timing,	true,	offsetof(t_rtsp_rsp_play, timing) },
	{ "RTP-Format", rtsp_parse_rtp_format,	true,	offsetof(t_rtsp_rsp_play, format) },
	MAP_FNC_NULL
};

// TEARDOWN attributes
const t_map_fnc tab_request_teardown[] = {
	{ "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_req_teardown, cseq) },
	{ "Session",	 rtsp_parse_str,	true,	offsetof(t_rtsp_req_teardown, session) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_teardown[] ={
        { "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_rsp_teardown, cseq) },
        { "Session",	rtsp_parse_str,		true,	offsetof(t_rtsp_rsp_teardown, session) },
        MAP_FNC_NULL
};

// HDCP attributes
const t_map_fnc tab_response_hdcp[] ={
        { "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_rsp_hdcp, cseq) },
        { "Session",	rtsp_parse_str, 	true,	offsetof(t_rtsp_rsp_hdcp, session) },
        { "ID",		rtsp_parse_str, 	true,	offsetof(t_rtsp_rsp_hdcp, id) },
        { "Content",	rtsp_parse_str,		true,	offsetof(t_rtsp_rsp_hdcp, content) },
        MAP_FNC_NULL
};

// UPDATE attributes
const t_map_fnc tab_request_update[] ={
        { "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_req_update, cseq) },
        { "Session",	rtsp_parse_str,		true,	offsetof(t_rtsp_req_update, session) },
        { "Event",	rtsp_parse_uint32,	true,	offsetof(t_rtsp_req_update, event) },
        { "RTP-Format",	rtsp_parse_rtp_format,	true,	offsetof(t_rtsp_req_update, format) },
        MAP_FNC_NULL
};

// PAUSE attributes
const t_map_fnc tab_request_pause[] ={
        { "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_req_pause, cseq) },
        { "Session",rtsp_parse_str,		true,	offsetof(t_rtsp_req_pause, session) },
        MAP_FNC_NULL
};

const t_map_fnc tab_response_pause[] ={
        { "CSeq",	rtsp_parse_ui32,	true,	offsetof(t_rtsp_req_pause, cseq) },
        { "Session",	rtsp_parse_str,		true,	offsetof(t_rtsp_req_pause, session) },
        MAP_FNC_NULL
};

// the methods
const t_map_set rtsp_srv_methods[] = {
    { "OPTIONS",        tab_request_options,        rmsq_options,       false,  RTSP_STATE_ALL,                         offsetof(t_rtsp_media, options)         },
    { "GET_PARAMETER",  tab_request_get_parameter,  rmsq_get_parameter, false,  RTSP_STATE_ALL,                         offsetof(t_rtsp_media, get_parameter)   },
    { "SET_PARAMETER",  tab_request_set_parameter,  rmsq_set_parameter, false,  RTSP_STATE_ALL,                         offsetof(t_rtsp_media, set_parameter)   },
    { "SETUP",          tab_request_setup,          rmsq_setup,         false,  RTSP_STATE_ALL,                         offsetof(t_rtsp_media, setup)           },
    { "HDCP",           tab_request_hdcp,           rmsq_hdcp,          true,   RTSP_STATE_ALL,                         offsetof(t_rtsp_media, hdcp)            },
    { "PLAY",           tab_request_play,           rmsq_play,          true,   RTSP_STATE_READY | RTSP_STATE_PLAYING,  offsetof(t_rtsp_media, play)            },
    { "PAUSE",          tab_request_pause,          rmsq_pause,         true,   RTSP_STATE_PLAYING,                     offsetof(t_rtsp_media, pause)           },
    { "TEARDOWN",       tab_request_teardown,       rmsq_teardown,      true,   RTSP_STATE_ALL,                         offsetof(t_rtsp_media, teardown)        },
    { "UPDATE",         tab_request_update,         rmsq_update,        true,   RTSP_STATE_ALL,                         offsetof(t_rtsp_media, update)          },
    MAP_SET_NULL
};

// the methods
const t_map_set rtsp_client_methods[] = {
    { "OPTIONS",        tab_request_options,        rmcq_options,       false,  RTSP_STATE_ALL,                         offsetof(t_rtsp_media, options)   },
    { "UPDATE",         tab_request_update,         rmcq_update,        true,   RTSP_STATE_ALL,                         offsetof(t_rtsp_media, update)    },
    { "PAUSE",          tab_request_pause,          rmcq_pause,         true,   RTSP_STATE_PLAYING,                     offsetof(t_rtsp_media, pause)     },
    { "TEARDOWN",       tab_request_teardown,       rmcq_teardown,      true,   RTSP_STATE_ALL,                         offsetof(t_rtsp_media, teardown)  },
    MAP_SET_NULL
};
