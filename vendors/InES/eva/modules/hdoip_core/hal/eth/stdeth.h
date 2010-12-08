/*
 *  stdeth.h
 *
 *  Created on: 26.10.2010
 *      Author: stth
 */

#ifndef STDETH_H_
#define STDETH_H_

#include <linux/types.h>

// values are in network order
typedef struct hdoip_eth_params {
    uint32_t    packet_size;          // in bytes
    uint32_t    ipv4_src_ip;
    uint32_t    ipv4_dst_ip;
    uint32_t    ipv4_ttl;
    uint32_t    ipv4_tos;
    uint32_t    udp_src_port;
    uint32_t    udp_dst_port;
    uint32_t    rtp_ssrc;
    uint8_t     dst_mac[6];
    uint8_t     src_mac[6];
} __attribute__((packed)) hdoip_eth_params;

int eth_report_params(struct hdoip_eth_params* eth_params);

#endif /* STDETH_H_ */
