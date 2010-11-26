
#include "stdeth.h"

static void eth_val_to_arr(uint8_t* arr, uint16_t upper16, uint32_t lower32) 
{
    arr[0] = (upper16 & 0xFF00) >> 8;
    arr[1] = (upper16 & 0x00FF);
    arr[2] = (lower32 & 0xFF000000) >> 24;
    arr[3] = (lower32 & 0x00FF0000) >> 16;
    arr[4] = (lower32 & 0x0000FF00) >> 8;
    arr[5] = (lower32 & 0x000000FF);
}

int eth_report_params(struct hdoip_eth_params* eth_params) 
{
    uint8_t arr[6];

    REPORT(INFO, "Ethernet packet length : %d words",eth_params->packet_length);
  
    eth_val_to_arr(arr, eth_params->dst_mac_upper16, eth_params->dst_mac_lower32);
    REPORT(INFO, "ETH destination MAC    : %02x:%02x:%02x:%02x:%02x:%02x",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]); 
    
    eth_val_to_arr(arr, eth_params->src_mac_upper16, eth_params->src_mac_lower32);
    REPORT(INFO, "ETH source MAC         : %02x:%02x:%02x:%02x:%02x:%02x",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]); 
  
    eth_val_to_arr(arr, 0, eth_params->ipv4_dst_ip); 
    REPORT(INFO, "IPv4 destination IP    : %d.%d.%d.%d",arr[2], arr[3], arr[4], arr[5]);

    eth_val_to_arr(arr, 0, eth_params->ipv4_src_ip); 
    REPORT(INFO, "IPv4 source IP         : %d.%d.%d.%d",arr[2], arr[3], arr[4], arr[5]);

    REPORT(INFO, "IPv4 TTL               : 0x%x", eth_params->ipv4_ttl);
    REPORT(INFO, "IPv4 TOS               : 0x%x", eth_params->ipv4_tos);
    REPORT(INFO, "UDP destination port   : 0x%04x", eth_params->udp_dst_port);
    REPORT(INFO, "UDP source port        : 0x%04x", eth_params->udp_src_port);
    REPORT(INFO, "RTP SSRC               : 0x%08x", eth_params->rtp_ssrc);

    return 0;
}
