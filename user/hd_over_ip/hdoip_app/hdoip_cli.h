/*
 * hdoip_cli.h
 *
 *  Created on: 20.10.2010
 *      Author: stth
 *
 */

#ifndef HDOIP_CLI_H_
#define HDOIP_CLI_H_

/* Constants */
#define CMD_NAME        "hdoip"
#define HELP_STR        "help"

/* Command definitions */
typedef struct {
    char*       cmd_str;
    int         arg_cnt;
    int         (*fnc_ptr)(int fd, int fdr, char**, int);
    char*       usage_str;
} t_hdoip_cli_cmd_arr;


extern const int cmd_cnt;
extern const t_hdoip_cli_cmd_arr cmd_arr[];

#endif
