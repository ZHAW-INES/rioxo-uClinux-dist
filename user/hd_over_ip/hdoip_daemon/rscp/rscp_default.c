/*
 * rscp_default.c
 *
 *  Created on: 23.11.2010
 *      Author: alda
 */

#include "rscp_default.h"

void rscp_default_transport(t_rscp_transport* t)
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
    t->mode = RSCP_METHOD_PLAY;
}

void rscp_default_response_setup(t_rscp_rsp_setup* p)
{
    p->cseq = 0;
    p->session[0] = 0;
    rscp_default_transport(&p->transport);
    p->hdcp.hdcp_on = 0;
    p->hdcp.port_nr = 55000;

}

void rscp_default_response_play(t_rscp_rsp_play* p)
{
    p->cseq = 0;
    p->session[0] = 0;
}

void rscp_default_response_teardown(t_rscp_rsp_teardown* p)
{
    p->cseq = 0;
    p->session[0] = 0;
}

