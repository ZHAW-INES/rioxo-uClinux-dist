/*
 * rtsp_server.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "map.h"
#include "rtsp_net.h"
#include "rtsp_server.h"
#include "rtsp_string.h"
#include "rtsp_parse_header.h"
#include "hdoipd.h"
#include "hoi_drv_user.h"


t_rtsp_server* rtsp_server_create(int fd, uint32_t addr)
{
    static int nr = 1;
    t_rtsp_server* server = malloc(sizeof(t_rtsp_server));
    if (server) {
        memset(server, 0, sizeof(t_rtsp_server));
        rtsp_coninit(&server->con, fd, addr);
        server->nr = nr++;
        server->kill = false;
#ifdef REPORT_RTSP_SERVER
        report(" + RTSP Server [%d] for %s", server->nr, str_ntoa(addr));
#endif
    } else {
        report(ERROR "rtsp_server_create.malloc: out of memory");
    }
    return server;
}

/** process incoming RTSP messages
 *
 * Format is the following:
 *  - Request Line [CRLF]
 *  - n * Header Line [CRLF]
 *  - [CRLF]
 *  - { Optional Body }
 *
 * if a format error is detected the function returns with an error code
 *
 */
int rtsp_server_thread(t_rtsp_server* handle)
{
    int n;
    t_rtsp_header_common common;
    const t_map_set* method;
    t_rtsp_media *media, *smedia;
    u_rtsp_header buf;

#ifdef REPORT_RTSP_SERVER
    report(INFO "rtsp server started");
#endif

    // receive request line
    while (1) {

        n = rtsp_parse_request(&handle->con, srv_method, &method, &buf, &common);

        // connection closed...

        if (n) {
        	break;
        }

        // find media
        media = rtsp_listener_get_media(handle->owner, common.uri.name);

        // fail response
        if (!media) {
            rtsp_response_error(&handle->con, 462, "Destination unreachable");
            continue;
        }

        // get media session
        smedia = rtsp_listener_get_session(handle->owner, common.session);
        if (smedia) media = smedia;

        // TODO: receive body

        lock("rtsp_server_thread");

        media->creator = handle;

#ifdef REPORT_RTSP
        report(" < RTSP Server [%d] %s", handle->nr, common.rq.method);
#endif
        // process request (function responses for itself)
        n = ((frtspm*)method->fnc)(media, &buf, &handle->con);
        // media may be not valid anymore!
        if (n != RTSP_SUCCESS) {
            report(" ? execute method \"%s\" error (%d)", common.rq.method, n);
            unlock("rtsp_server_thread");
            break;
        }

        unlock("rtsp_server_thread");

    }

    handle->kill = true;

#ifdef REPORT_RTSP_SERVER
    report(" - RTSP Server [%d] ended", handle->nr);
#endif
    return n;
}

/** Teardown connection
 *
 * Note: caller must remove session
 *
 */
void rtsp_server_teardown(t_rtsp_media* media)
{
    t_rtsp_server* server = media->creator;
    char *uri = "rtsp://255.255.255.255:65536/";
    struct in_addr a1; a1.s_addr = server->con.address;
    sprintf(uri, "rtsp://%s/", inet_ntoa(a1));

    if (server) {
        // a server connection is active for this media -> use it to send a teardown message
        rtsp_request_teardown(&server->con, uri, media->sessionid);
    }

    rtsp_server_close(media);
}

/** Close connection
 *
 * Note: caller must remove session
 *
 */
void rtsp_server_close(t_rtsp_media* media)
{
    t_rtsp_server* server = media->creator;

    if ((server) && (server->con.fdr != -1)) {
        // a server connection is active for this media -> shut it down
#ifdef REPORT_RTSP_SERVER
        report(DEL "RTSP Server [%d] close %s:%s", server->nr, media->name, media->sessionid);
#endif

        if (shutdown(server->con.fdr, SHUT_RDWR) == -1) {
            report(ERROR "close socket error: %s", strerror(errno));
        }
        close(server->con.fdr);
        server->con.fdr = -1;
    }
}

void rtsp_server_update(t_rtsp_media* media, uint32_t event)
{
    t_rtsp_server* server = media->creator;
    t_rtsp_rtp_format fmt;
    char *s;

    if (!server) {report(ERROR "rtsp_server_update: no server") return;}

    char *uri = "rtsp://255.255.255.255:65536/";
    struct in_addr a1; a1.s_addr = server->con.address;
    sprintf(uri, "rtsp://%s/", inet_ntoa(a1));

    s = reg_get("compress");
    if (strcmp(s, "jp2k") == 0) {
        fmt.compress = FORMAT_JPEG2000;
    } else {
        fmt.compress = FORMAT_PLAIN;
    }
    hoi_drv_get_mtime(&fmt.rtptime);
    fmt.value = reg_get_int("advcnt-min");

#ifdef REPORT_RTSP_UPDATE
    report(" > RTSP Server [%d] UPDATE", server->nr);
#endif

    if (server) {
        rtsp_request_update(&server->con, uri, media->sessionid, event, &fmt);
    }
}

void rtsp_server_pause(t_rtsp_media* media)
{
    t_rtsp_server* server = media->creator;

    if (!server) {report("rtsp server pause: no server"); return;}

    char *uri = "rtsp://255.255.255.255:65536/";
    struct in_addr a1; a1.s_addr = server->con.address;
    sprintf(uri, "rtsp://%s/", inet_ntoa(a1));

    if (server) {
        rtsp_request_pause(&server->con, uri, media->sessionid);
    }

    rmsr_pause(media, 0);
}

