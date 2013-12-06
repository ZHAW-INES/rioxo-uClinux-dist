
#include "stdeth.h"
#include "std.h"


int eth_report_params(struct hdoip_eth_params* eth_params, int unsigned stream)
{
    REPORT(INFO, "Stream                 : %d", stream);
    REPORT(INFO, "Ethernet packet length : %d bytes",eth_params->packet_size);
    REPORT(INFO, "ETH destination MAC    : %pM", eth_params->dst_mac);
    REPORT(INFO, "ETH source MAC         : %pM", eth_params->src_mac);
    REPORT(INFO, "IPv4 destination IP    : %pI4", &eth_params->ipv4_dst_ip);
    REPORT(INFO, "IPv4 source IP         : %pI4", &eth_params->ipv4_src_ip);
    REPORT(INFO, "IPv4 TTL               : 0x%x", eth_params->ipv4_ttl);
    REPORT(INFO, "IPv4 TOS               : 0x%x", eth_params->ipv4_tos);
    REPORT(INFO, "UDP destination port   : %d", ntohs(eth_params->udp_dst_port));
    REPORT(INFO, "UDP source port        : %d", ntohs(eth_params->udp_src_port));
    REPORT(INFO, "RTP SSRC               : 0x%08x", eth_params->rtp_ssrc);

    return 0;
}
