
#include "asi_hal.h"

void asi_set_dst_mac(void* p, uint8_t* mac)
{
    HOI_WR8(p, ASI_OFF_DST_MAC_0, mac[1]);
    HOI_WR8(p, ASI_OFF_DST_MAC_1, mac[0]);
    HOI_WR8(p, ASI_OFF_DST_MAC_2, mac[5]);
    HOI_WR8(p, ASI_OFF_DST_MAC_3, mac[4]);
    HOI_WR8(p, ASI_OFF_DST_MAC_4, mac[3]);
    HOI_WR8(p, ASI_OFF_DST_MAC_5, mac[2]);
}

void asi_get_dst_mac(void* p, uint8_t* mac)
{
    mac[1] = HOI_RD8(p, ASI_OFF_DST_MAC_0);
    mac[0] = HOI_RD8(p, ASI_OFF_DST_MAC_1);
    mac[5] = HOI_RD8(p, ASI_OFF_DST_MAC_2);
    mac[4] = HOI_RD8(p, ASI_OFF_DST_MAC_3);
    mac[3] = HOI_RD8(p, ASI_OFF_DST_MAC_4);
    mac[2] = HOI_RD8(p, ASI_OFF_DST_MAC_5);
}

void asi_set_src_mac(void* p, uint8_t* mac)
{
    HOI_WR8(p, ASI_OFF_SRC_MAC_0, mac[3]);
    HOI_WR8(p, ASI_OFF_SRC_MAC_1, mac[2]);
    HOI_WR8(p, ASI_OFF_SRC_MAC_2, mac[1]);
    HOI_WR8(p, ASI_OFF_SRC_MAC_3, mac[0]);
    HOI_WR8(p, ASI_OFF_SRC_MAC_4, mac[5]);
    HOI_WR8(p, ASI_OFF_SRC_MAC_5, mac[4]);
}

void asi_get_src_mac(void* p, uint8_t* mac)
{
    mac[3] = HOI_RD8(p, ASI_OFF_SRC_MAC_0);
    mac[2] = HOI_RD8(p, ASI_OFF_SRC_MAC_1);
    mac[1] = HOI_RD8(p, ASI_OFF_SRC_MAC_2);
    mac[0] = HOI_RD8(p, ASI_OFF_SRC_MAC_3);
    mac[5] = HOI_RD8(p, ASI_OFF_SRC_MAC_4);
    mac[4] = HOI_RD8(p, ASI_OFF_SRC_MAC_5);
}

/** Sets the time to live (TTL) parameter of the IPv4 header
 *
 * @param p pointer to the ACB registers
 * @param ttl time to live value
 */
void asi_set_ttl(void* p, uint8_t ttl)
{
    uint32_t tmp = HOI_RD32(p, ASI_OFF_TTL_TOS);
    HOI_WR32(p, ASI_OFF_TTL_TOS, (tmp & ~ASI_TTL_MSK) | (ttl << 8));
}


void asi_set_ttl_tos(void* p, uint8_t ttl, uint8_t tos)
{
    HOI_WR32(p, ASI_OFF_TTL_TOS, (tos & ASI_TOS_MSK) | ((ttl << 8) & ASI_TTL_MSK));
}

void asi_get_ttl_tos(void* p, uint8_t* ttl, uint8_t* tos)
{
    uint32_t tmp = HOI_RD32(p, ASI_OFF_TTL_TOS);
    *ttl = (tmp & ASI_TTL_MSK) >> 8;
    *tos = (tmp & ASI_TOS_MSK);
}


/** Return the time to live (TTL) parameter of the IPv4 header
 *
 * @param p pointer to the ACB registers
 * @return time to live value
 */
uint8_t asi_get_ttl(void* p)
{
    return ((HOI_RD32(p, ASI_OFF_TTL_TOS) & ASI_TTL_MSK) >> 8);
}

/** Sets the type of service (TOS) parameter of the IPv4 header
 *
 * @param p pointer to the ACB registers
 * @param tos type of service value
 */
void asi_set_tos(void* p, uint8_t tos)
{
    uint32_t tmp = HOI_RD32(p, ASI_OFF_TTL_TOS);
    HOI_WR32(p, ASI_OFF_TTL_TOS, (tmp & ~ASI_TOS_MSK) | tos);
}

/** Returns the type of service (TOS) parameter of the IPv4 header
 *
 * @param p pointer to the ACB registers
 * @return type of service (TOS) value
 */
uint8_t asi_get_tos(void* p)
{
    return (HOI_RD32(p, ASI_OFF_TTL_TOS) & ASI_TOS_MSK);
}

/** set the audio parameters (bits per sample, channel left count, channel right count)
 * 
 * @param p pointer to the ACB register
 * @param aud_params pointer to the audio parameter struct
 */
void asi_set_aud_params(void* p, struct hdoip_aud_params* aud_params)
{
    asi_set_aud_cfg(p, aud_params->ch_cnt_left, aud_params->ch_cnt_right, aud_params->sample_width);
}

/** get the audio parameters (bit per sample, channel left count, channel right count)
 * 
 * @param p pointer to the ACB register
 * @param aud_params pointe to the audio parameter struct
 */
void asi_get_aud_params(void* p, struct hdoip_aud_params* aud_params)
{
    asi_get_aud_cfg(p, &(aud_params->ch_cnt_left), &(aud_params->ch_cnt_right), &(aud_params->sample_width));
}

/** set following ethernet parameters: destination & source MAC, destination & source IPs, destination & source udp port and rtp ssrc
 * 
 * @param p pointer to the ACB registers
 * @param eth_params pointer to the ethernet parameter struct
 */ 
void asi_set_eth_params(void* p, struct hdoip_eth_params* eth_params)
{

    asi_set_dst_mac(p, eth_params->dst_mac);
    asi_set_src_mac(p, eth_params->src_mac);
    asi_set_ttl_tos(p, eth_params->ipv4_ttl, eth_params->ipv4_tos);
    asi_set_src_ip(p, eth_params->ipv4_src_ip);
    asi_set_dst_ip(p, eth_params->ipv4_dst_ip);
    asi_set_src_port(p, eth_params->udp_src_port);
    asi_set_dst_port(p, eth_params->udp_dst_port);
    asi_set_ssrc(p, eth_params->rtp_ssrc);
}

/** set following ethernet parameters: destination & source MAC, destination & source IPs, destination & source udp port, packet length and rtp ssrc
 * 
 * @param p pointer to the ACB registers
 * @param eth_params pointer to the ethernet parameter struct
 */ 
void asi_get_eth_params(void* p, struct hdoip_eth_params* eth_params)
{
    asi_get_dst_mac(p, eth_params->dst_mac);
    asi_get_src_mac(p, eth_params->src_mac);
    asi_get_ttl_tos(p, &(eth_params->ipv4_ttl), &(eth_params->ipv4_tos));

    eth_params->packet_size = asi_get_frame_size(p) * 4;
    eth_params->ipv4_src_ip = asi_get_src_ip(p);
    eth_params->ipv4_dst_ip = asi_get_dst_ip(p);
    eth_params->udp_src_port = asi_get_src_port(p);
    eth_params->udp_dst_port = asi_get_dst_port(p);
    eth_params->rtp_ssrc = asi_get_ssrc(p);

}

/** Sets the audio parameters (channel count left/right and bits per channel)
 *
 * @param p pointer to the ACB registers
 * @param ch_cnt_l channel count left value
 * @param ch_cnt_r channel count right value
 * @param bits number of bits per channel
 * @return error code (0=no error; 1=unsupported bits per channel
 */ 
uint8_t asi_set_aud_cfg(void* p, uint8_t ch_cnt_l, uint8_t ch_cnt_r, uint8_t bits)
{
    uint32_t reg_value = 0;
    
    reg_value = aud_bits_to_container(bits);

    reg_value = ((reg_value << 8) & ASI_CONTAINER_MSK) | (ch_cnt_l & ASI_CNT_LEFT_MSK) | ((ch_cnt_r << 4) & ASI_CNT_RIGHT_MSK);
    HOI_WR32(p, ASI_OFF_AUD_INFO, reg_value);

    return 0;
}

/** Reads the audio parameters (channel count left/right and bits per channel)
 *
 * @param p pointer to the ACB registers
 * @param ch_cnt_l pointer to the channel count left variable
 * @param ch_cnt_r pointer to the channel count right variable
 * @param bits pointer to the number of bits per channel variable
 */ 
void asi_get_aud_cfg(void* p, uint8_t* ch_cnt_l, uint8_t* ch_cnt_r, uint8_t* bits)
{
    uint32_t reg_value = HOI_RD32(p, ASI_OFF_AUD_INFO);

    *ch_cnt_l = (uint8_t) (reg_value & ASI_CNT_LEFT_MSK);
    *ch_cnt_r = (uint8_t) ((reg_value >> 4) & ASI_CNT_RIGHT_MSK);
    *bits = aud_container_to_bits(reg_value & ASI_CONTAINER_MSK);
}


