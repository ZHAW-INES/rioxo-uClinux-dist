#define _GNU_SOURCE	/* for getline() */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "hdoipd.h"
#include "hoi_cfg.h"

char *pid_udhcpc = "/tmp/udhcp.pid";

static uint32_t hex2int(char *a, unsigned int len)
{
    unsigned int i;
    uint32_t val = 0;

    for(i=0;i<len;i++)
       if(a[i] <= 57)
        val += (a[i]-48)*(1<<(4*(len-1-i)));
       else
        val += (a[i]-55)*(1<<(4*(len-1-i)));
    return val;
}


void hoi_cfg_store(char* key, char* value, void* fd)
{
    fprintf((FILE*)fd, "%s=%s\n", key, value);
}

int hoi_cfg_write(char* filename)
{
    FILE* fd;

    report("write config file %s", filename);
    if (!(fd = fopen(filename, "w"))) {
        report("Could not open %s", filename);
        return -1;
    }

    fprintf(fd, "# HD over IP\n");

    bstmap_traverse(hdoipd.registry, hoi_cfg_store, fd);

    fclose(fd);

    return 0;
}

static void strtrim(char** p)
{
    int i;
    char* s;
    for (s=*p;*s==' ';s++);
    for (i=strlen(s);i&&(s[i-1]==' ');i--);
    s[i] = 0;
    *p = s;
}

int hoi_cfg_read(char* filename, bool reset_to_default_values)
{
    FILE *fd;
    char *line = 0, *key, *value;
    size_t len = 0;

    report(INFO "read config file %s", filename);

    if (!(fd = fopen(filename, "r"))) {
        report("Could not open %s", filename);
        return -1;
    }

    while (getline(&line, &len, fd) != -1) {
        if (line && (line[0] != '#')) {
            key = strtok(line, "=");
            value = strtok(0, "\n");
            if (key && value) {
                strtrim(&key);
                strtrim(&value);
                if (reset_to_default_values) {
                    if ((!strcmp(key, "system-mac")) || (!strcmp(key, "serial-number"))) {
                        reg_verify_set(key, value);
                    }
                } else {
                    reg_verify_set(key, value);
                }
            }
        }
    }

    if (line) free(line);
   
    fclose(fd);

    return 0;
}

#define sysprintf(x, ...) \
{ \
    sprintf(x, __VA_ARGS__); \
    reportn(CONT "%s", x); \
    system(x); \
    report(" [DONE]"); \
}

// Set MAC address
void hoi_cfg_mac_addr(char *ifname, char *mac)
{
    char tmp[256];
    sysprintf(tmp, "/sbin/ifconfig %s hw ether %s", ifname, mac);
}

void hoi_cfg_default_gw(char *gw)
{
    char tmp[256];
    sysprintf(tmp, "/sbin/route del default");
    sysprintf(tmp, "/sbin/route add default gw %s", gw);
}

void hoi_cfg_ip_addr(char *ifname, char *ip, char *netmask)
{
    char tmp[256];
    sysprintf(tmp, "/sbin/ifconfig %s %s netmask %s up", ifname, ip, netmask);
}

// Dynamic IP: DHCP client (busybox udhcpc)
void hoi_cfg_dynamic_ip()
{
    char tmp[256];
    sysprintf(tmp, "/sbin/ifconfig %s 0.0.0.0", reg_get("system-ifname"));
    sysprintf(tmp, "/bin/busybox udhcpc -i %s -h %s -p %s",reg_get("system-ifname"),reg_get("system-hostname"), pid_udhcpc);
}

// Static IP
void hoi_cfg_static_ip()
{
    char tmp[256], buf[256];

    // disable DHCP client (kill)
    int fd = open(pid_udhcpc, O_RDWR, 0600);
    if(fd != -1) {
        read(fd, buf, 20);
        sysprintf(tmp, "/bin/kill %s", buf);
    }
    close(fd);

    hoi_cfg_ip_addr(reg_get("system-ifname"), reg_get("system-ip"), reg_get("system-subnet"));
    hoi_cfg_default_gw(reg_get("system-gateway"));
    hoi_cfg_set_dns_server(reg_get("system-dns1"), reg_get("system-dns2"));
}

void hoi_cfg_system()
{
    char tmp[256];

    report(INFO "config system...");

    hdoipd.eth_alive = reg_get_int("network-alive");
    hdoipd.eth_timeout = reg_get_int("network-timeout");

    hoi_cfg_mac_addr(reg_get("system-ifname"), reg_get("system-mac"));

    if(hdoipd.dhcp) {
        hoi_cfg_dynamic_ip();
    } else {
        hoi_cfg_static_ip();
    }

    sysprintf(tmp, "%s", reg_get("system-cmd"));

    report(INFO "config system done!");
}

int hoi_cfg_set_dns_server(char* dns1, char* dns2)
{
    char *line = 0;
    int ret = 0;
    int fd;

    system("/bin/rm /etc/resolv.conf");
    fd = open("/etc/resolv.conf", O_CREAT|O_RDWR, 0600);
    if(fd != -1) {
        if(strcmp(dns1,"") != 0) {
            sprintf(line, "nameserver %s\n", dns1);
            ret = write(fd, line, strlen(line));
            if(ret == -1) {  return -1;
            } else {         report("wrote %d bytes", ret); }
        }

        if(strcmp(dns2,"") != 0) {
            sprintf(line, "nameserver %s\n", dns2);
            ret = write(fd, line, strlen(line));
            if(ret == -1) {  return -1;
            } else {         report("wrote %d bytes", ret); }
        }

    } else {
        return -1;
    }
    close(fd);
    return 0;
}

int hoi_cfg_get_dns_server(char *dns1, char *dns2)
{

    char *line = 0;
    char *key = 0, *value = 0;
    int i = 0;
    size_t len;
    FILE *fd;

    fd = fopen("/etc/resolv.conf", "r");
    if(fd != NULL) {
        while (getline(&line, &len, fd) != -1) {
            key = strtok(line, " ");
            if(strcmp(key, "nameserver") == 0) {
                value = strtok(0, "\n");
                if(value != NULL) {
                    if(i == 0) {
                        strcpy(dns1, value);
                    } else if(i == 1) {
                        strcpy(dns2, value);
                    }
                    i++;
                }
            }
        }

        if(i == 0)      strcpy(dns1, "");
        if(i < 2)       strcpy(dns2, "");

    } else {
        return -1;
    }
    fclose(fd);
    return 0;
}

int hoi_cfg_get_ip(char *ip)
{
    struct ifreq ifreq;
    struct sockaddr_in sin;
    int sock;
    char *ifname = reg_get("system-ifname");
    char *tmp;

    memset(&ifreq, 0, sizeof(ifreq));
    strncpy(ifreq.ifr_name, ifname, sizeof(ifname));

    sock = socket(PF_INET, SOCK_DGRAM, 0);

    if(sock != -1) {
        /* Read device ip */
        if((ioctl(sock, SIOCGIFADDR, (void*)&ifreq)) == -1 ) {
            sin.sin_addr.s_addr = 0x00000000;
        } else {
            memcpy(&sin, &ifreq.ifr_addr, sizeof(struct sockaddr));
        }

        tmp = inet_ntoa(sin.sin_addr);
        strcpy(ip, tmp);
    } else {
        strcpy(ip,"");
    }

    shutdown(sock, SHUT_RDWR);
    close(sock);

    return 0;
}

int hoi_cfg_get_subnet(char *subnet)
{
    struct ifreq ifreq;
    struct sockaddr_in sin;
    int sock;
    char *ifname = reg_get("system-ifname");
    char *tmp;

    memset(&ifreq, 0, sizeof(ifreq));
    strncpy(ifreq.ifr_name, ifname, sizeof(ifname));

    sock = socket(PF_INET, SOCK_DGRAM, 0);

    if(sock != -1) {
        /* Read device subnet mask */
        if((ioctl(sock, SIOCGIFNETMASK, (void*)&ifreq)) == -1) {
            sin.sin_addr.s_addr = 0x00000000;
        } else {
            sin.sin_addr.s_addr = 0x000000FF;
            memcpy(&sin, &ifreq.ifr_netmask, sizeof(struct sockaddr));
        }
        tmp = inet_ntoa(sin.sin_addr);
        strcpy(subnet,tmp);
    } else {
        strcpy(subnet,"");
    }

    shutdown(sock, SHUT_RDWR);
    close(sock);

    return 0;
}

int hoi_cfg_get_default_gw(char *gw)
{
    struct sockaddr_in sin;
    FILE *fd;
    char *line = 0, *ifname, *value;
    size_t len = 0;
    uint32_t gw_ip;

    ifname = reg_get("system-ifname");

    fd = fopen("/proc/net/route", "r");
    if(fd != NULL) {
        getline(&line, &len, fd);   // skip first line
        while (getline(&line, &len, fd) != -1) {
            value = strtok(line, "\t");
            if(strcmp(ifname, value) == 0) {    // if interface name match
                value = strtok(0, "\t");
                if(strcmp("00000000", value) == 0) { // if default gateway
                    value = strtok(0, "\t");
                    gw_ip = hex2int(value, 8);
                    sin.sin_addr.s_addr = gw_ip;
                    value = inet_ntoa(sin.sin_addr);
                    strcpy(gw,value);
                    fclose(fd);
                    return 0;
                }
            }
        }
    }
    strcpy(gw, "");
    fclose(fd);
    return -1;
}

