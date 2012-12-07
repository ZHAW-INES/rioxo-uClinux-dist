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
#include "rtsp_error.h"
#include "rtsp_parameter.h"
#include "version.h"

#define RTSP_PARAM_GW_VERSION   "GatewareVersion"
#define RTSP_PARAM_MAC_ADDR     "MacAddress"
#define RTSP_PARAM_SERIAL       "SerialNumber"
#define RTSP_PARAM_SW_VERSION   "SoftwareVersion"
#define RTSP_PARAM_SOPC_SYSID   "SopcSysid"

typedef int (f_rtsp_param_get)(t_rtsp_media *media, t_rtsp_buffer *buf);
typedef int (f_rtsp_param_set)(t_rtsp_media *media, t_rtsp_buffer *buf,
                               char *content, size_t n);

struct rtsp_parameter {
    const char *name;       // name
    bool media_needed;      // whether a media is needed
    f_rtsp_param_get *get;  // getter
    f_rtsp_param_set *set;  // setter, NULL for readonly parameters
};

static const struct rtsp_parameter rtsp_parameters[];

static int rtsp_param_get_help(t_rtsp_media *media UNUSED, t_rtsp_buffer *buf)
{
    const struct rtsp_parameter *param;
    bool first;

    rtsp_buffer_printf(buf, "GET_PARAMETER: ");
    for (param = rtsp_parameters, first = true; param->name != NULL; param++) {
        if (!param->get)
            continue;
        if (!first)
            rtsp_buffer_printf(buf, ",");
        rtsp_buffer_printf(buf, "%s", param->name);
        if (first)
            first = !first;
    }
    rtsp_buffer_printf(buf, "\r\n");

    rtsp_buffer_printf(buf, "SET_PARAMETER: ");
    for (param = rtsp_parameters, first = true; param->name != NULL; param++) {
        if (!param->set)
            continue;
        if (!first)
            rtsp_buffer_printf(buf, ",");
        rtsp_buffer_printf(buf, "%s", param->name);
        if (first)
            first = !first;
    }
    rtsp_buffer_printf(buf, "\r\n");

    return 0;
}

static int rtsp_param_get_gw_version(t_rtsp_media *media UNUSED, t_rtsp_buffer *buf)
{
    t_hoic_getversion cmd;
    struct tm *t;
    char tbuf[128];

    hoi_drv_getversion(&cmd);
    t = localtime((time_t *) &cmd.fpga_date);
    strftime(tbuf, sizeof(tbuf), "%x %X", t);

    rtsp_buffer_printf(buf, RTSP_PARAM_GW_VERSION ": %s (SVN: %d)\r\n", tbuf, cmd.fpga_svn);
    return 0;
}

static int rtsp_param_get_mac_addr(t_rtsp_media *media UNUSED, t_rtsp_buffer *buf)
{
    rtsp_buffer_printf(buf, RTSP_PARAM_MAC_ADDR ": %s\r\n", reg_get("system-mac"));
    return 0;
}

static int rtsp_param_get_serial(t_rtsp_media *media UNUSED, t_rtsp_buffer *buf)
{
    rtsp_buffer_printf(buf, RTSP_PARAM_SERIAL ": %s\r\n", reg_get("serial-number"));
    return 0;
}

static int rtsp_param_get_sw_version(t_rtsp_media *media UNUSED, t_rtsp_buffer *buf)
{
    rtsp_buffer_printf(buf, RTSP_PARAM_SW_VERSION ": %d.%d\r\n", VERSION_MAJOR, VERSION_MINOR);
    return 0;
}

static int rtsp_param_get_sopc_sysid(t_rtsp_media *media UNUSED, t_rtsp_buffer *buf)
{
    t_hoic_getversion cmd;
    struct tm *t;
    char tbuf[128];

    hoi_drv_getversion(&cmd);
    t = localtime((time_t *) &cmd.sysid_date);
    strftime(tbuf, sizeof(tbuf), "%x %X", t);

    rtsp_buffer_printf(buf, RTSP_PARAM_SOPC_SYSID ": %s (Id: 0x%08x)\r\n", tbuf, cmd.sysid_id);
    return 0;
}

static int rtsp_get_parameter(t_rtsp_media *media, t_rtsp_buffer *repbuf,
                              const char *name)
{
    const struct rtsp_parameter *param;

    report(">>> GET_PARAMETER %s", name);

    for (param = rtsp_parameters; param->name != NULL; param++) {
        size_t len = strlen(param->name);

        if (param->media_needed && media == NULL)
            continue;
        if (strlen(name) != len)
            continue;
        if (strncmp(param->name, name, len) == 0 && param->get)
            return param->get(media, repbuf);
    }

    // TODO: How to handle inexistent parameters? Best bet would be to
    // just ignore the request

    return RTSP_UNSUPPORTED_PARAMETER;
}

int rtsp_handle_get_parameter(t_rtsp_media *media, t_rtsp_connection *con)
{
    char *line;
    int n;
    int ret;
    size_t sz = 0;
    ssize_t rem = con->common.content_length;
    t_rtsp_buffer *repbuf;
    size_t content_length;

    repbuf = malloc(sizeof(t_rtsp_buffer));
    if (!repbuf)
	    return -1;

    rtsp_buffer_init(repbuf);

    while (rem > 0) {
        /* TODO: handle multiple lines properly in rtsp_receive */
        n = rtsp_receive(con, &line, 0, 0, &sz);
        if (n != RTSP_SUCCESS) {
            report(ERROR "failed to read RTSP body: %d", n);
            break;
        }

        rem -= sz;

        /* The specified content-length was too short, we shouldn't handle the
         * extracted line and discard the whole rest of the message */
        if (rem < 0) {
            char *buf;
            size_t sz;

            report(ERROR "Provided Content-Length too short\n");

            /* read rest of message (disregarding Content-Length) */
            while (rtsp_receive(con, &buf, 0, 1, &sz) == 0)
                /* empty */ ;
            break;
        }

        if (!line || *line == '\0')
            break;

        ret = rtsp_get_parameter(media, repbuf, line);
        if (ret != RTSP_SUCCESS)
            report(ERROR "Failed to get parameter %s", line); // TODO: Anything else?
    }

    content_length = (size_t) (repbuf->eol - repbuf->sol);

    /* Actually construct and send the reply */
    rtsp_response_line(con, RTSP_STATUS_OK, "OK");
    if (content_length > 0) {
	    msgprintf(con, "Content-Type: text/parameters\r\n");
	    msgprintf(con, "Content-Length: %zu\r\n", content_length);
    }
    rtsp_eoh(con);  // End of header

    repbuf->eol = '\0';
    msgprintf(con, "%s", repbuf->buf);

    free(repbuf);

    return 0;
}

static const struct rtsp_parameter rtsp_parameters[] = {
    { "Help",                   false,	rtsp_param_get_help,        NULL },
    { RTSP_PARAM_GW_VERSION,    false,	rtsp_param_get_gw_version,  NULL },
    { RTSP_PARAM_MAC_ADDR,      false,	rtsp_param_get_mac_addr,    NULL },
    { RTSP_PARAM_SERIAL,        false,	rtsp_param_get_serial,      NULL },
    { RTSP_PARAM_SW_VERSION,    false,	rtsp_param_get_sw_version,  NULL },
    { RTSP_PARAM_SOPC_SYSID,    false,	rtsp_param_get_sopc_sysid,  NULL },
    { NULL,                     false,	NULL,                       NULL },
};
