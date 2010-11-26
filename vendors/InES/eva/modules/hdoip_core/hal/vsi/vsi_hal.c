
#include "vsi_hal.h"

/** 
	Example: MAC = 01:02:03:04:05:06
	upper16 = 0x0102
	lower32 = 0x03040506
 */

void vsi_set_dst_mac(void* p, uint16_t upper16, uint32_t lower32)
{
	HOI_WR32(p, VSI_OFF_DST_MAC_FIRST_2_RW, upper16);
	HOI_WR32(p, VSI_OFF_DST_MAC_LAST_4_RW, lower32);
}

void vsi_get_dst_mac(void* p, uint16_t* upper16, uint32_t* lower32)
{
	*upper16 = HOI_RD32(p, VSI_OFF_DST_MAC_FIRST_2_RW);
	*lower32 = HOI_RD32(p, VSI_OFF_DST_MAC_LAST_4_RW);
}

void vsi_set_src_mac(void* p, uint16_t upper16, uint32_t lower32) 
{
	HOI_WR32(p, VSI_OFF_SRC_MAC_FIRST_4_RW, (upper16<<16) | (lower32>>16));
	HOI_WR32(p, VSI_OFF_SRC_MAC_LAST_2_RW, lower32&0xFFFF);
}

void vsi_get_src_mac(void* p, uint16_t* upper16, uint32_t* lower32) 
{
    *lower32 = HOI_RD32(p, VSI_OFF_SRC_MAC_FIRST_4_RW);
    *upper16 = (*lower32 & 0xFFFF0000) >> 16;
    *lower32 = ((*lower32 & 0x0000FFFF) << 16) | (HOI_RD32(p,VSI_OFF_SRC_MAC_LAST_2_RW) & 0xFFFF);
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
    vsi_set_eth_word_len(p, eth_params->packet_length);
	vsi_set_dst_mac(p, eth_params->dst_mac_upper16, eth_params->dst_mac_lower32);
	vsi_set_src_mac(p, eth_params->src_mac_upper16, eth_params->src_mac_lower32);
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

    vsi_get_dst_mac(p, &(eth_params->dst_mac_upper16), &(eth_params->dst_mac_lower32));
    vsi_get_src_mac(p, &(eth_params->src_mac_upper16), &(eth_params->src_mac_lower32));

    eth_params->packet_length = vsi_get_eth_word_len(p); 
    eth_params->ipv4_dst_ip = vsi_get_dst_ip(p);
    eth_params->ipv4_src_ip = vsi_get_src_ip(p);
    eth_params->ipv4_ttl = vsi_get_ttl(p);
    eth_params->ipv4_tos = vsi_get_tos(p);
    eth_params->udp_src_port = vsi_get_src_port(p);
    eth_params->udp_dst_port = vsi_get_dst_port(p);
    eth_params->rtp_ssrc = vsi_get_ssrc(p);

}


