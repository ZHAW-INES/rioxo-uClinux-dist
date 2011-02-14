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

    report("read config file %s", filename);
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

void hoi_cfg_system()
{
    char tmp[256];
    sprintf(tmp, "/sbin/ifconfig %s hw ether %s", reg_get("system-ifname"), reg_get("system-mac"));
    system(tmp);
    sprintf(tmp, "/sbin/ifconfig %s %s netmask %s up", reg_get("system-ifname"), reg_get("system-ip"), reg_get("system-subnet"));
    system(tmp);
    sprintf(tmp, "/sbin/route add default gw %s", reg_get("system-gateway"));
    system(tmp);
    system(reg_get("system-cmd"));
}

