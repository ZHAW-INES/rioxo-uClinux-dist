/*
 *  stdeth.h
 *
 *  Created on: 26.10.2010
 *      Author: stth
 */

#ifndef STDETH_H_
#define STDETH_H_

#include <linux/types.h>

typedef struct hdoip_eth_params {
    uint16_t    packet_length;          /* in 32 bit words (total ethernet packet size) */
    uint16_t    dst_mac_upper16;
    uint32_t    dst_mac_lower32;
    uint16_t    src_mac_upper16;
    uint32_t    src_mac_lower32;
    uint32_t    ipv4_src_ip;
    uint32_t    ipv4_dst_ip;
    uint8_t     ipv4_ttl;
    uint8_t     ipv4_tos;
    uint16_t    udp_src_port;
    uint16_t    udp_dst_port;
    uint32_t    rtp_ssrc;
} __attribute__((packed)) hdoip_eth_params;

int eth_report_params(struct hdoip_eth_params* eth_params);

#endif /* STDETH_H_ */
