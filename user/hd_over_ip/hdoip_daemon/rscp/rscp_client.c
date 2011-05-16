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
        dest_addr.sin_addr.s_addr = addr.s_addr;

        //try to connect server
        if (connect(fd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr)) == -1) {
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
int rscp_client_setup(t_rscp_client* client, t_rscp_transport* transport, t_rscp_edid *edid, t_rscp_hdcp *hdcp)
{
    int n;
    u_rscp_header buf;
    t_rscp_header_common common;


    report(" > RSCP Client [%d] SETUP", client->nr);
    // send setup message
    rscp_request_setup(&client->con, client->uri, transport, edid, hdcp);

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
// HDCP
// returns response code
int rscp_client_hdcp(t_rscp_client* client)
{
    int n, i;
    u_rscp_header buf;
    t_rscp_header_common common;
    char id[]="00";
    char content[]="";
    uint32_t keys[6];


    report(INFO "Ask server to start session key exchange");
    /* send start hdcp */
    rscp_request_hdcp(&client->con, &client->media->sessionid, &client->uri, id, content);

    rscp_default_response_hdcp((void*)&buf);

    // response
    n = rscp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);
    t_rscp_rsp_hdcp* p;
    p = &buf;
    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    strcpy(hdoipd.hdcp.rtx, p->content);  //save rtx

    /* send certificate */

    char id2[]="03";
    //char number2[]="1745bb8bd04bc83c79578f90c914b8938055aa4ac1fa80393827975af6622de43808dcd5d90b83cb3d89eb00d0944f43f5fabb9c4c996ef78b58f6977b47d08149c81a08f041fa088e120c7344a49356599cf5319f0c68176055cb9deddab3db092a1234f0c71304278f655aebd36258e250d4e5e8e776a60e3c1e9eecd2b9e186397d4e67501000100001d0a61eaabf8a82b0269a134fd91ac2bf28f348bd484fa62bc014a4aa2b214bfb5f4dfac80930d13ec9ce5d83470519a6680ebbecc7e45f0e6396384c9b98e8caf9ca9d40eeb9a572a1741ca97f31996b55d0f30a384e573a2ed05697a22ce841f3e399e2876c9bc895b70b17bf4edb67412ab482964ce6c6004eba97aa215a6589aad32c75339e5fef037a7a0c5ffecd9b005bb2513a0a4c70b2a5dc68f5111cb36ed5c177e2220c3eb408c67bb1cd247b0e0bde74ccd5dd52312f83b1d913bf3c760ea902448e592216cf6d95e768d2b86a67c16aea83608a037141ad703e14031ca6c95e010b043cfb7e03005b9acb70868cd7e11472a033beb74c819628b2f1191b6064fe02a44204329131fddd04a116c0e83bf22623bebecd77628ba648842c873a79e4a693ab20c4b3ad950db7c51ee15e06b2c63a69157ddbf174723ad15cbb991180b518ff91c5167c10b78f5d955dc48e4c083a5df75e2dc88d2c6dddf1f379035f6fddae0043269c1afd9f911c5aa7458321c71aaa714fb231722";
    char number2[]="11111111111111111111111111";
    rscp_request_hdcp(&client->con, &client->media->sessionid, &client->uri, id2, number2);

    //rscp_default_response_setup((void*)&buf);

    // response
    n = rscp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);
    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);

    //receiver private key
    char pp[]="dc1a02b836ed3ae87474cd72284aee3190e4d06af9f6f8d35029c2849798105dea7b88fd36c50499adab270a5a2af9187b7db0c3cbe35ac29a10f7c99a183eb5";
    char q[]="db42e942e32a78c96f2b7b74d69baeb93df4e735901cc45ab4228c3c089ba529645729b2c480f9eec694303ed2339fbb6a430389149b8f20b8601f7f303b20c1";
    char dp[]="73d1a418b79e81df0c58e23aee04efee59266e9dbc473f8c42a496dd1ac043ec8794d5f318bcf7bcbe6c4fb0dcddbc122bf969e8be0337212bdd3de67215cbf9";
    char dq[]="091de61f0edd043ab3f1a5e77cc8ea61ef6e90728cb47581a3fdcfc0eb46b57e5c1ab7b424318cb2ddf4e970a342dc4069b1b1a2f0856b551bf57b39c9a29bc1";
    char qInv[]="8958a5a363d9a9ee0e7ea1c0562d59fcf8661c2648219de061e4a8069764c7017747118ea281d200dd5a1b8f7a1b2c685639cfcdd36aff73811d913d48b4434c";

    /* decrypt km */
    rsaes_decrypt(p->content, &hdoipd.hdcp.km,pp,q,dp,dq,qInv);
    report(INFO "km: %s",hdoipd.hdcp.km);
    report(INFO "rtx: %s",hdoipd.hdcp.rtx);

    /* send rrx  */
    char id6[]="06";
    char rrx[17];
    hdcp_generate_random_nr(rrx);
    strcpy(hdoipd.hdcp.rrx, rrx);  //save rrx
    rscp_request_hdcp(&client->con, &client->media->sessionid, &client->uri, id6, rrx);
    // response (only acknowledge, therefore ignore content)
    n = rscp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, 1000); //test

    /* send AKE send H_prime */
    char H[65];
    char kd[65]="";
    char id7[]="07";
    int repeater=1;
	report(INFO "rtx: %s", hdoipd.hdcp.rtx);
	report(INFO "km: %s", hdoipd.hdcp.km);

    hdcp_calculate_h(&hdoipd.hdcp.rtx, repeater, H, &hdoipd.hdcp.km, kd);
    strcpy(hdoipd.hdcp.kd, kd);  //save kd
    report(INFO "kd: %s", hdoipd.hdcp.kd);
    report(INFO "H: %s", H);
    rscp_request_hdcp(&client->con, &client->media->sessionid, &client->uri, id7, H);

    // response LC_init
    n = rscp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);
    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    strcpy(hdoipd.hdcp.rn, p->content);

    /* generate HMAC of rn and send it back*/
    char L[65];
    char id10[]="10";
    report(INFO "rn: %s", hdoipd.hdcp.rn);
    report(INFO "rrx: %s", hdoipd.hdcp.rrx);
    report(INFO "kd: %s", hdoipd.hdcp.kd);
    hdcp_calculate_l(&hdoipd.hdcp.rn, &hdoipd.hdcp.rrx, &hdoipd.hdcp.kd, L);
    report(INFO "L: %s", L);
    rscp_request_hdcp(&client->con, &client->media->sessionid, &client->uri, id10, L);

    // response contains encrypted session key
    n = rscp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);
    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    /*devide received content into ks and riv*/
    for (i=0;i<17;i++){
    	hdoipd.hdcp.riv[i]=p->content[32+i];
    }
    p->content[32]='\0';
    // decrypt session key
    char ks[33];
    hdcp_ske_dec_ks(&hdoipd.hdcp.rn, &ks, p->content, &hdoipd.hdcp.rtx, &hdoipd.hdcp.rrx, &hdoipd.hdcp.km);
    strcpy(hdoipd.hdcp.ks, ks);  //save ks
    report(INFO "THE SESSION KEY: %s", ks);
    report(INFO "RIV: %s", hdoipd.hdcp.riv);

    /* write keys to HW and enable encryption*/
    hdcp_convert_sk_char_int(&hdoipd.hdcp.ks, &hdoipd.hdcp.riv, keys);
    hoi_drv_hdcp(keys); 				//write keys to kernel
    //DISABLED -> NO KEYS		    	//hoi_drv_hdcp_adv9889en();   		//enable hdcp encryption in AD9889
	/*hoi_drv_hdcp_viden_eti();*/
	report(INFO "Video encryption enabled (eti)!");
	//DISABLED -> NO KEYS			    //hoi_drv_hdcp_adv9889en();			//enable hdcp encryption in AD9889
	/*hoi_drv_hdcp_auden_eti();*/
	report(INFO "Audio encryption enabled (eti)!");


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

    report(" > RSCP Client [%s %d] PLAY", client->media->name, client->nr);

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

    report(" > RSCP Client [%s %d] TEARDOWN", client->media->name, client->nr);

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

/** just say "hello" to the server
 *  the first message of an message-exchange
 *
 * */
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

/** checks if the received message is a request or a response
 *  if its a request, write to pipe 2, else to pipe 1
 *
 * */
void* rscp_client_thread(void* _client)
{
    int n;
    char* line, *tst;
    t_rscp_client* client = _client;

    report(" + RSCP Client [%d] filter", client->nr);
    // receive as long as no errors occur ??
    while ((n = rscp_receive(&client->con1, &line, 0)) == RSCP_SUCCESS) {
        tst = line;
        //check if received message is a request or a response
        if (!str_starts_with(&tst, RSCP_VERSION)) {  //if request...
            do {
                msgprintf(&client->con2, "%s\r\n", line);
                if (*line == 0) break;
            } while ((n = rscp_receive(&client->con1, &line, 0)) == RSCP_SUCCESS);
            if (n != RSCP_SUCCESS) {
                report("rscp client filter receive error on request");
                break;
            }
            rscp_write(&client->con2);  //write to pipe 2 ??

        } else {	//if response
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
    report(" - RSCP Client [%d] filter: (%d) %s", client->nr, n, strerror(errno));

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

