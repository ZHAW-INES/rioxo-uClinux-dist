/*
 * RTSP Parameter getter/setter
 *
 *  Created on: 26.11.2012
 *      Author: klto
 */

#include <stdlib.h>
#include <time.h>

#include "hdoipd.h"
#include "hdoipd_msg.h"
#include "hoi_drv_user.h"
#include "rtsp_connection.h"
#include "rtsp_parameter.h"
#include "version.h"

#define RTSP_PARAM_GW_VERSION   "GatewareVersion"
#define RTSP_PARAM_SERIAL       "SerialNumber"
#define RTSP_PARAM_SW_VERSION   "SoftwareVersion"
#define RTSP_PARAM_SOPC_SYSID   "SopcSysid"

const struct rtsp_parameter rtsp_parameters[];

static int rtsp_param_get_help(t_rtsp_connection *con)
{
    const struct rtsp_parameter *param;
    bool first;

    msgprintf(con, "GET_PARAMETER: ");
    for (param = rtsp_parameters, first = true; param->name != NULL; param++) {
        if (!param->get)
            continue;
        if (!first)
            msgprintf(con, ",");
        msgprintf(con, param->name);
        if (first)
            first = !first;
    }
    msgprintf(con, "\r\n");

    msgprintf(con, "SET_PARAMETER: ");
    for (param = rtsp_parameters, first = true; param->name != NULL; param++) {
        if (!param->set)
            continue;
        if (!first)
            msgprintf(con, ",");
        msgprintf(con, param->name);
        if (first)
            first = !first;
    }
    msgprintf(con, "\r\n");

    return 0;
}

static int rtsp_param_get_gw_version(t_rtsp_connection *con)
{
    t_hoic_getversion cmd;
    struct tm *t;
    char tbuf[128];

    hoi_drv_getversion(&cmd);
    t = localtime((time_t *) &cmd.fpga_date);
    strftime(tbuf, sizeof(tbuf), "%x %X", t);

    msgprintf(con, RTSP_PARAM_GW_VERSION ": %s (SVN: %d)\r\n", tbuf, cmd.fpga_svn);
    return 0;
}

static int rtsp_param_get_serial(t_rtsp_connection *con)
{
    msgprintf(con, RTSP_PARAM_SERIAL ": %s\r\n", reg_get("serial-number"));
    return 0;
}

static int rtsp_param_get_sw_version(t_rtsp_connection *con)
{
    msgprintf(con, RTSP_PARAM_SW_VERSION ": %d.%d\r\n", VERSION_MAJOR, VERSION_MINOR);
    return 0;
}

static int rtsp_param_get_sopc_sysid(t_rtsp_connection *con)
{
    t_hoic_getversion cmd;
    struct tm *t;
    char tbuf[128];
    
    hoi_drv_getversion(&cmd);
    t = localtime((time_t *) &cmd.sysid_date);
    strftime(tbuf, sizeof(tbuf), "%x %X", t);

    msgprintf(con, RTSP_PARAM_SOPC_SYSID ": %s (Id: 0x%08x)\r\n", tbuf, cmd.sysid_id);
    return 0;
}

const struct rtsp_parameter rtsp_parameters[] = {
    { "Help",                   rtsp_param_get_help,        NULL },
    { RTSP_PARAM_GW_VERSION,    rtsp_param_get_gw_version,  NULL },
    { RTSP_PARAM_SERIAL,        rtsp_param_get_serial,      NULL },
    { RTSP_PARAM_SW_VERSION,    rtsp_param_get_sw_version,  NULL },
    { RTSP_PARAM_SOPC_SYSID,    rtsp_param_get_sopc_sysid,  NULL },
    { NULL,                     NULL,                       NULL },
};

t_rtsp_parameter *rtsp_parameter_lookup(const char *name)
{
    t_rtsp_parameter *ret = NULL;
    t_rtsp_parameter *param;

    for (param = (t_rtsp_parameter *) rtsp_parameters; param->name != NULL; param++) {
        size_t len = strlen(param->name);
        if (strncmp(param->name, name, len) == 0) {
            ret = param;
            break;
        }
    }

    return ret;
}
