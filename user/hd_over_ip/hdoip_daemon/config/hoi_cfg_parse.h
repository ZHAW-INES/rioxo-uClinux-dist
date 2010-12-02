/*
 * hoi_cfg_parse.h
 *
 *  Created on: 26.11.2010
 *      Author: stth
 */

#ifndef HOI_CFG_PARSE_H_
#define HOI_CFG_PARSE_H_

#include "hoi_cfg.h"

void hoi_cfg_prs_ip_str(uint32_t ip, char* str);
void hoi_cfg_prs_ip_val(char* str, uint32_t* ip);
void hoi_cfg_prs_mac_str(char* mac, char* str);
void hoi_cfg_prs_mac_val(char* str, char* mac);
void hoi_cfg_prs_str16(uint16_t port, char* str);
void hoi_cfg_prs_val16(char* str, uint16_t* port);
void hoi_cfg_prs_str32(uint32_t port, char* str);
void hoi_cfg_prs_val32(char* str, uint32_t* port);

static inline void hoi_cfg_get_dev_ip(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_ip_str(handle->dev_ip, str); }

static inline void hoi_cfg_set_dev_ip(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_ip_val(str, &(handle->dev_ip)); }

static inline void hoi_cfg_get_dev_mac(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_mac_str(handle->dev_mac, str); }

static inline void hoi_cfg_set_dev_mac(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_mac_val(str, handle->dev_mac); }

static inline void hoi_cfg_get_dev_mode(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_str32(handle->dev_mode, str); }

static inline void hoi_cfg_set_dev_mode(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_val32(str, handle->dev_mode); }

static inline void hoi_cfg_get_aud_ip(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_ip_str(handle->aud_ip, str); }

static inline void hoi_cfg_set_aud_ip(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_ip_val(str, &(handle->aud_ip)); }

static inline void hoi_cfg_get_vid_ip(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_ip_str(handle->vid_ip, str); }

static inline void hoi_cfg_set_vid_ip(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_ip_val(str, &(handle->vid_ip)); }

static inline void hoi_cfg_get_aud_port(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_str16(handle->aud_udp_port, str); }

static inline void hoi_cfg_set_aud_port(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_val16(str, &(handle->aud_udp_port)); }

static inline void hoi_cfg_get_vid_port(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_str16(handle->vid_udp_port, str); }

static inline void hoi_cfg_set_vid_port(t_hoi_cfg* handle, char* str) {
    hoi_cfg_prs_val16(str, &(handle->vid_udp_port)); }


#endif /* HOI_CFG_PARSE_H_ */
