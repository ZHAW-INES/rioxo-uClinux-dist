#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "hoi_cfg.h"
#include "hdoipd.h"

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

void strtrim(char** p)
{
    int i;
    char* s;
    for (s=*p;*s==' ';s++);
    for (i=strlen(s);i&&(s[i-1]==' ');i--);
    s[i] = 0;
    *p = s;
}

int hoi_cfg_read(char* filename)
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
                reg_verify_set(key, value);
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

void hoi_cfg_system()
{
    char tmp[256];

    report(INFO "config system...");

    hdoipd.eth_alive = reg_get_int("network-alive");
    hdoipd.eth_timeout = reg_get_int("network-timeout");

    sysprintf(tmp, "/sbin/ifconfig %s hw ether %s", reg_get("system-ifname"), reg_get("system-mac"));
    sysprintf(tmp, "/sbin/ifconfig %s %s netmask %s up", reg_get("system-ifname"), reg_get("system-ip"), reg_get("system-subnet"));
    sysprintf(tmp, "/sbin/route add default gw %s", reg_get("system-gateway"));
    sysprintf(tmp, "%s", reg_get("system-cmd"));

    hoi_cfg_set_dns_server(reg_get("system-dns1"), reg_get("system-dns2"));

    report(INFO "config system done!");
}

int hoi_cfg_set_dns_server(char* dns1, char* dns2)
{
    char *line = 0;
    int  len=0, ret = 0;
    int *fd;

    fd = open("/etc/resolv.conf", O_RDWR, 0600);
    if(fd) {
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
    return 0;
}

int hoi_cfg_get_dns_server(char *dns1, char *dns2)
{

    char *line = 0;
    char *key = 0, *value = 0;
    int  len=0, i=0;
    FILE *fd;

    fd = fopen("/etc/resolv.conf", "r");
    if(fd) {
        while (getline(&line, &len, fd) != -1) {
            key = strtok(line, " ");
            if(strcmp(key, "nameserver") == 0) {
                value = strtok(0, "\n");
                if(value != NULL) {
                    report("value : %s",value);
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

        return 0;
    } else {
        return -1;
    }
    return 0;
}

