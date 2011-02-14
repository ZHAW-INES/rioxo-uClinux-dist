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
        report(" + RSCP Server [%d] for %s", server->nr, str_ntoa(addr));
    }
    return server;
}

void rscp_server_free(t_rscp_server* server)
{
    report(" - RSCP Server [%d]", server->nr);
    free(server);
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

    report("rscp server started");

    // receive request line
    while (1) {

        n = rscp_parse_request(&handle->con, srv_method, &method, &buf, &common);

        // connection closed...
        if (n) return n;

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

        report(" < RSCP Server [%d] %s", handle->nr, common.rq.method);

        // process request (function responses for itself)
        n = ((frscpo*)method->fnc)(media, &buf, &handle->con, handle->owner);
        if (n != RSCP_SUCCESS) {
            report(" ? execute method \"%s\" error (%d)", common.rq.method, n);
            unlock("rscp_server_thread");
            return n;
        }

        unlock("rscp_server_thread");

    }

    report("rscp server ended");

    return RSCP_SUCCESS;
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

    rscp_request_teardown(&server->con, uri, media->sessionid);

    rmsr_teardown(media, 0, &server->con, server->owner);

    if (shutdown(server->con.fdr, SHUT_RDWR) == -1) {
        report("close socket error: %s", strerror(errno));
    }

    free(media);
}

void rscp_server_close(t_rscp_media* media)
{
    t_rscp_server* server = media->creator;

    rmsr_teardown(media, 0, &server->con, server->owner);

    if (shutdown(server->con.fdr, SHUT_RDWR) == -1) {
        report("close socket error: %s", strerror(errno));
    }

    free(media);
}

void rscp_server_update(t_rscp_media* media, uint32_t event)
{
    t_rscp_server* server = media->creator;
    char *uri = "rscp://255.255.255.255:65536/";
    struct in_addr a1; a1.s_addr = server->con.address;
    sprintf(uri, "rscp://%s/", inet_ntoa(a1));

    rscp_request_update(&server->con, uri, media->sessionid, event);

    rmsr_update(media, 0, &server->con);
}

void rscp_server_pause(t_rscp_media* media)
{
    t_rscp_server* server = media->creator;
    char *uri = "rscp://255.255.255.255:65536/";
    struct in_addr a1; a1.s_addr = server->con.address;
    sprintf(uri, "rscp://%s/", inet_ntoa(a1));

    rscp_request_pause(&server->con, uri, media->sessionid);

    rmsr_pause(media, 0, &server->con);
}

