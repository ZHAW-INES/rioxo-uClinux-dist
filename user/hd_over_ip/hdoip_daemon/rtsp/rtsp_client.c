/*
 * rtsp_client.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 *
 *  Functions to establish connection(s) to the server
 *  Functions to send and receive messages (every sent message
 *  expects an answer)
 *  Difference request/response:
 *  Response: server response to a request from client (normal case)
 *  Request: request from server (to client) initiated by server
 */

#include <stdio.h>
#include <pthread.h>
#include "rtsp.h"
#include "rtsp_media.h"
#include "rtsp_net.h"
#include "rtsp_string.h"
#include "rtsp_client.h"
#include "rtsp_command.h"
#include "rtsp_parse_header.h"
#include "hdoipd.h"
#include "hdcp.h"
#include "hoi_drv_user.h"

#include "../hdcp/rsaes-oaep/rsaes.h"
#include "../hdcp/protocol/protocol.h"


void* rtsp_client_thread(void* _client);
void* rtsp_client_req_thread(void* _client);


/** Opens a new rtsp client connection
 *
 * !!! hdoipd must be locked !!! (because media is manipulated)
 */
t_rtsp_client* rtsp_client_open(t_node* list, t_rtsp_media *media, char* address)
{
    static int nr = 1;
    char buf[200];
    int fd, port, ret = RTSP_SUCCESS;
    t_str_uri uri;
    struct hostent* host;
    struct in_addr addr;
    struct sockaddr_in dest_addr;

    if (media) if (rtsp_media_active(media)) return 0;

    t_rtsp_client* client = malloc(sizeof(t_rtsp_client));
    if (!client) {
        report(ERROR "rtsp_client_open.malloc: out of memory");
        return 0;
    }

    memset(client, 0, sizeof(t_rtsp_client));
    client->task = 0;

    if (media) {
        if (media->creator) {
            report(" ? RTSP Client - Media has already a client");
            ret = RTSP_CLIENT_ERROR;
        }
    }

    if (ret == RTSP_SUCCESS) {
        client->nr = nr++;
#ifdef REPORT_RTSP_CLIENT
        report(" + RTSP Client [%d] open %s", client->nr, address);
#endif

        memcpy(client->uri, address, strlen(address)+1);
        memcpy(buf, address, strlen(address)+1);

        if (!str_split_uri(&uri, buf)) {
            report("uri error: scheme://server[:port]/name");
            ret = RTSP_CLIENT_ERROR;
        }
    }

    if (ret == RTSP_SUCCESS) {
        host = gethostbyname(uri.server);

        if (!host) {
            //herror("gethostbyname");
            ret = RTSP_CLIENT_ERROR;
        }
    }

    if (ret == RTSP_SUCCESS) {
        addr.s_addr = *((uint32_t*)host->h_addr_list[0]);

        // default port 554
        if (uri.port) {
            port = htons(atoi(uri.port));
        } else {
            port = htons(reg_get_int("rtsp-server-port"));
        }
#ifdef REPORT_RTSP_CLIENT
        report(" i server name: %s - %s:%d", host->h_name, inet_ntoa(addr), ntohs(port));
#endif
        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            report(ERROR "RTSP Client [%d]: socket error: %s", client->nr, strerror(errno));
            ret = RTSP_CLIENT_ERROR;
        }
    }

    if (ret == RTSP_SUCCESS) {
        // setup own address
        memset(&dest_addr, 0, sizeof(struct sockaddr_in));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = port;
        dest_addr.sin_addr.s_addr = addr.s_addr;        // remote address

        if (connect(fd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_in)) == -1) {
            close(fd);
            perrno(ERROR "RTSP Client [%d]: connection refused", client->nr);
            ret = RTSP_CLIENT_ERROR;
        }
    }

    if (ret == RTSP_SUCCESS) {
        rtsp_coninit(&client->con, fd, addr.s_addr);

        if (media) {
            ret = rtsp_split(&client->con, &client->con1, &client->con2);
        }
    }

    // attach client to media
    if (ret == RTSP_SUCCESS) {
        // attach client & media
        client->media = media;
        if (media) media->creator = client;

        // add to client list
        if (list) client->idx = list_add(list, client);

        if (media) {
            unlock("rtsp_client_split");
                pthread(client->th1, rtsp_client_thread, client);
                pthread(client->th2, rtsp_client_req_thread, client);
            lock("rtsp_client_split");
        }
    } else {
        free(client);
        client = 0;
        // tell media open failed
        if (media && media->error) media->error(media, 0, 0);
    }

    return client;
}


/** Closes and removes a rtsp client connection
 *
 * !!! hdoipd must be locked !!! (because client-list is manipulated)
 */
int rtsp_client_close(t_rtsp_client* client)
{
    if (!client) return RTSP_NULL_POINTER;


    // remove client
    if (client->idx) {
        list_remove(client->idx);
        client->idx = 0;
    } else {
        report(ERROR "RTSP Client [%d] already closed", client->nr);
        return RTSP_NULL_POINTER;
    }

    client->task = E_RTSP_CLIENT_KILL;
    shutdown(client->con.fdw, SHUT_RDWR);
    close(client->con.fdw);

    // detach client from media
    if (client->media) {
#ifdef REPORT_RTSP_CLIENT
        report(" i RTSP Client [%d] detach media %s", client->nr, client->media->name);
#endif
        rmcr_teardown(client->media, 0);
        client->media->creator = 0;
        client->media = 0;
#ifdef REPORT_RTSP_CLIENT
        report(" i RTSP Client [%d] join pthread", client->nr);
#endif

        unlock("rtsp_client_close");
            pthread_join(client->th1, 0);
            // cancel thread because there is a blocking read in it (should a cleanup handler be called after this?)
            pthread_cancel(client->th2);
        lock("rtsp_client_close");

        close(client->con.fdr);
        close(client->con2.fdr);
    }
#ifdef REPORT_RTSP_CLIENT
    report(" - RTSP Client [%d]", client->nr);
#endif

    free(client);

    return RTSP_SUCCESS;
}

// returns response code
int rtsp_client_setup(t_rtsp_client* client, t_rtsp_transport* transport, t_rtsp_edid *edid, t_rtsp_hdcp *hdcp)
{
    int n;
    u_rtsp_header buf;
    t_rtsp_header_common common;

#ifdef REPORT_RTSP
    report(" > RTSP Client [%d] SETUP", client->nr);
#endif

    // send setup message
    rtsp_request_setup(&client->con, client->uri, transport, edid, hdcp);

    rtsp_default_response_setup((void*)&buf);

    // response
    n = rtsp_parse_response(&client->con, tab_response_setup, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    if (n == RTSP_SUCCESS) {
        // TODO:
        strcpy(client->media->sessionid, common.session);
        rmcr_setup(client->media, (void*)&buf);
    } else if (n == RTSP_RESPONSE_ERROR) {
        if (client->media->error) client->media->error(client->media, (void*)n, &client->con);
    } else {
        perrno("internal failure (%d)", n);
    }

    return n;
}
/** HDCP session key exchange
 *  Exchange session key with server
 *
 * */
int rtsp_client_hdcp(t_rtsp_client* client)
{
    int n, i;
    u_rtsp_header buf;
    t_rtsp_header_common common;
    char content[]="";
    char H[65];
    char ks[33];
    char riv[17];
    char L[65];
    char *id[16] = {"00","01","02","03","04","05","06","07","08","09","10","11","12","13","14","15"};
    client->media->hdcp_var.repeater = 0;  //repeater value
    t_rtsp_rsp_hdcp* p;
    p = (t_rtsp_rsp_hdcp*)&buf;

    /* Get the encrypted HDCP keys from flash and decrypt them*/
    hdcp_decrypt_flash_keys();
    report(INFO "Ask server to start session key exchange");

    /* send start hdcp */
    rtsp_request_hdcp(&client->con, client->media->sessionid, client->uri, id[0], content);
    rtsp_default_response_hdcp((void*)&buf);
    // response
    n = rtsp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "02")) return RTSP_HDCP_ERROR;  //check if correct message was received
    strcpy(client->media->hdcp_var.rtx, p->content);  //save rtx

    /* Add repeater value to certificate */
    memmove(hdoipd.hdcp.certrx + 1,hdoipd.hdcp.certrx,1045);
    if (client->media->hdcp_var.repeater == 1) hdoipd.hdcp.certrx[0]='1';
    else hdoipd.hdcp.certrx[0]='0';

    /* send certificate */
    rtsp_request_hdcp(&client->con, client->media->sessionid, client->uri, id[3], hdoipd.hdcp.certrx);
    rtsp_default_response_hdcp((void*)&buf);
    // response
    n = rtsp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "04")) return RTSP_HDCP_ERROR;  //check if correct message was received

    /* decrypt km */
    rsaes_decrypt(p->content, client->media->hdcp_var.km, hdoipd.hdcp.p, hdoipd.hdcp.q,hdoipd.hdcp.dp,hdoipd.hdcp.dq,hdoipd.hdcp.qInv);
    //report(INFO "km: %s",client->media->hdcp_var.km); //SECRET VALUE, SHOW ONLY TO DEBUG
    report(INFO "rtx: %s",client->media->hdcp_var.rtx);

    /* send rrx  */
    hdcp_generate_random_nr(client->media->hdcp_var.rrx);
    rtsp_request_hdcp(&client->con, client->media->sessionid, client->uri, id[6], client->media->hdcp_var.rrx);
    rtsp_default_response_hdcp((void*)&buf);
    /* response (only acknowledge, therefore ignore content) */
    n = rtsp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    if (strcmp(p->id, "00")) return RTSP_HDCP_ERROR;  //check if correct message was received

    /* send AKE send H_prime */
	report(INFO "rtx: %s", client->media->hdcp_var.rtx);
    hdcp_calculate_h(client->media->hdcp_var.rtx, &client->media->hdcp_var.repeater, H, client->media->hdcp_var.km, client->media->hdcp_var.kd);
    report(INFO "H: %s", H);
    rtsp_request_hdcp(&client->con, client->media->sessionid, client->uri, id[7], H);
    rtsp_default_response_hdcp((void*)&buf);
    // response LC_init
    n = rtsp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "09")) return RTSP_HDCP_ERROR;  //check if correct message was received
    strcpy(client->media->hdcp_var.rn, p->content);

    /* generate HMAC of rn and send it back*/
    report(INFO "rn: %s", client->media->hdcp_var.rn);
    report(INFO "rrx: %s", client->media->hdcp_var.rrx);
    hdcp_calculate_l(client->media->hdcp_var.rn, client->media->hdcp_var.rrx, client->media->hdcp_var.kd, L);
    report(INFO "L: %s", L);
    rtsp_request_hdcp(&client->con, client->media->sessionid, client->uri, id[10], L);
    // response contains encrypted session key
    n = rtsp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "11")) return RTSP_HDCP_ERROR;  //check if correct message was received
    /*devide received content into ks and riv*/
    for (i=0;i<17;i++){
    	riv[i]=p->content[32+i];
    }
    p->content[32]='\0';
    // decrypt session key
    hdcp_ske_dec_ks(client->media->hdcp_var.rn, ks, p->content, client->media->hdcp_var.rtx, client->media->hdcp_var.rrx, client->media->hdcp_var.km);
    //report(INFO "THE SESSION KEY: %s", ks); //SECRET VALUE; SHOW ONLY TO DEBUG
    report(INFO "RIV: %s", riv);
	/* xor session key with lc128 */
	xor_strings(ks, hdoipd.hdcp.lc128, ks,32);
    /* write keys to HW and enable encryption*/
    hdcp_convert_sk_char_int(ks, riv, hdoipd.hdcp.keys);
    hdoipd.hdcp.ske_executed = HDCP_SKE_EXECUTED;
    report(INFO "SESSION KEY EXCHANGE SUCCESSFUL!")
    return RTSP_SUCCESS;
}


int rtsp_client_set_play(t_rtsp_client* client)
{
    client->task |= E_RTSP_CLIENT_PLAY;

    return RTSP_SUCCESS;
}

int rtsp_client_play(t_rtsp_client* client, t_rtsp_rtp_format* fmt)
{
    int n;
    u_rtsp_header buf;

#ifdef REPORT_RTSP
    report(" > RTSP Client [%d] PLAY", client->nr);
#endif

    rtsp_request_play(&client->con, client->uri, client->media->sessionid, fmt);

    rtsp_default_response_play((void*)&buf);

    // response
    n = rtsp_parse_response(&client->con, tab_response_play, (void*)&buf, 0, CFG_RSP_TIMEOUT);

    if (n == RTSP_SUCCESS) {
        rmcr_play(client->media, (void*)&buf, &client->con);
    } else if (n == RTSP_RESPONSE_ERROR) {
        if (client->media->error) client->media->error(client->media, (void*)n, &client->con);
    } else {
        perrno("internal failure (%d)", n);
    }

    return n;
}

int rtsp_client_set_teardown(t_rtsp_client* client)
{
	client->task |= E_RTSP_CLIENT_TEARDOWN;

	return RTSP_SUCCESS;
}

int rtsp_client_teardown(t_rtsp_client* client)
{
    u_rtsp_header buf;

#ifdef REPORT_RTSP
    report(" > RTSP Client [%d] TEARDOWN", client->nr);
#endif

    // request teardown
    rtsp_request_teardown(&client->con, client->uri, client->media->sessionid);

    rtsp_default_response_teardown((void*)&buf);

    // response
    rtsp_parse_response(&client->con, tab_response_teardown, (void*)&buf, 0, CFG_RSP_TIMEOUT);
    rmcr_teardown(client->media, (void*)&buf);

    rtsp_client_close(client);

    return RTSP_SUCCESS;
}

int rtsp_client_set_kill(t_rtsp_client* client)
{
	client->task |= E_RTSP_CLIENT_KILL;

	return RTSP_SUCCESS;
}

int rtsp_client_kill(t_rtsp_client* client)
{
#ifdef REPORT_RTSP_CLIENT
    report(" > RTSP Client [%d] kill", client->nr);
#endif
    // response
    rmcr_teardown(client->media, 0);

    rtsp_client_close(client);

    return RTSP_SUCCESS;
}

int rtsp_client_update(t_rtsp_client* client, uint32_t event)
{
    t_rtsp_rtp_format fmt;
    char* s;
    s = reg_get("compress");
    if (strcmp(s, "jp2k") == 0) {
        fmt.compress = FORMAT_JPEG2000;
    } else {
        fmt.compress = FORMAT_PLAIN;
    }
    hoi_drv_get_mtime(&fmt.rtptime);
    fmt.value = reg_get_int("advcnt-min");

#ifdef REPORT_RTSP_UPDATE
    report(" > RTSP Client [%d] UPDATE", client->nr);
#endif

    rtsp_request_update(&client->con, client->uri, client->media->sessionid, event, &fmt);

    return RTSP_SUCCESS;
}

/** just say "hello" to the server
 *  the first message of an message-exchange
 *
 * */
int rtsp_client_hello(t_rtsp_client* client)
{
#ifdef REPORT_RTSP_HELLO
    report(" > RTSP Client [%d] HELLO", client->nr);
#endif

    rtsp_request_hello(&client->con, client->uri);

    return RTSP_SUCCESS;
}

void rtsp_client_deactivate(t_node* list)
{
    t_rtsp_client* client;

    while ((client = list_peek(list))) {
        if (client->media->state != RTSP_INIT) {
            // proper teradown
            rtsp_client_teardown(client);
        } else {
            // delete client
            rtsp_client_close(client);
        }
    }
}

void rtsp_client_event(t_node* list, uint32_t event)
{
    t_rtsp_client* client;
    node_anchor(task_list);

    LIST_FOR(client, list) {
        if (client->media) rtsp_media_event(client->media, event);
        if (client->task)  queue_put(&task_list, client);
    }

    client = queue_get(&task_list);
    while(client) {
        if(client->task & E_RTSP_CLIENT_KILL) {
            rtsp_client_close(client);
        } else { 
            if(client->task & E_RTSP_CLIENT_TEARDOWN) {
                rtsp_client_teardown(client);
            } else {
                if (client->task & E_RTSP_CLIENT_PLAY) {
                    rtsp_media_play(client->media);
                }
            }
        }
        client->task = 0;
        client = queue_get(&task_list);
    }
}

void rtsp_client_force_close(t_node* list)
{
    t_rtsp_client* client;
    while ((client = list_peek(list))) {
        rtsp_client_close(client);
    }
}

/** checks if the received message is a request or a response
 *  if its a request, write to pipe 2, else to pipe 1
 *
 * */
void* rtsp_client_thread(void* _client)
{
    int n;
    char* line, *tst;
    t_rtsp_client* client = _client;

#ifdef REPORT_RTSP_CLIENT
    report(" + RTSP Client [%d] filter", client->nr);
#endif

    while ((n = rtsp_receive(&client->con1, &line, 0)) == RTSP_SUCCESS) {
        tst = line;
        if (!str_starts_with(&tst, RTSP_VERSION)) { // if response
            do {
                msgprintf(&client->con2, "%s\r\n", line);
                if (*line == 0) break;
            } while ((n = rtsp_receive(&client->con1, &line, 0)) == RTSP_SUCCESS);
            if (n != RTSP_SUCCESS) {
                report(ERROR "rtsp client filter receive error on request");
                break;
            }

            rtsp_write(&client->con2);

        } else { // if request

            do {
                msgprintf(&client->con1, "%s\r\n", line);
                if (*line == 0) break;
            } while ((n = rtsp_receive(&client->con1, &line, 0)) == RTSP_SUCCESS);
            if (n != RTSP_SUCCESS) {
                report("rtsp client filter receive error on response");
                break;
            }
            rtsp_write(&client->con1);  //write to pipe 1 ??
        }
    }
    client->task = E_RTSP_CLIENT_KILL;

#ifdef REPORT_RTSP_CLIENT
    report(" - RTSP Client [%d] filter: (%d) %s", client->nr, n, strerror(errno));
#endif

    close(client->con1.fdw);
    close(client->con2.fdw);

    return 0;
}

/** function processes requests only
 *
 * */
void* rtsp_client_req_thread(void* _client)
{
    int n = 0;
    t_rtsp_client* client = _client;
    t_rtsp_header_common common;
    const t_map_set* method;
    u_rtsp_header buf;

    pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);
    pthread_setcanceltype  (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);


    //lock("rtsp_client_req_thread-start");
#ifdef REPORT_RTSP_CLIENT
    report(" + RTSP Client [%d] request ep", client->nr);
#endif
    //unlock("rtsp_client_req_thread-start");

    while (!(client->task & E_RTSP_CLIENT_KILL)) {
        n = rtsp_parse_request(&client->con2, client_method, &method, (void*)&buf, &common);

        // connection closed...
        if (n) break;

        lock("rtsp_client_req_thread");

        if(!(client->task & E_RTSP_CLIENT_KILL) && (client->media)) {
#ifdef REPORT_RTSP_RX
            report(" < RTSP Client [%d] %s", client->nr, common.rq.method);
#endif
            // process request (function responses for itself)
            n = ((frtspm*)method->fnc)(client->media, (void*)&buf, &client->con);
            if (n != RTSP_SUCCESS) {
                report(" ? execute method \"%s\" error (%d) media (%s)", common.rq.method, n, client->media->name);
                unlock("rtsp_client_req_thread");
                break;
            }
        }
        unlock("rtsp_client_req_thread");

    }

    //lock("rtsp_client_req_thread-close");
#ifdef REPORT_RTSP_CLIENT
    report(" - RTSP Client [%d] request ep", client->nr);
#endif
    //unlock("rtsp_client_req_thread-close");

    return 0;
}

int rtsp_client_pause(t_rtsp_client* client)
{
    rtsp_request_pause(&client->con, client->uri, client->media->sessionid);

    return 0;
}
