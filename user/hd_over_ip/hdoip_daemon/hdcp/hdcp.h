/*
 * hdcp.h
 *
 *  Created on: 28.04.2011
 *      Author: itin
 */

#ifndef HDCP_H_
#define HDCP_H_

#include "rscp.h"
#include "rscp_net.h"
#include "hdoipd.h"
#include "rscp_error.h"
#include "hdoipd_fsm.h"

int hdcp_open_socket(t_rscp_hdcp* m, int* sockfd, struct sockaddr_in* cli_addr );
int hdcp_ske_server(t_rscp_hdcp* m, int* sockfd, struct sockaddr_in* cli_addr, char* media_type);
int hdcp_ske_client(t_rscp_hdcp* m, char* media_type);
int get_hdcp_status();

#endif /* HDCP_H_ */
