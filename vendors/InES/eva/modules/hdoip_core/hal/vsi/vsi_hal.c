
#include "vsi_hal.h"

/** 
	Example: MAC = 01:02:03:04:05:06
	upper16 = 0x0102
	lower32 = 0x03040506
 */

void vsi_set_dst_mac(void* p, uint8_t* mac)
{
    HOI_WR8(p, VSI_OFF_DST_MAC_0, mac[0]);
    HOI_WR8(p, VSI_OFF_DST_MAC_1, mac[1]);
    HOI_WR8(p, VSI_OFF_DST_MAC_2, mac[2]);
    HOI_WR8(p, VSI_OFF_DST_MAC_3, mac[3]);
    HOI_WR8(p, VSI_OFF_DST_MAC_4, mac[4]);
    HOI_WR8(p, VSI_OFF_DST_MAC_5, mac[5]);
}

void vsi_get_dst_mac(void* p, uint8_t* mac)
{
    mac[0] = HOI_RD8(p, VSI_OFF_DST_MAC_0);
    mac[1] = HOI_RD8(p, VSI_OFF_DST_MAC_1);
    mac[2] = HOI_RD8(p, VSI_OFF_DST_MAC_2);
    mac[3] = HOI_RD8(p, VSI_OFF_DST_MAC_3);
    mac[4] = HOI_RD8(p, VSI_OFF_DST_MAC_4);
    mac[5] = HOI_RD8(p, VSI_OFF_DST_MAC_5);
}

void vsi_set_src_mac(void* p, uint8_t* mac)
{
    HOI_WR8(p, VSI_OFF_SRC_MAC_0, mac[0]);
    HOI_WR8(p, VSI_OFF_SRC_MAC_1, mac[1]);
    HOI_WR8(p, VSI_OFF_SRC_MAC_2, mac[2]);
    HOI_WR8(p, VSI_OFF_SRC_MAC_3, mac[3]);
    HOI_WR8(p, VSI_OFF_SRC_MAC_4, mac[4]);
    HOI_WR8(p, VSI_OFF_SRC_MAC_5, mac[5]);
}

void vsi_get_src_mac(void* p, uint8_t* mac)
{
    mac[0] = HOI_RD8(p, VSI_OFF_SRC_MAC_0);
    mac[1] = HOI_RD8(p, VSI_OFF_SRC_MAC_1);
    mac[2] = HOI_RD8(p, VSI_OFF_SRC_MAC_2);
    mac[3] = HOI_RD8(p, VSI_OFF_SRC_MAC_3);
    mac[4] = HOI_RD8(p, VSI_OFF_SRC_MAC_4);
    mac[5] = HOI_RD8(p, VSI_OFF_SRC_MAC_5);
}


void vsi_set_ttl(void* p, uint8_t ttl)
{
	uint32_t reg_val;
	
	reg_val = HOI_RD32(p, VSI_OFF_TTL_TOS_RW) & ~VSI_TTL_MSK;
	HOI_WR32(p, VSI_OFF_TTL_TOS_RW, reg_val | (ttl<<8));
}

uint8_t vsi_get_ttl(void* p)
{
    return (HOI_RD32(p, VSI_OFF_TTL_TOS_RW)>>8);
}

void vsi_set_tos(void* p, uint8_t tos)
{
	uint32_t reg_val;
	
	reg_val = HOI_RD32(p, VSI_OFF_TTL_TOS_RW) & ~VSI_TOS_MSK;
	HOI_WR32(p, VSI_OFF_TTL_TOS_RW, reg_val | tos);
}

uint8_t vsi_get_tos(void* p)
{
    return (HOI_RD32(p, VSI_OFF_TTL_TOS_RW) & VSI_TOS_MSK);
}

void vsi_set_eth_params(void* p, struct hdoip_eth_params* eth_params)
{
    vsi_set_eth_word_len(p, eth_params->packet_size/4);
	vsi_set_dst_mac(p, eth_params->dst_mac);
	vsi_set_src_mac(p, eth_params->src_mac);
	vsi_set_src_ip(p, eth_params->ipv4_src_ip);
	vsi_set_dst_ip(p, eth_params->ipv4_dst_ip);
	vsi_set_ttl(p, eth_params->ipv4_ttl);
	vsi_set_tos(p, eth_params->ipv4_tos);
	vsi_set_src_port(p, eth_params->udp_src_port);	
	vsi_set_dst_port(p, eth_params->udp_dst_port);	
	vsi_set_ssrc(p, eth_params->rtp_ssrc);
}

void vsi_get_eth_params(void* p, struct hdoip_eth_params* eth_params)
{

    vsi_get_dst_mac(p, eth_params->dst_mac);
    vsi_get_src_mac(p, eth_params->src_mac);

    eth_params->packet_size = vsi_get_eth_word_len(p)*4;
    eth_params->ipv4_dst_ip = vsi_get_dst_ip(p);
    eth_params->ipv4_src_ip = vsi_get_src_ip(p);
    eth_params->ipv4_ttl = vsi_get_ttl(p);
    eth_params->ipv4_tos = vsi_get_tos(p);
    eth_params->udp_src_port = vsi_get_src_port(p);
    eth_params->udp_dst_port = vsi_get_dst_port(p);
    eth_params->rtp_ssrc = vsi_get_ssrc(p);

}


