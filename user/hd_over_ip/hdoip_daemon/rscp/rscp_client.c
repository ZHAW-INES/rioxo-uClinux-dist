/*
 * rscp_client.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include <stdio.h>
#include <pthread.h>
#include "rscp.h"
#include "rscp_media.h"
#include "rscp_net.h"
#include "rscp_string.h"
#include "rscp_client.h"
#include "rscp_command.h"
#include "rscp_parse_header.h"
#include "hdoipd.h"

int rscp_client_open(t_rscp_client* client, t_rscp_media *media, char* uri);
int rscp_client_close(t_rscp_client* client);

void* rscp_client_thread(void* _client);
void* rscp_client_req_thread(void* _client);

/** Opens a new rscp client connection
 *
 * !!! hdoipd must be locked !!! (because media is manipulated)
 */
int rscp_client_open(t_rscp_client* client, t_rscp_media *media, char* address)
{
    static int nr = 1;
    char buf[200];
    int fd, port;
    t_str_uri uri;
    struct hostent* host;
    struct in_addr addr;
    struct sockaddr_in dest_addr;

    if (client->media && (client->media != media)) {
        report(" ? RSCP Client has already media");
        return -1;
    }

    if (media) {
        if (media->creator && (media->creator != client)) {
            report(" ? RSCP Client - Media has already a client");
            return -1;
        }
    }

    client->nr = nr++;
    report(" + RSCP Client [%d] open %s", client->nr, address);

    memcpy(client->uri, address, strlen(address)+1);
    memcpy(buf, address, strlen(address)+1);

    if (!str_split_uri(&uri, buf)) {
        report("uri error: scheme://server[:port]/name");
        return -1;
    }

    host = gethostbyname(uri.server);

    if (!host) {
        report("gethostbyname failed");
        return -1;
    }

    addr.s_addr = *((uint32_t*)host->h_addr_list[0]);

    // default port 554
    if (uri.port) {
        port = htons(atoi(uri.port));
    } else {
        port = htons(554);
    }

    report(" i server name: %s - %s:%d", host->h_name, inet_ntoa(addr), ntohs(port));

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        report("client socket error: %s", strerror(errno));
        return -1;
    }

    // setup own address
    memset(&dest_addr, 0, sizeof(struct sockaddr_in));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = port;
    dest_addr.sin_addr.s_addr = addr.s_addr;

    if (connect(fd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr)) == -1) {
        close(fd);
        report(" - RSCP Client [%d]: connection refused", client->nr);

        if(media && media->error) media->error(media, NULL, NULL);
        return -1;
    }

    client->media = media;
    if (media) media->creator = client;

    rscp_coninit(&client->con, fd, addr.s_addr);

    if (media) {
        // start filter thread
        if (rscp_split(&client->con, &client->mux, &client->con2) == RSCP_SUCCESS) {
            unlock("rscp_client_split");
                pthread(client->th1, rscp_client_thread, client);
                pthread(client->th2, rscp_client_req_thread, client);
            lock("rscp_client_split");
        }
    }

    return RSCP_SUCCESS;
}

int rscp_client_close(t_rscp_client* client)
{
    shutdown(client->con.fdw, SHUT_RDWR);
    //if (client->media) close(client->mux.fdw);
    //if (client->media) close(client->con2.fdw);

    report(" i RSCP Client [%d] join pthread", client->nr);

    unlock("rscp_client_close");
        pthread_join(client->th1, 0);
        pthread_join(client->th2, 0);
    lock("rscp_client_close");

    report(" - RSCP Client [%d]", client->nr);

    if (client->media) {
        client->media->creator = 0;
        client->media = 0;
    }

    return RSCP_SUCCESS;
}

// returns response code
int rscp_client_setup(t_rscp_client* client, t_rscp_transport* transport, t_rscp_edid *edid)
{
    int n;
    u_rscp_header buf;
    t_rscp_header_common common;

    report(" > RSCP Client [%d] SETUP", client->nr);

    rscp_request_setup(&client->con, client->uri, transport, edid);

    rscp_default_response_setup((void*)&buf);

    // response
    n = rscp_parse_response(&client->con, tab_response_setup, (void*)&buf, &common);

    if (n == RSCP_SUCCESS) {
        // TODO:
        strcpy(client->media->sessionid, common.session);
        rmcr_setup(client->media, (void*)&buf, &client->con);
    } else if (n == RSCP_RESPONSE_ERROR) {
        if (client->media->error) client->media->error(client->media, (void*)n, &client->con);
    } else {
        report("internal failure (%d)", n);
    }

    return n;
}

int rscp_client_play(t_rscp_client* client, t_rscp_rtp_format* fmt)
{
    int n;
    u_rscp_header buf;

    report(" > RSCP Client [%d] PLAY", client->nr);

    rscp_request_play(&client->con, client->uri, client->media->sessionid, fmt);

    rscp_default_response_play((void*)&buf);

    // response
    n = rscp_parse_response(&client->con, tab_response_play, (void*)&buf, 0);
    if (n == RSCP_SUCCESS) {
        rmcr_play(client->media, (void*)&buf, &client->con);
    } else if (n == RSCP_RESPONSE_ERROR) {
        if (client->media->error) client->media->error(client->media, (void*)n, &client->con);
    } else {
        report("internal failure (%d)", n);
    }

    return n;
}

int rscp_client_teardown(t_rscp_client* client)
{
    u_rscp_header buf;

    report(" > RSCP Client [%d] TEARDOWN", client->nr);

    // request teardown
    rscp_request_teardown(&client->con, client->uri, client->media->sessionid);

    rscp_default_response_teardown((void*)&buf);

    // response
    rscp_parse_response(&client->con, tab_response_teardown, (void*)&buf, 0);
    rmcr_teardown(client->media, (void*)&buf, &client->con);

    rscp_client_remove(client);

    return RSCP_SUCCESS;
}

int rscp_client_kill(t_rscp_client* client)
{
    report(" > RSCP Client [%d] kill", client->nr);

    // response
    rmcr_teardown(client->media, 0, &client->con);

    rscp_client_remove(client);

    return RSCP_SUCCESS;
}

int rscp_client_update(t_rscp_client* client, uint32_t event)
{
    rscp_request_update(&client->con, client->uri, client->media->sessionid, event);

    return RSCP_SUCCESS;
}

int rscp_client_hello(t_rscp_client* client)
{
    report(" > RSCP Client [%d] HELLO", client->nr);

    rscp_request_hello(&client->con, client->uri);

    return RSCP_SUCCESS;
}

bool rscp_client_active(t_rscp_client* client)
{
    if (!client) return false;
    if (!client->media) return false;
    return (client->media->state != RSCP_INIT);
}


bool rscp_client_inuse(t_rscp_client* client)
{
    if (!client) return false;
    if (!client->media) return false;
    return true;
}

extern const t_map_fnc tab_request_update[];
void* rscp_client_thread(void* _client)
{
    int n;
    char* line, *tst;
    t_rscp_client* client = _client;

    //lock("rscp_client_thread-start");
        report(" + RSCP Client [%d] filter", client->nr);
    //unlock("rscp_client_thread-start");

    while ((n = rscp_receive(&client->mux, &line)) == RSCP_SUCCESS) {
        tst = line;
        if (!str_starts_with(&tst, RSCP_VERSION)) {
            rscp_msginit(&client->con2);
            do {
                msgprintf(&client->con2, "%s\r\n", line);
                if (*line == 0) break;
            } while ((n = rscp_receive(&client->mux, &line)) == RSCP_SUCCESS);
            if (n != RSCP_SUCCESS) {
                report("rscp client filter receive error on request");
                break;
            }
            rscp_send(&client->con2);
        } else {
            rscp_msginit(&client->mux);
            do {
                msgprintf(&client->mux, "%s\r\n", line);
                if (*line == 0) break;
            } while ((n = rscp_receive(&client->mux, &line)) == RSCP_SUCCESS);
            if (n != RSCP_SUCCESS) {
                report("rscp client filter receive error on response");
                break;
            }
            rscp_send(&client->mux);
        }
    }

    //lock("rscp_client_thread-close");
        report(" - RSCP Client [%d] filter: (%d) %s", client->nr, n, strerror(errno));

        close(client->mux.fdw);
        close(client->con2.fdw);
    //unlock("rscp_client_thread-close");

    return 0;
}

void* rscp_client_req_thread(void* _client)
{
    int n = 0;
    t_rscp_client* client = _client;
    t_rscp_header_common common;
    const t_map_set* method;
    u_rscp_header buf;

    lock("rscp_client_req_thread-start");
        report(" + RSCP Client [%d] request ep", client->nr);
    unlock("rscp_client_req_thread-start");

    while (1) {
        n = rscp_parse_request(&client->con2, client_method, &method, (void*)&buf, &common);

        // connection closed...
        if (n) break;

        lock("rscp_client_req_thread");

#ifdef REPORT_RSCP
        report(" < RSCP Client [%d] %s", client->nr, common.rq.method);
#endif

        // process request (function responses for itself)
        n = ((frscpo*)method->fnc)(client->media, (void*)&buf, &client->con, 0);
        if (n != RSCP_SUCCESS) {
            report(" ? execute method \"%s\" error (%d)", common.rq.method, n);
            unlock("rscp_client_req_thread");
            break;
        }

        unlock("rscp_client_req_thread");

    }

    lock("rscp_client_req_thread-close");
        report(" - RSCP Client [%d] request ep", client->nr);
    unlock("rscp_client_req_thread-close");

    return (void*)n;
}

t_rscp_client* rscp_client_add(t_node* list, t_rscp_media *media, char* address)
{
    t_rscp_client* client = malloc(sizeof(t_rscp_client));

    if (!client) return 0;

    memset(client, 0, sizeof(t_rscp_client));
    client->idx = list_add(list, client);

    if ((rscp_client_open(client, media, address))) {
        list_remove(client->idx);
        free(client);
        client = 0;
    }

    return client;
}

int rscp_client_remove(t_rscp_client* client)
{
    int ret;

    if (!client) return -1;

    list_remove(client->idx);
    ret = rscp_client_close(client);
    free(client);

    return ret;
}


void rscp_client_deactivate(t_node* list)
{
    t_rscp_client* client;

    while ((client = list_peek(list))) {
        if (rscp_client_active(client)) {
            // proper teradown
            rscp_client_teardown(client);
        } else {
            // delete client
            rscp_client_remove(client);
        }
    }
}

void rscp_client_event(t_node* list, uint32_t event)
{
    t_rscp_client* client;

    LIST_FOR(client, list) {
        if (client->media) rscp_media_event(client->media, event);
    }
}

void rscp_client_force_close(t_node* list)
{
    t_rscp_client* client;

    while ((client = list_peek(list))) {
        if (rscp_client_inuse(client)) {
            rmcr_teardown(client->media, 0, &client->con);
        }
        rscp_client_remove(client);
    }
}

