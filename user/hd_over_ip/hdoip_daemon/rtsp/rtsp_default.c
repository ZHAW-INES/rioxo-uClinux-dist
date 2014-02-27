/*
 * rtsp_default.c
 *
 *  Created on: 23.11.2010
 *      Author: alda
 */

#include "rtsp_default.h"

void rtsp_default_transport(t_rtsp_transport* t)
{
    t->protocol = PROTOCOL_RTP;
    t->profile = PROFILE_AVP;
    t->lower = LOWER_UDP;
    t->multicast = false;
    t->destination = 0;
    t->port = PORT_RANGE(0, 0);
    t->client_port = PORT_RANGE(0, 0);
    t->server_port = PORT_RANGE(0, 0);
}

void rtsp_default_response(t_rtsp_rsp_setup* p)
{
    p->cseq = 0;
}

void rtsp_default_response_session(t_rtsp_rsp_setup* p)
{
    rtsp_default_response(p);
    p->session[0] = 0;
}

void rtsp_default_response_setup(t_rtsp_rsp_setup* p)
{
    rtsp_default_response_session(p);
    rtsp_default_transport(&p->transport);
    p->hdcp.hdcp_on = 0;
}

void rtsp_default_response_hdcp(t_rtsp_rsp_hdcp* p)
{
    rtsp_default_response_session(p);
    p->id[0] =0;
    p->content[0]= 0;
}

void rtsp_default_response_play(t_rtsp_rsp_play* p)
{
    rtsp_default_response_session(p);
}

void rtsp_default_response_pause(t_rtsp_rsp_pause* p)
{
    rtsp_default_response_session(p);
}

void rtsp_default_response_teardown(t_rtsp_rsp_teardown* p)
{
    rtsp_default_response_session(p);
}

