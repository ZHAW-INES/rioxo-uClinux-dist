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


void* rscp_client_thread(void* _client);
void* rscp_client_req_thread(void* _client);


/** Opens a new rscp client connection
 *
 * !!! hdoipd must be locked !!! (because media is manipulated)
 */
t_rscp_client* rscp_client_open(t_node* list, t_rscp_media *media, char* address)
{
    static int nr = 1;
    char buf[200];
    int fd, port, ret = RSCP_SUCCESS;
    t_str_uri uri;
    struct hostent* host;
    struct in_addr addr;
    struct sockaddr_in dest_addr;

    if (media) if (rscp_media_active(media)) return 0;

    t_rscp_client* client = malloc(sizeof(t_rscp_client));
    if (!client) {
        report(ERROR "rscp_client_open.malloc: out of memory");
        return 0;
    }

    memset(client, 0, sizeof(t_rscp_client));
    client->task = 0;

    if (media) {
        if (media->creator) {
            report(" ? RSCP Client - Media has already a client");
            ret = RSCP_CLIENT_ERROR;
        }
    }

    if (ret == RSCP_SUCCESS) {
        client->nr = nr++;
        report(" + RSCP Client [%d] open %s", client->nr, address);

        memcpy(client->uri, address, strlen(address)+1);
        memcpy(buf, address, strlen(address)+1);

        if (!str_split_uri(&uri, buf)) {
            report("uri error: scheme://server[:port]/name");
            ret = RSCP_CLIENT_ERROR;
        }
    }

    if (ret == RSCP_SUCCESS) {
        host = gethostbyname(uri.server);

        if (!host) {
            report("gethostbyname failed");
            ret = RSCP_CLIENT_ERROR;
        }
    }

    if (ret == RSCP_SUCCESS) {
        addr.s_addr = *((uint32_t*)host->h_addr_list[0]);

        // default port 554
        if (uri.port) {
            port = htons(atoi(uri.port));
        } else {
            port = htons(554);
        }

        report(" i server name: %s - %s:%d", host->h_name, inet_ntoa(addr), ntohs(port));

        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            report(ERROR "RSCP Client [%d]: socket error: %s", client->nr, strerror(errno));
            ret = RSCP_CLIENT_ERROR;
        }
    }

    if (ret == RSCP_SUCCESS) {
        // setup own address
        memset(&dest_addr, 0, sizeof(struct sockaddr_in));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = port;
        dest_addr.sin_addr.s_addr = addr.s_addr;        // remote address

        if (connect(fd, (struct sockaddr_in*)&dest_addr, sizeof(struct sockaddr_in)) == -1) {
            close(fd);
            perrno(ERROR "RSCP Client [%d]: connection refused", client->nr);
            ret = RSCP_CLIENT_ERROR;
        }
    }

    if (ret == RSCP_SUCCESS) {
        rscp_coninit(&client->con, fd, addr.s_addr);

        if (media) {
            ret = rscp_split(&client->con, &client->con1, &client->con2);
        }
    }

    // attach client to media
    if (ret == RSCP_SUCCESS) {
        // attach client & media
        client->media = media;
        if (media) media->creator = client;

        // add to client list
        if (list) client->idx = list_add(list, client);

        if (media) {
            unlock("rscp_client_split");
                pthread(client->th1, rscp_client_thread, client);
                pthread(client->th2, rscp_client_req_thread, client);
            lock("rscp_client_split");
        }
    } else {
        free(client);
        client = 0;
        // tell media open failed
        if (media && media->error) media->error(media, 0, 0);
    }

    return client;
}


/** Closes and removes a rscp client connection
 *
 * !!! hdoipd must be locked !!! (because client-list is manipulated)
 */
int rscp_client_close(t_rscp_client* client)
{
    if (!client) return RSCP_NULL_POINTER;

    // remove client
    if (client->idx) {
        list_remove(client->idx);
        client->idx = 0;
    } else {
        report(ERROR "RSCP Client [%d] already closed", client->nr);
        return RSCP_NULL_POINTER;
    }

    client->task = E_RSCP_CLIENT_KILL;
    shutdown(client->con.fdw, SHUT_RDWR);
    close(client->con.fdw);

    // detach client from media
    if (client->media) {
        report(" i RSCP Client [%d] detach media %s", client->nr, client->media->name);
        rmcr_teardown(client->media, 0, 0);
        client->media->creator = 0;
        client->media = 0;

        report(" i RSCP Client [%d] join pthread", client->nr);

        unlock("rscp_client_close");
            pthread_join(client->th1, 0);
            pthread_join(client->th2, 0);
        lock("rscp_client_close");

        close(client->con.fdr);
        close(client->con2.fdr);
    }

    report(" - RSCP Client [%d]", client->nr);

    free(client);

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
    n = rscp_parse_response(&client->con, tab_response_setup, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    if (n == RSCP_SUCCESS) {
        // TODO:
        strcpy(client->media->sessionid, common.session);
        rmcr_setup(client->media, (void*)&buf, &client->con);
    } else if (n == RSCP_RESPONSE_ERROR) {
        if (client->media->error) client->media->error(client->media, (void*)n, &client->con);
    } else {
        perrno("internal failure (%d)", n);
    }

    return n;
}

int rscp_client_set_play(t_rscp_client* client)
{
    client->task |= E_RSCP_CLIENT_PLAY;

    return RSCP_SUCCESS;
}

int rscp_client_play(t_rscp_client* client, t_rscp_rtp_format* fmt)
{
    int n;
    u_rscp_header buf;

    report(" > RSCP Client [%d] PLAY", client->nr);

    rscp_request_play(&client->con, client->uri, client->media->sessionid, fmt);

    rscp_default_response_play((void*)&buf);

    // response
    n = rscp_parse_response(&client->con, tab_response_play, (void*)&buf, 0, CFG_RSP_TIMEOUT);
    if (n == RSCP_SUCCESS) {
        rmcr_play(client->media, (void*)&buf, &client->con);
    } else if (n == RSCP_RESPONSE_ERROR) {
        if (client->media->error) client->media->error(client->media, (void*)n, &client->con);
    } else {
        perrno("internal failure (%d)", n);
    }

    return n;
}

int rscp_client_set_teardown(t_rscp_client* client)
{
	client->task |= E_RSCP_CLIENT_TEARDOWN;

	return RSCP_SUCCESS;
}

int rscp_client_teardown(t_rscp_client* client)
{
    u_rscp_header buf;
    int nr = client->nr;

    report(" > RSCP Client [%d] TEARDOWN", client->nr);

    // request teardown
    rscp_request_teardown(&client->con, client->uri, client->media->sessionid);

    rscp_default_response_teardown((void*)&buf);

    // response
    rscp_parse_response(&client->con, tab_response_teardown, (void*)&buf, 0, CFG_RSP_TIMEOUT);
    rmcr_teardown(client->media, (void*)&buf, &client->con);

    rscp_client_close(client);

    return RSCP_SUCCESS;
}

int rscp_client_set_kill(t_rscp_client* client)
{
	client->task |= E_RSCP_CLIENT_KILL;

	return RSCP_SUCCESS;
}

int rscp_client_kill(t_rscp_client* client)
{
    report(" > RSCP Client [%d] kill", client->nr);

    // response
    rmcr_teardown(client->media, 0, &client->con);

    rscp_client_close(client);

    return RSCP_SUCCESS;
}

int rscp_client_update(t_rscp_client* client, uint32_t event)
{
#ifdef REPORT_RSCP
    report(" > RSCP Client [%d] UPDATE", client->nr);
#endif

    rscp_request_update(&client->con, client->uri, client->media->sessionid, event);

    return RSCP_SUCCESS;
}


int rscp_client_hello(t_rscp_client* client)
{
    report(" > RSCP Client [%d] HELLO", client->nr);

    rscp_request_hello(&client->con, client->uri);

    return RSCP_SUCCESS;
}


void rscp_client_deactivate(t_node* list)
{
    t_rscp_client* client;

    while ((client = list_peek(list))) {
        if (client->media->state != RSCP_INIT) {
            // proper teradown
            rscp_client_teardown(client);
        } else {
            // delete client
            rscp_client_close(client);
        }
    }
}

void rscp_client_event(t_node* list, uint32_t event)
{
    t_rscp_client* client;
    node_anchor(task_list);

    LIST_FOR(client, list) {
        if (client->media) rscp_media_event(client->media, event);
        if (client->task)  queue_put(&task_list, client);
    }

    while(client = queue_get(&task_list)) {
        if(client->task & E_RSCP_CLIENT_KILL) rscp_client_close(client);
        else if(client->task & E_RSCP_CLIENT_TEARDOWN) rscp_client_teardown(client);
        else if(client->task & E_RSCP_CLIENT_PLAY) rscp_media_play(client->media);
        client->task = 0;
    }
}

void rscp_client_force_close(t_node* list)
{
    t_rscp_client* client;
    while ((client = list_peek(list))) {
        rscp_client_close(client);
    }
}


void* rscp_client_thread(void* _client)
{
    int n;
    char* line, *tst;
    t_rscp_client* client = _client;

    report(" + RSCP Client [%d] filter", client->nr);

    while ((n = rscp_receive(&client->con1, &line, 0)) == RSCP_SUCCESS) {
        tst = line;
        if (!str_starts_with(&tst, RSCP_VERSION)) { // if response

            do {
                msgprintf(&client->con2, "%s\r\n", line);
                if (*line == 0) break;
            } while ((n = rscp_receive(&client->con1, &line, 0)) == RSCP_SUCCESS);
            if (n != RSCP_SUCCESS) {
                report("rscp client filter receive error on request");
                break;
            }
            rscp_write(&client->con2);

        } else { // if request

            do {
                msgprintf(&client->con1, "%s\r\n", line);
                if (*line == 0) break;
            } while ((n = rscp_receive(&client->con1, &line, 0)) == RSCP_SUCCESS);
            if (n != RSCP_SUCCESS) {
                report("rscp client filter receive error on response");
                break;
            }
            rscp_write(&client->con1);

        }
    }

    client->task = E_RSCP_CLIENT_KILL;

    report(" - RSCP Client [%d] filter: (%d) %s", client->nr, n, strerror(errno));

    close(client->con1.fdw);
    close(client->con2.fdw);

    return 0;
}

void* rscp_client_req_thread(void* _client)
{
    int n = 0;
    t_rscp_client* client = _client;
    t_rscp_header_common common;
    const t_map_set* method;
    u_rscp_header buf;

    //lock("rscp_client_req_thread-start");
        report(" + RSCP Client [%d] request ep", client->nr);
    //unlock("rscp_client_req_thread-start");

    while (!(client->task & E_RSCP_CLIENT_KILL)) {
        n = rscp_parse_request(&client->con2, client_method, &method, (void*)&buf, &common);

        // connection closed...
        if (n) break;

        lock("rscp_client_req_thread");

        if(!(client->task & E_RSCP_CLIENT_KILL) && (client->media)) {
#ifdef REPORT_RSCP
            report(" < RSCP Client [%d] %s", client->nr, common.rq.method);
#endif

            // process request (function responses for itself)
            n = ((frscpm*)method->fnc)(client->media, (void*)&buf, &client->con);
            if (n != RSCP_SUCCESS) {
                report(" ? execute method \"%s\" error (%d)", common.rq.method, n);
                unlock("rscp_client_req_thread");
                break;
            }
        }

        unlock("rscp_client_req_thread");

    }

    //lock("rscp_client_req_thread-close");
        report(" - RSCP Client [%d] request ep", client->nr);
    //unlock("rscp_client_req_thread-close");

    return 0;
}

