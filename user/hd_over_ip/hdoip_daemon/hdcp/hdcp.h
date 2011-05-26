/*
 * hdcp.h
 *
 *  Created on: 28.04.2011
 *      Author: itin
 */
#ifndef HDCP_H_
#define HDCP_H_

#define LENGHT_CERTRX 1044
#define LENGHT_PRIVATE_KEY 128
#define LENGHT_LC128 32
#define LENGHT_OFFSET 8

#include "rscp.h"
#include "rscp_net.h"
#include "hdoipd.h"
#include "rscp_error.h"
#include "hdoipd_fsm.h"
#include "rsaes-oaep/rsaes.h"
#include "rsassa_pkcs1/rsassa.h"
#include "aes128/aes_encrypt.h"
#include "sha256/sha256.h"
#include "protocol/protocol.h"
#include <debug.h>


int check_certificate(char* certificate, int* repeater, char* km, char* enc_km);
int hdcp_ske_s(t_rscp_media* media, t_rscp_req_hdcp* m, t_rscp_connection* rsp, int* timeout);
int generate_rtx(char* rtx);
int hdcp_open_socket(t_rscp_hdcp* m, int* sockfd, struct sockaddr_in* cli_addr );
int hdcp_ske_server(t_rscp_hdcp* m, int* sockfd, struct sockaddr_in* cli_addr, char* media_type);
int hdcp_ske_client(t_rscp_hdcp* m, char* media_type);
int get_hdcp_status();
int hdcp_decrypt_flash_keys();

#endif /* HDCP_H_ */
