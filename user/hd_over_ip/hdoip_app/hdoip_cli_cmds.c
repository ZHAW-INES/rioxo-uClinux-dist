#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

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
int hdoip_cli_help(int fd, int fdr, char** argv, int argc)
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

int hdoip_cli_canvas(int fd, int fdr, char** argv, int argc)
{
    uint32_t width, height, fps;
    //printf("hdoip_cli_canvas\n");

    if (!(hdoip_lib_res(argv[0], &width, &height, &fps))) {
        hoic_canvas(fd, width, height, fps);
    }

    return 0;
}

int hdoip_cli_capture(int fd, int fdr, char** argv, int argc)
{
    //printf("hdoip_cli_capture\n");

    if(strcmp(argv[0], "none") == 0) {
        hoic_capture(fd, false, 0, argv[1]);
    } else if(strcmp(argv[0], "jp2") == 0) {
        hoic_capture(fd, true, atoi(argv[1]), argv[2]);
    } else {
        printf("format not supported\n");
    }

    return 0;
}

int hdoip_cli_osd(int fd, int fdr, char** argv, int argc)
{
    if(strcmp(argv[0], "on") == 0) {
        hoic_osd_on(fd);
    } else if(strcmp(argv[0], "off") == 0) {
        hoic_osd_off(fd);
    } else {
        printf("use: hdoip osd {on|off}\n");
    }

    return 0;
}

int hdoip_cli_hpd(int fd, int fdr, char** argv, int argc)
{
    if(strcmp(argv[0], "on") == 0) {
        hoic_hpd_on(fd);
    } else if(strcmp(argv[0], "off") == 0) {
        hoic_hpd_off(fd);
    } else {
        printf("use: hdoip hpd {on|off}\n");
    }

    return 0;
}

int hdoip_cli_load(int fd, int fdr, char** argv, int argc)
{
    //printf("hdoip_cli_load\n");

    hoic_load(fd, argv[0]);
    return 0;
}

int hdoip_cli_loop(int fd, int fdr, char** argv, int argc)
{
    //printf("hdoip_cli_loop\n");

    hoic_loop(fd);

    return 0;
}

int hdoip_cli_vtb(int fd, int fdr, char** argv, int argc)
{
    //printf("hdoip_cli_vtb\n");

    hoic_vtb(fd);

    return 0;
}

int hdoip_cli_vrb_setup(int fd, int fdr, char** argv, int argc)
{
    //printf("hdoip_cli_vrb_setup\n");

    hoic_vrb_setup(fd, argv[0]);

    return 0;
}

int hdoip_cli_vrb_play(int fd, int fdr, char** argv, int argc)
{
    //printf("hdoip_cli_vrb_play\n");

    hoic_vrb_play(fd);

    return 0;
}

int hdoip_cli_vrb_pause(int fd, int fdr, char** argv, int argc)
{
    //printf("hdoip_cli_vrb_pause\n");

    hoic_vrb_pause(fd);

    return 0;
}

int hdoip_cli_vrb_pause_play(int fd, int fdr, char** argv, int argc)
{
    //printf("hdoip_cli_vrb_pause_play\n");

    hoic_vrb_pause_play(fd);

    return 0;
}

int hdoip_cli_ready(int fd, int fdr, char** argv, int argc)
{
    //printf("hdoip_cli_vrb_play\n");

    hoic_ready(fd);

    return 0;
}

int hdoip_cli_reboot(int fd, int fdr, char** argv, int argc)
{
    hoic_reboot(fd);

    return 0;
}

int hdoip_cli_store(int fd, int fdr, char** argv, int argc)
{
    hoic_store_cfg(fd);
    return 0;
}

int hdoip_cli_fmt(int fd, int fdr, char** argv, int argc)
{
    int fmt = 0;
    //printf("hdoip_cli_fmt\n");

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

int hdoip_cli_set(int fd, int fdr, char** argv, int argc)
{
    hoic_set_param(fd, argv[0], argv[1]);
    return 0;
}

int hdoip_cli_get(int fd, int fdr, char** argv, int argc)
{
    char* s = hoic_get_param(fd, fdr, argv[0]);
    printf("return: \"%s\"\n", s);
    free(s);
    return 0;
}

int hdoip_cli_read(int fd, int fdr, char** argv, int argc)
{
    uint32_t s = hoic_read(fd, fdr, strtol(argv[0], 0, 16));
    printf("return: %08x\n", s);
    return 0;
}

int hdoip_cli_remote_update(int fd, int fdr, char** argv, int argc)
{
    hoic_remote_update(fd, argv[0]);
    return 0;
}

int hdoip_cli_get_edid(int fd, int fdr, char** argv, int argc)
{
    hoic_get_edid(fd, argv[0]);
    return 0;
}

int hdoip_cli_getversion(int fd, int fdr, char** argv, int argc)
{
    t_hoic_version version;

    hoic_getversion(fd, fdr, &version);

    printf("\n FPGA date        : %s", ctime(&version.fpga_date));
    printf(" FPGA SVN         : %d\n", version.fpga_svn);
    printf(" SOPC date        : %s", ctime(&version.sysid_date));
    printf(" SOPC ID          : 0x%08x\n", version.sysid_id);
    printf(" Software version : %d.%d\n", (version.sw_version>>16), (version.sw_version&0xFFFF));
    printf(" Software tag     : %s\n\n", version.sw_tag);
    return 0;
}

int hdoip_cli_default(int fd, int fdr, char** argv, int argc)
{
    hoic_factory_default(fd); 
    return 0;
}

int hdoip_cli_debug(int fd, int fdr, char** argv, int argc)
{
    hoic_debug(fd);
    return 0;
}

/* Command definitions */
const t_hdoip_cli_cmd_arr cmd_arr[] = {
        { "help",           0, hdoip_cli_help,          ""},
        { "canvas",         1, hdoip_cli_canvas,        "Resolution"},
        { "capture",       -1, hdoip_cli_capture,       "{none|jp2 [size]} outputfile"},
        { "load",           1, hdoip_cli_load,          "inputfile"},
        { "loop",           0, hdoip_cli_loop,          ""},
        { "osd",            1, hdoip_cli_osd,           "{on|off}"},
        { "hpd",            1, hdoip_cli_hpd,           "{on|off}"},
        { "fmt",           -1, hdoip_cli_fmt,           "{in|out} *{fmts}"},
        { "vtb",            0, hdoip_cli_vtb,           ""},
        { "vrb",            1, hdoip_cli_vrb_setup,     "uri"},
        { "play",           0, hdoip_cli_vrb_play,      ""},
        { "pause",          0, hdoip_cli_vrb_pause,     ""},
        { "pause-play",     0, hdoip_cli_vrb_pause_play,""},
        { "set",            2, hdoip_cli_set,           "name value"},
        { "get",            1, hdoip_cli_get,           "name"},
        { "ready",          0, hdoip_cli_ready,         ""},
        { "store",          0, hdoip_cli_store,         ""},
        { "reboot",         0, hdoip_cli_reboot,        ""},
        { "read",           1, hdoip_cli_read,          "hex-address"},
        { "remote-update",  1, hdoip_cli_remote_update, "file"},
        { "version",        0, hdoip_cli_getversion,    ""},
        { "factory-default",0, hdoip_cli_default,       ""},
        { "debug"          ,0, hdoip_cli_debug,         ""},
        { "get-edid"       ,1, hdoip_cli_get_edid,      "file"},
    };
const int cmd_cnt = sizeof(cmd_arr)/sizeof(t_hdoip_cli_cmd_arr);
