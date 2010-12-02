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
    t->append = false;
    t->ttl = 0;
    t->layers = 0;
    t->port = PORT_RANGE(0, 0);
    t->client_port = PORT_RANGE(0, 0);
    t->server_port = PORT_RANGE(0, 0);
    t->ssrc = 0;
    t->mode = RTSP_METHOD_PLAY;
}

void rtsp_default_response_setup(t_rtsp_rsp_setup* p)
{
    p->cseq = 0;
    p->session[0] = 0;
    rtsp_default_transport(&p->transport);
}

void rtsp_default_response_play(t_rtsp_rsp_play* p)
{
    p->cseq = 0;
    p->session[0] = 0;
}

