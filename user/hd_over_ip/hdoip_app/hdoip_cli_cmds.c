#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hdoip_cli.h"
#include "hdoip_lib.h"
#include "hdoipd_msg.h"

/* Shows CLI help
 *
 * @param fd 
 * @param argv
 * @param argc 
 * @return error code
 */
int hdoip_cli_help(int fd, char** argv, int argc)
{
    int i;
    printf("General:\n");
    printf("%s cmd help\n\n", CMD_NAME);
    printf("Command list:\n");
    for(i=0 ; i<cmd_cnt ; i++) {
        printf("%s %s %s\n", CMD_NAME, cmd_arr[i].cmd_str, cmd_arr[i].usage_str);
    }
    return 0;
}

int hdoip_cli_canvas(int fd, char** argv, int argc)
{
    uint32_t width, height, fps;
    printf("hdoip_cli_canvas\n");

    if (!(hdoip_lib_res(argv[0], &width, &height, &fps))) {
        hoic_canvas(fd, width, height, fps);
    } else {
        printf("\n");
    }

    return 0;
}

int hdoip_cli_capture(int fd, char** argv, int argc)
{
    printf("hdoip_cli_capture\n");

    if(strcmp(argv[0], "none") == 0) {
        hoic_capture(fd, false, 0, argv[1]);
    } else if(strcmp(argv[0], "jp2") == 0) {
        hoic_capture(fd, true, atoi(argv[1]), argv[2]);
    } else {
        printf("format not supported\n");
    }

    return 0;
}

int hdoip_cli_osd(int fd, char** argv, int argc)
{
    printf("hdoip_cli_capture\n");

    if(strcmp(argv[0], "on") == 0) {
        hoic_osd_on(fd);
    } else if(strcmp(argv[0], "off") == 0) {
        hoic_osd_off(fd);
    } else {
        printf("use: hdoip osd {on|off}\n");
    }

    return 0;
}

int hdoip_cli_load(int fd, char** argv, int argc)
{
    printf("hdoip_cli_load\n");

    hoic_load(fd, argv[0]);
    return 0;
}

int hdoip_cli_loop(int fd, char** argv, int argc)
{
    printf("hdoip_cli_loop\n");

    hoic_loop(fd);

    return 0;
}

int hdoip_cli_fmt(int fd, char** argv, int argc)
{
    int fmt = 0;
    printf("hdoip_cli_fmt\n");

    if (argc == 2) {
        fmt = hdoip_lib_color_format(argv[1]);
    } else if (argc == 3) {
        fmt = hdoip_lib_color_format(argv[1]);
        fmt |= hdoip_lib_color_format(argv[2]);
    } else {
        printf("too many arguments\n");
        return 0;
    }

    if (fmt == -1) {
        printf("unsupported format\n");
        return 0;
    }


    if(strcmp(argv[0], "in") == 0) {
        hoic_fmt_in(fd, fmt);
    } else if(strcmp(argv[0], "out") == 0) {
        hoic_fmt_out(fd, fmt);
    } else if(strcmp(argv[0], "proc") == 0) {
        hoic_fmt_proc(fd, fmt);
    } else {
        printf("unknown direction\n");
    }

    return 0;
}

/* Dummy function, just show text 
 *
 * @param fd
 * @param argv
 * @param argc
 * @return error code
 */
int hdoip_cli_dummy(int fd, char** argv, int argc)
{
    printf("hdoip_cli_dummy with no parameters\n");
    return 0;
}

/* dummy function with two arguments
 *
 * @param fd
 * @param argv
 * @param argc
 * @return error code
 */
int hdoip_cli_dummy2(int fd, char** argv, int argc) 
{
    int ret;
    int arg1, arg2;

    /* convert to int */
    arg1 = strtoul(argv[0], NULL, 10); // 10 = decimal
    arg2 = strtoul(argv[1], NULL, 10); 
 
    printf("hdoip_cli_dummy2()\n");
    printf(" arg1 : %s (char)\n arg2 : %s (char)\n",argv[0], argv[1]);
    printf(" arg1 : %d (int)\n arg2 : %d (int)\n",arg1, arg2);

    return 0;
}


/* Command definitions */
const int cmd_cnt = 7;
const t_hdoip_cli_cmd_arr cmd_arr[] = {
        { "help",       0, hdoip_cli_help,      ""},
        { "canvas",     1, hdoip_cli_canvas,    "Resolution"},
        { "capture",   -1, hdoip_cli_capture,   "{none|jp2 [size]} outputfile"},
        { "load",       1, hdoip_cli_load,      "inputfile"},
        { "loop",       0, hdoip_cli_loop,      ""},
        { "osd",        1, hdoip_cli_osd,       "{on|off}"},
        { "fmt",       -1, hdoip_cli_fmt,       "{in|out} *{fmts}"},

        { "dummy",      0, hdoip_cli_dummy,     ""},
        { "dummy2",     2, hdoip_cli_dummy2,    "arg1 arg2"}
    };

