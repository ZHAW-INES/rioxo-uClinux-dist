/*
 * rscp_net.c
 *
 *  Created on: 01.12.2010
 *      Author: alda
 */

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "rscp_net.h"
#include "rscp_error.h"
#include "hdoipd.h"



char *mac_ntoa(unsigned char *ptr)
{
    static char address[30];
    snprintf(address, 30, "%02X:%02X:%02X:%02X:%02X:%02X",
            ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
    return(address);
}


int net_get_local_hwaddr(int sock, char* ifn, uint8_t* mac)
{
    int n;
    struct ifreq req;

    strcpy(req.ifr_name, ifn);

    if((n = ioctl(sock, SIOCGIFHWADDR, (void*)&req)) == -1 ) {
        perror("ioctl(SIOCGIFHWADDR) failed");
        return RSCP_ERRORNO;
    }

    for (int i=0; i<6; i++) {
        mac[i] = req.ifr_hwaddr.sa_data[i];
    }

    return RSCP_SUCCESS;
}

int net_get_local_addr(int sock, char* ifn, uint32_t* addr)
{
    int n;
    struct ifreq req;

    strcpy(req.ifr_name, ifn);

    if((n = ioctl(sock, SIOCGIFADDR, (void*)&req)) == -1 ) {
        perror("ioctl(SIOCGIFADDR) failed");
        return RSCP_ERRORNO;
    }

    *addr = ((struct sockaddr_in*)&req.ifr_addr)->sin_addr.s_addr;

    return RSCP_SUCCESS;
}

int net_read_arp_map(int sock, char* ifn, uint32_t address, uint8_t* mac)
{
    struct arpreq req;
    struct sockaddr_in *sin;

    memset(&req, 0, sizeof(req));
    sin = (struct sockaddr_in *)&req.arp_pa;
    sin->sin_family = AF_INET; /* Address Family: Internet */
    sin->sin_addr.s_addr = address;
    strcpy(req.arp_dev, ifn);

    if (ioctl(sock, SIOCGARP, (void*)&req) == -1) {
        perror("ioctl(SIOCGARP) failed");
        return RSCP_ERRORNO;
    }

    if(!(req.arp_flags & ATF_COM)){
        return RSCP_ERRORNO;
    }

    for (int i=0;i<6;i++) {
        mac[i] = req.arp_ha.sa_data[i];
    }

    return RSCP_SUCCESS;
}


int net_get_remote_hwaddr(int sock, char* ifn, uint32_t address, uint8_t* mac)
{
    char *str;
    char buf[128];
    struct hostent* gw;
    struct ifreq ifreq;
    struct sockaddr_in sin;
    int ret;
    uint32_t subnet, dev_ip;

    memset(&ifreq, 0, sizeof(ifreq));
    strncpy(ifreq.ifr_name, "eth0", sizeof("eth0"));

    /* Read device ip */
    if((ioctl(sock, SIOCGIFADDR, (void*)&ifreq)) == -1 ) {
            perror("ioctl(SIOCGIFADDR) failed");
            return RSCP_ERRORNO;
    }

    memcpy(&sin, &ifreq.ifr_addr, sizeof(struct sockaddr));
    dev_ip = sin.sin_addr.s_addr;

    /* Read subnet mask */
    if((ioctl(sock, SIOCGIFNETMASK, (void*)&ifreq)) == -1 ) {
        perror("ioctl(SIOCGIFNETMASK) failed");
        return RSCP_ERRORNO;
    }

    memcpy(&sin, &ifreq.ifr_addr, sizeof(struct sockaddr));
    subnet = sin.sin_addr.s_addr;

    /* destination is in local network*/
    if((subnet & address) == (subnet & dev_ip)) {
        ret = net_read_arp_map(sock, ifn, address, mac);
        if (ret != RSCP_SUCCESS) {
            sin.sin_addr.s_addr = address;
            sprintf(buf, "/bin/busybox arping -c 1 %s", inet_ntoa(sin.sin_addr));
            system(buf);

            ret = net_read_arp_map(sock, ifn, address, mac);
        }
    }

    if(ret == RSCP_SUCCESS) {
        return RSCP_SUCCESS;
    }

    /* Switching to gateway */
    str = reg_get("system-gateway");
    gw = gethostbyname(str);

    if (!gw) {
        perror("net_get_remote_hwaddr() gethostbyname failed");
        return RSCP_ERRORNO;
    }

    address = *((uint32_t*)gw->h_addr_list[0]);
    ret = net_read_arp_map(sock, ifn, address, mac);

    if(ret != RSCP_SUCCESS) {
        sprintf(buf, "/sbin/busybox arping -c 1 %s", str);
        system(buf);
        ret =  net_read_arp_map(sock, ifn, address, mac);
    }

    return ret;
}

char* str_ntoa(uint32_t address)
{
    struct in_addr a1 = {
        .s_addr = address
    };
    return inet_ntoa(a1);
}
