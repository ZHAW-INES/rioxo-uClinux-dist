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

#include "hdoipd.h"

int check_certificate(char* certificate, int* repeater, char* km, char* enc_km);
int hdcp_ske_s(t_rscp_media* media, t_rscp_req_hdcp* m, t_rscp_connection* rsp);
int generate_rtx(char* rtx);
int get_hdcp_status();
int hdcp_decrypt_flash_keys();
int hdcp_generate_random_nr(char* number);
int hdcp_check_certificate(char* certificate, uint32_t* repeater, char* km, char* enc_km);
int hdcp_calculate_h(char* rtx, uint32_t* repeater, char* H, char* km, char* kd);
int hdcp_calculate_l(char* rn, char* rrx, char* kd, char* L);
int hdcp_convert_sk_char_int(char* ks, char* riv, uint32_t* keys);
int hdcp_ske_dec_ks(char* rn, char* ks, char* Edkey_ks, char* rtx, char* rrx, char* km);
int hdcp_ske_enc_ks(char* rn, char* km, char* rrx, char* rtx, char* ks, char* Edkey_ks);

#endif /* HDCP_H_ */
