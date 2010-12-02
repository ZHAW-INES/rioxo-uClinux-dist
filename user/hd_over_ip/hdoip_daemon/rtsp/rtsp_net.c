/*
 * rtsp_net.c
 *
 *  Created on: 01.12.2010
 *      Author: alda
 */

#include "rtsp_net.h"
#include "rtsp_error.h"

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
        return RTSP_ERRORNO;
    }

    for (int i=0; i<6; i++) {
        mac[i] = req.ifr_hwaddr.sa_data[i];
    }

    return RTSP_SUCCESS;
}

int net_get_local_addr(int sock, char* ifn, uint32_t* addr)
{
    int n;
    struct ifreq req;

    strcpy(req.ifr_name, ifn);

    if((n = ioctl(sock, SIOCGIFADDR, (void*)&req)) == -1 ) {
        perror("ioctl(SIOCGIFADDR) failed");
        return RTSP_ERRORNO;
    }

    *addr = *(uint32_t*)req.ifr_hwaddr.sa_data;

    return RTSP_SUCCESS;
}

int net_get_remote_hwaddr(int sock, char* ifn, uint32_t address, uint8_t* mac)
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
        return RTSP_ERRORNO;
    }

    if(!(req.arp_flags & ATF_COM)){
        return RTSP_ERRORNO;
    }

    for (int i=0;i<6;i++) {
        mac[i] = req.arp_ha.sa_data[i];
    }

    return RTSP_SUCCESS;
}
