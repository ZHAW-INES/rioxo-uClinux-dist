/*
 * rscp_server.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "map.h"
#include "rscp_net.h"
#include "rscp_server.h"
#include "rscp_string.h"
#include "rscp_parse_header.h"
#include "hdoipd.h"

t_rscp_server* rscp_server_create(int fd, uint32_t addr)
{
    static int nr = 1;
    t_rscp_server* server = malloc(sizeof(t_rscp_server));
    if (server) {
        memset(server, 0, sizeof(t_rscp_server));
        rscp_coninit(&server->con, fd, addr);
        server->nr = nr++;
        server->kill = false;
        report(" + RSCP Server [%d] for %s", server->nr, str_ntoa(addr));
    } else {
        report(ERROR "rscp_server_create.malloc: out of memory");
    }
    return server;
}

/** process incoming RSCP messages
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
int rscp_server_thread(t_rscp_server* handle)
{
    int n;
    t_rscp_header_common common;
    const t_map_set* method;
    t_rscp_media *media, *smedia;
    u_rscp_header buf;

    report(INFO "rscp server started");

    // receive request line
    while (1) {

        n = rscp_parse_request(&handle->con, srv_method, &method, &buf, &common);

        // connection closed...
        if (n) {
        	break;
        }

        // find media
        media = rscp_listener_get_media(handle->owner, common.uri.name);

        // fail response
        if (!media) {
            rscp_response_error(&handle->con, 462, "Destination unreachable");
            continue;
        }

        // get media session
        smedia = rscp_listener_get_session(handle->owner, common.session);
        if (smedia) media = smedia;

        // TODO: receive body

        lock("rscp_server_thread");

        media->creator = handle;

#ifdef REPORT_RSCP
        report(" < RSCP Server [%d] %s", handle->nr, common.rq.method);
#endif

        // process request (function responses for itself)
        n = ((frscpm*)method->fnc)(media, &buf, &handle->con);
        // media may be not valid anymore!
        if (n != RSCP_SUCCESS) {
            report(" ? execute method \"%s\" error (%d)", common.rq.method, n);
            unlock("rscp_server_thread");
            break;
        }

        unlock("rscp_server_thread");

    }

    handle->kill = true;

    report(" - RSCP Server [%d] ended", handle->nr);

    return n;
}


/** Teardown connection
 *
 * Note: caller must remove session
 *
 */
void rscp_server_teardown(t_rscp_media* media)
{
    t_rscp_server* server = media->creator;
    char *uri = "rscp://255.255.255.255:65536/";
    struct in_addr a1; a1.s_addr = server->con.address;
    sprintf(uri, "rscp://%s/", inet_ntoa(a1));

    if (server) {
        // a server connection is active for this media -> use it to send a teardown message
        rscp_request_teardown(&server->con, uri, media->sessionid);
    }

    rscp_server_close(media);
}

/** Close connection
 *
 * Note: caller must remove session
 *
 */
void rscp_server_close(t_rscp_media* media)
{
    t_rscp_server* server = media->creator;

    if ((server) && (server->con.fdr != -1)) {
        // a server connection is active for this media -> shut it down
        report(DEL "RSCP Server [%d] close %s:%s", server->nr, media->name, media->sessionid);

        if (shutdown(server->con.fdr, SHUT_RDWR) == -1) {
            report(ERROR "close socket error: %s", strerror(errno));
        }
        close(server->con.fdr);
        server->con.fdr = -1;
    }
}

void rscp_server_update(t_rscp_media* media, uint32_t event)
{
    t_rscp_server* server = media->creator;
    char *uri = "rscp://255.255.255.255:65536/";
    struct in_addr a1; a1.s_addr = server->con.address;
    sprintf(uri, "rscp://%s/", inet_ntoa(a1));

#ifdef REPORT_RSCP
    report(" > RSCP Server [%d] UPDATE", server->nr);
#endif

    if (server) {
        rscp_request_update(&server->con, uri, media->sessionid, event);
    }
}

void rscp_server_pause(t_rscp_media* media)
{
    t_rscp_server* server = media->creator;
    char *uri = "rscp://255.255.255.255:65536/";
    struct in_addr a1; a1.s_addr = server->con.address;
    sprintf(uri, "rscp://%s/", inet_ntoa(a1));

    if (server) {
        rscp_request_pause(&server->con, uri, media->sessionid);
    }

    rmsr_pause(media, 0, 0);
}

