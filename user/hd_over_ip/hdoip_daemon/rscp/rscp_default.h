/*
 * rscp_default.h
 *
 *  Created on: 24.11.2010
 *      Author: alda
 */

#ifndef RSCP_DEFAULT_H_
#define RSCP_DEFAULT_H_

#include "rscp.h"

void rscp_default_transport(t_rscp_transport* t);

void rscp_default_response_setup(t_rscp_rsp_setup* p);
void rscp_default_response_play(t_rscp_rsp_play* p);
void rscp_default_response_teardown(t_rscp_rsp_teardown* p);

#endif /* RSCP_DEFAULT_H_ */
