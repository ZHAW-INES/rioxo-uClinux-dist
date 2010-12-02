#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "hoi_cfg.h"
#include "hoi_cfg_parse.h"

typedef struct {
    char*       param_str;
    void        (*read_fnc)(t_hoi_cfg* handle, char* str);
    void        (*write_fnc)(t_hoi_cfg* handle, char* str);
} t_hoi_cfg_param_arr;


/****************************************************
 * Parameter definition
 */

#define HOI_CFG_PARAM_CNT           7

const t_hoi_cfg_param_arr param_arr[] = {
    {"dev_mac",         hoi_cfg_get_dev_mac,    hoi_cfg_set_dev_mac},
    {"dev_ip",          hoi_cfg_get_dev_ip,     hoi_cfg_set_dev_ip},
    {"dev_mode",        hoi_cfg_get_dev_mode,   hoi_cfg_set_dev_mode},
    {"aud_ip",          hoi_cfg_get_aud_ip,     hoi_cfg_set_aud_ip},
    {"aud_udp_port",    hoi_cfg_get_aud_port,   hoi_cfg_set_aud_port},
    {"vid_ip",          hoi_cfg_get_vid_ip,     hoi_cfg_set_vid_ip},
    {"vid_udp_port",    hoi_cfg_get_vid_port,   hoi_cfg_set_vid_port}
};


/* Sets the handle to default values
 *
 * @param handle pointer to the config handle
 * @return error code
 */
int hoi_cfg_default(t_hoi_cfg* handle)
{
    handle->dev_mac[0] = 0x00;              // 00:01:02:03:04:05
    handle->dev_mac[1] = 0x01;
    handle->dev_mac[2] = 0x02;
    handle->dev_mac[3] = 0x03;
    handle->dev_mac[4] = 0x04;
    handle->dev_mac[5] = 0x05; 
    handle->dev_ip = 0xC0A802C8;            // 192.168.1.200
    handle->dev_mode = HOI_CFG_MODE_RX;     // Receiver mode

    handle->aud_ip = 0xC0A80234;            // 192.168.1.52
    handle->vid_ip = 0xC0A80235;            // 192.168.1.53
    handle->aud_udp_port = 0x5000;          // Audio UDP port: 20480d
    handle->vid_udp_port = 0x6000;          // Video UDP port: 24576d

    return 0;
}

/* Initialize the config handle. Read from file (if exist) or creat file and set values to default
 *
 * @param handle pointer to the config handle
 * @param file_location path to the file as string
 * @return error code
 */
int hoi_cfg_init(t_hoi_cfg* handle, char* file_location) 
{
    int fd, exist;

    strcpy(handle->file_location, file_location);
    
    fd = open(handle->file_location, O_RDWR); 
    exist = fd;
    close(fd);

    /* Does a config file exist? */
    if(exist < 0) {
        printf("\nCreate config file: %s\n", handle->file_location);
        hoi_cfg_default(handle);
        hoi_cfg_save(handle);
    } else {
        /* read config file */
        printf("\nRead config file: %s\n", handle->file_location);
        hoi_cfg_read(handle);
    }
    
    return 0;
}

/* Saves the handle parameters to file
 *
 * @param handle pointer to the config handle
 * @return error code
 */
int hoi_cfg_save(t_hoi_cfg* handle) 
{
    int i;
    char config[200] = "# HD over IP configuration file\n#\n# UDP Ports are in hexadecimal\n# Device mode (dev_mode) 01: Receiver; 02: Transmitter\n#\n\n";
    char param[20];
    FILE* fd;

    fd = fopen(handle->file_location, "w+");

    if(fd == NULL) {
        printf("Could not open %s\n", handle->file_location);
        return -1;
    }

    fprintf(fd, config);    // writing title
 
    for(i=0 ; i<HOI_CFG_PARAM_CNT ; i++) {
        (param_arr[i].read_fnc)(handle, param);
        fprintf(fd, "%s=%s\n",param_arr[i].param_str, param);
    }
   
    fclose(fd);

    return 0;
}

/* Reads the config file and put the parameter into the handle
 *
 * @param handle pointer to the config handle
 * @return error code
 */
int hoi_cfg_read(t_hoi_cfg* handle) 
{
    FILE* fd;
    char* line = NULL, value, key;
    size_t len = 0;
    uint32_t i;

    fd = fopen(handle->file_location, "r+");

    if(fd == NULL) {
        printf("Could not open %s\n", handle->file_location);
        return -1;
    }

    while((getline(&line, &len, fd)) != -1) {
        if((strcmp(line[0], '#') != 0) && (strcmp(line[0], '\n') != 0)) { // 0x10 = new line
            value = strchr(line, 0x3D);
            if(value != NULL) {
                value += 1;
                key = strtok(line, "=");
                
                for(i=0 ; i < HOI_CFG_PARAM_CNT ; i++) {
                    if(strcmp(key, param_arr[i].param_str) == 0) {
                        (param_arr[i].write_fnc)(handle, value);
                    }
                } 
            }
        }
    }

    fclose(fd);   

    return 0;
}
