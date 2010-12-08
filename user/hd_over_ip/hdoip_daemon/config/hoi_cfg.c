#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "hdoipd.h"
#include "hoi_cfg.h"
#include "hoi_cfg_default.h"

typedef struct {
    char*       param_str;
    void        (*read_fnc)(t_hoi_cfg* handle, char* str);
    void        (*write_fnc)(t_hoi_cfg* handle, char* str);
} t_hoi_cfg_param_arr;


/****************************************************
 * Parameter definition
 */

/* Initialize the config handle. Read from file (if exist) or creat file and set values to default
 *
 * @param handle pointer to the config handle
 * @param file_location path to the file as string
 * @return error code
 */
int hoi_cfg_init(t_hoi_cfg* handle, char* file_location) 
{
    int fd, exist;

    hoi_cfg_default();
    
    strcpy(handle->file_location, file_location);
    
    fd = open(handle->file_location, O_RDWR); 
    exist = fd;
    close(fd);

    /* Does a config file exist? */
    if(exist < 0) {
        printf("\nCreate config file: %s\n", handle->file_location);
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
    int i, cnt = 0;
    char config[200] = "# HD over IP configuration file\n#\n# UDP Ports are in hexadecimal\n# Device mode (dev_mode) 01: Receiver; 02: Transmitter\n#\n\n";
    FILE *fd;
    void **keyvalues;

    reg_cnt(&cnt);
    keyvalues = malloc(cnt * sizeof(void*));
    cnt = 0;
    reg_get_all(keyvalues, &cnt);

    fd = fopen(handle->file_location, "w+");

    if(fd == NULL) {
        printf("Could not open %s\n", handle->file_location);
        return -1;
    }

    fprintf(fd, config);    // writing title
 
    for(i=0 ; i<cnt ; i++) {
        fprintf(fd, "%s=%s\n",((t_keyvalue*)(keyvalues[i]))->key, ((t_keyvalue*)(keyvalues[i]))->value);
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
    FILE *fd;
    char *line = NULL, *value, *key;
    size_t size;
    uint32_t len;
    uint32_t i;

    fd = fopen(handle->file_location, "r+");

    if(fd == NULL) {
        printf("Could not open %s\n", handle->file_location);
        return -1;
    }

    while((getline(&line, &size, fd)) != -1) {
        if((strcmp(line[0], '#') != 0) && (strcmp(line[0], '\n') != 0)) { // 0x10 = new line
            len = strlen(line);
            if(line[len-1] == '\n') { // remove new line
                line[len-1] = 0;
            }
            value = strchr(line, 0x3D);
            if(value != NULL) {
                value += 1;
                key = strtok(line, "=");
                reg_set(key, value);
            }
        }
    }

    fclose(fd);   

    return 0;
}
