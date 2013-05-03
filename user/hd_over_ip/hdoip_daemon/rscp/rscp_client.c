/*
 * rscp_client.c
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
#include "rscp.h"
#include "rscp_media.h"
#include "rscp_net.h"
#include "rscp_string.h"
#include "rscp_client.h"
#include "rscp_command.h"
#include "rscp_parse_header.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hdcp.h"
#include "hoi_drv_user.h"

#include "../hdcp/rsaes-oaep/rsaes.h"
#include "../hdcp/protocol/protocol.h"


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
#ifdef REPORT_RSCP_CLIENT
        report(" + RSCP Client [%d] open %s", client->nr, address);
#endif

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
            //herror("gethostbyname");
            ret = RSCP_CLIENT_ERROR;
        }
    }

    if (ret == RSCP_SUCCESS) {
        addr.s_addr = *((uint32_t*)host->h_addr_list[0]);

        // default port 554
        if (uri.port) {
            port = htons(atoi(uri.port));
        } else {
            port = htons(reg_get_int("rscp-server-port"));
        }
#ifdef REPORT_RSCP_CLIENT
        report(" i server name: %s - %s:%d", host->h_name, inet_ntoa(addr), ntohs(port));
#endif
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

        if (connect(fd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_in)) == -1) {
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
#ifdef REPORT_RSCP_CLIENT
        report(" i RSCP Client [%d] detach media %s", client->nr, client->media->name);
#endif
        rmcr_teardown(client->media, 0);
        client->media->creator = 0;
        client->media = 0;
#ifdef REPORT_RSCP_CLIENT
        report(" i RSCP Client [%d] join pthread", client->nr);
#endif

        unlock("rscp_client_close");
            pthread_join(client->th1, 0);
            // cancel thread because there is a blocking read in it (should a cleanup handler be called after this?)
            pthread_cancel(client->th2);
        lock("rscp_client_close");

        close(client->con.fdr);
        close(client->con2.fdr);
    }
#ifdef REPORT_RSCP_CLIENT
    report(" - RSCP Client [%d]", client->nr);
#endif

    free(client);

    return RSCP_SUCCESS;
}

// returns response code
int rscp_client_setup(t_rscp_client* client, t_rscp_transport* transport, t_rscp_edid *edid, t_rscp_hdcp *hdcp)
{
    int n;
    u_rscp_header buf;
    t_rscp_header_common common;

#ifdef REPORT_RSCP
    report(" > RSCP Client [%d] SETUP", client->nr);
#endif

    // send setup message
    rscp_request_setup(&client->con, client->uri, transport, edid, hdcp);

    rscp_default_response_setup((void*)&buf);

    // response
    n = rscp_parse_response(&client->con, tab_response_setup, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    if (n == RSCP_SUCCESS) {
        // TODO:
        strcpy(client->media->sessionid, common.session);
        rmcr_setup(client->media, (void*)&buf);
    } else if (n == RSCP_RESPONSE_ERROR) {
        if (client->media->error) client->media->error(client->media, (void*)n, &client->con);
    } else {
        perrno("client setup internal failure (%d)", n);
    }

    return n;
}
/** HDCP session key exchange
 *  Exchange session key with server
 *
 * */
int rscp_client_hdcp(t_rscp_client* client)
{
    int n, i;
    u_rscp_header buf;
    t_rscp_header_common common;
    char content[]="";
    char H[65];
    char ks[33];
    char riv[17];
    char L[65];
    char *id[16] = {"00","01","02","03","04","05","06","07","08","09","10","11","12","13","14","15"};
    client->media->hdcp_var.repeater = 0;  //repeater value
    t_rscp_rsp_hdcp* p;
    p = (t_rscp_rsp_hdcp*)&buf;

    /* Get the encrypted HDCP keys from flash and decrypt them*/
    hdcp_decrypt_flash_keys();
    report(INFO "Ask server to start session key exchange");

    /* send start hdcp */
    rscp_request_hdcp(&client->con, client->media->sessionid, client->uri, id[0], content);
    rscp_default_response_hdcp((void*)&buf);
    // response
    n = rscp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "02")) return RSCP_HDCP_ERROR;  //check if correct message was received
    strcpy(client->media->hdcp_var.rtx, p->content);  //save rtx

    /* Add repeater value to certificate */
    memmove(hdoipd.hdcp.certrx + 1,hdoipd.hdcp.certrx,1045);
    if (client->media->hdcp_var.repeater == 1) hdoipd.hdcp.certrx[0]='1';
    else hdoipd.hdcp.certrx[0]='0';

    /* send certificate */
    rscp_request_hdcp(&client->con, client->media->sessionid, client->uri, id[3], hdoipd.hdcp.certrx);
    rscp_default_response_hdcp((void*)&buf);
    // response
    n = rscp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "04")) return RSCP_HDCP_ERROR;  //check if correct message was received

    /* decrypt km */
    rsaes_decrypt(p->content, client->media->hdcp_var.km, hdoipd.hdcp.p, hdoipd.hdcp.q,hdoipd.hdcp.dp,hdoipd.hdcp.dq,hdoipd.hdcp.qInv);
    //report(INFO "km: %s",client->media->hdcp_var.km); //SECRET VALUE, SHOW ONLY TO DEBUG
    report(INFO "rtx: %s",client->media->hdcp_var.rtx);

    /* send rrx  */
    hdcp_generate_random_nr(client->media->hdcp_var.rrx);
    rscp_request_hdcp(&client->con, client->media->sessionid, client->uri, id[6], client->media->hdcp_var.rrx);
    rscp_default_response_hdcp((void*)&buf);
    /* response (only acknowledge, therefore ignore content) */
    n = rscp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    if (strcmp(p->id, "00")) return RSCP_HDCP_ERROR;  //check if correct message was received

    /* send AKE send H_prime */
	report(INFO "rtx: %s", client->media->hdcp_var.rtx);
    hdcp_calculate_h(client->media->hdcp_var.rtx, &client->media->hdcp_var.repeater, H, client->media->hdcp_var.km, client->media->hdcp_var.kd);
    report(INFO "H: %s", H);
    rscp_request_hdcp(&client->con, client->media->sessionid, client->uri, id[7], H);
    rscp_default_response_hdcp((void*)&buf);
    // response LC_init
    n = rscp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "09")) return RSCP_HDCP_ERROR;  //check if correct message was received
    strcpy(client->media->hdcp_var.rn, p->content);

    /* generate HMAC of rn and send it back*/
    report(INFO "rn: %s", client->media->hdcp_var.rn);
    report(INFO "rrx: %s", client->media->hdcp_var.rrx);
    hdcp_calculate_l(client->media->hdcp_var.rn, client->media->hdcp_var.rrx, client->media->hdcp_var.kd, L);
    report(INFO "L: %s", L);
    rscp_request_hdcp(&client->con, client->media->sessionid, client->uri, id[10], L);
    // response contains encrypted session key
    n = rscp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "11")) return RSCP_HDCP_ERROR;  //check if correct message was received
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
    return RSCP_SUCCESS;
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

#ifdef REPORT_RSCP
    report(" > RSCP Client [%d] PLAY", client->nr);
#endif

    rscp_request_play(&client->con, client->uri, client->media->sessionid, fmt);

    rscp_default_response_play((void*)&buf);

    // response
        
        //if audio_board stream and no audio board connected -> do NOT stream!
    if(!strcmp(client->media->name,"audio_board") && !hdoipd_rsc(RSC_AUDIO_BOARD_OUT)) {
        n = RSCP_RESPONSE_ERROR;                        //TODO generate another error (-> generates error NULL in client->media->error)                 
    } else {
        n = rscp_parse_response(&client->con, tab_response_play, (void*)&buf, 0, CFG_RSP_TIMEOUT);
    }
    
    if (n == RSCP_SUCCESS) {
        rmcr_play(client->media, (void*)&buf, &client->con);
    } else if (n == RSCP_RESPONSE_ERROR) {
        if (client->media->error) client->media->error(client->media, (void*)n, &client->con);
    } else {
        perrno("client play internal failure (%d)", n);
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

#ifdef REPORT_RSCP
    report(" > RSCP Client [%d] TEARDOWN", client->nr);
#endif

    // request teardown
    rscp_request_teardown(&client->con, client->uri, client->media->sessionid);

    rscp_default_response_teardown((void*)&buf);

    // response
    rscp_parse_response(&client->con, tab_response_teardown, (void*)&buf, 0, CFG_RSP_TIMEOUT);
    rmcr_teardown(client->media, (void*)&buf);

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
#ifdef REPORT_RSCP_CLIENT
    report(" > RSCP Client [%d] kill", client->nr);
#endif
    // response
    rmcr_teardown(client->media, 0);

    rscp_client_close(client);

    return RSCP_SUCCESS;
}

int rscp_client_update(t_rscp_client* client, uint32_t event)
{
    t_rscp_rtp_format fmt;
    char* s;
    s = reg_get("compress");
    if (strcmp(s, "jp2k") == 0) {
        fmt.compress = FORMAT_JPEG2000;
    } else {
        fmt.compress = FORMAT_PLAIN;
    }
    hoi_drv_get_mtime(&fmt.rtptime);
    fmt.value = reg_get_int("advcnt-min");

#ifdef REPORT_RSCP_UPDATE
    report(" > RSCP Client [%d] UPDATE", client->nr);
#endif

    rscp_request_update(&client->con, client->uri, client->media->sessionid, event, &fmt);

    return RSCP_SUCCESS;
}

/** just say "hello" to the server
 *  the first message of an message-exchange
 *
 * */
int rscp_client_hello(t_rscp_client* client)
{
#ifdef REPORT_RSCP_HELLO
    report(" > RSCP Client [%d] HELLO", client->nr);
#endif

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

    client = queue_get(&task_list);
    while(client) {
        if(client->task & E_RSCP_CLIENT_KILL) {
            rscp_client_close(client);
        } else { 
            if(client->task & E_RSCP_CLIENT_TEARDOWN) {
                rscp_client_teardown(client);
            } else {
                if (client->task & E_RSCP_CLIENT_PLAY) {
                    rscp_media_play(client->media);
                }
            }
        }
        client->task = 0;
        client = queue_get(&task_list);
    }
}

void rscp_client_force_close(t_node* list)
{
    t_rscp_client* client;
    while ((client = list_peek(list))) {
        rscp_client_close(client);
    }
}

/** checks if the received message is a request or a response
 *  if its a request, write to pipe 2, else to pipe 1
 *
 * */
void* rscp_client_thread(void* _client)
{
    int n;
    char* line, *tst;
    t_rscp_client* client = _client;

#ifdef REPORT_RSCP_CLIENT
    report(" + RSCP Client [%d] filter", client->nr);
#endif

    while ((n = rscp_receive(&client->con1, &line, 0)) == RSCP_SUCCESS) {
        tst = line;
        if (!str_starts_with(&tst, RSCP_VERSION)) { // if response
            do {
                msgprintf(&client->con2, "%s\r\n", line);
                if (*line == 0) break;
            } while ((n = rscp_receive(&client->con1, &line, 0)) == RSCP_SUCCESS);
            if (n != RSCP_SUCCESS) {
                report(ERROR "rscp client filter receive error on request");
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
            rscp_write(&client->con1);  //write to pipe 1 ??
        }
    }
    client->task = E_RSCP_CLIENT_KILL;

#ifdef REPORT_RSCP_CLIENT
    report(" - RSCP Client [%d] filter: (%d) %s", client->nr, n, strerror(errno));
#endif

    close(client->con1.fdw);
    close(client->con2.fdw);

    return 0;
}

/** function processes requests only
 *
 * */
void* rscp_client_req_thread(void* _client)
{
    int n = 0;
    t_rscp_client* client = _client;
    t_rscp_header_common common;
    const t_map_set* method;
    u_rscp_header buf;

    pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);
    pthread_setcanceltype  (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);


    //lock("rscp_client_req_thread-start");
#ifdef REPORT_RSCP_CLIENT
    report(" + RSCP Client [%d] request ep", client->nr);
#endif
    //unlock("rscp_client_req_thread-start");

    while (!(client->task & E_RSCP_CLIENT_KILL)) {
        n = rscp_parse_request(&client->con2, client_method, &method, (void*)&buf, &common);

        // connection closed...
        if (n) break;

        lock("rscp_client_req_thread");

        if(!(client->task & E_RSCP_CLIENT_KILL) && (client->media)) {
#ifdef REPORT_RSCP_RX
            report(" < RSCP Client [%d] %s", client->nr, common.rq.method);
#endif
            // process request (function responses for itself)
            n = ((frscpm*)method->fnc)(client->media, (void*)&buf, &client->con);
            if (n != RSCP_SUCCESS) {
                report(" ? execute method \"%s\" error (%d) media (%s)", common.rq.method, n, client->media->name);
                unlock("rscp_client_req_thread");
                break;
            }
        }
        unlock("rscp_client_req_thread");

    }

    //lock("rscp_client_req_thread-close");
#ifdef REPORT_RSCP_CLIENT
    report(" - RSCP Client [%d] request ep", client->nr);
#endif
    //unlock("rscp_client_req_thread-close");

    return 0;
}

int rscp_client_pause(t_rscp_client* client)
{
    rscp_request_pause(&client->con, client->uri, client->media->sessionid);

    return 0;
}
