
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>

#include "hdoip_cli.h"

#define FIFO_NODE        "/tmp/hdoipd0"


int main(int argc, char **argv)
{
    int i, cmd_found = 0;
    int fd = -1;

    if(argc < 2)  {                                                                         /* No command */
        hdoip_cli_help(fd, argv, argc);
        goto out;
    }

    fd = open(FIFO_NODE, O_RDWR, 0600);                                                     /* open character device */
    if(fd < 0) {
        err(1, "Failed to open %s: %s \n", FIFO_NODE, strerror(errno)); 
        exit(EXIT_FAILURE);
    }

    for(i=0 ; i<cmd_cnt ; i++) {
         if(strcmp(argv[1], (cmd_arr[i]).cmd_str) == 0) {
            cmd_found = 1;
            if((argc == 3) && (strcmp(argv[2], HELP_STR) == 0)) {                           /* Command help */
                printf("%s %s %s\n",CMD_NAME, cmd_arr[i].cmd_str, cmd_arr[i].usage_str);
                break;
            } else if(((argc-2) == cmd_arr[i].arg_cnt) || (cmd_arr[i].arg_cnt == -1)) {     /* Command execution */
                if((argc-2) > 0) {
                    argv = argv+2;
                }
                printf("calling %s\n", cmd_arr[i].cmd_str);
                (cmd_arr[i].fnc_ptr)(fd, argv, argc-2);
                break;
            } else {
                printf("argument error! Usage:\n");                                         /* Argument error */
                printf("%s %s %s\n",CMD_NAME, cmd_arr[i].cmd_str, cmd_arr[i].usage_str);
                break;
            }
        }
    }

    if(cmd_found == 0) {
        printf("\'%s %s\' not found!\n", CMD_NAME, argv[1]);
        hdoip_cli_help(fd, argv, argc);
    }

out:
    close(fd);
    return 0;
}

