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

#include <pthread.h>
#include <stdio.h>

#include "hdcp.h"
#include "hdoipd.h"
#include "hdoipd_osd.h"
#include "hoi_drv_user.h"
#include "rtsp.h"
#include "rtsp_client.h"
#include "rtsp_command.h"
#include "rtsp_error.h"
#include "rtsp_media.h"
#include "rtsp_net.h"
#include "rtsp_parse_header.h"
#include "rtsp_string.h"

#include "../hdcp/rsaes-oaep/rsaes.h"
#include "../hdcp/protocol/protocol.h"

typedef void (*request_handler)(t_rtsp_client*, t_rtsp_media*, void*);

typedef struct {
  t_rtsp_client *client;
  request_handler handler;
  void *data;
} t_client_data;

/**
 * add the given media to the client's list of active media-controls
 *
 * The name being used in the binary-search-tree for lookups is
 * taken from the "name" member of the media parameter.
 */
int add_media(t_rtsp_client* handle, t_rtsp_media* media)
{
  if (handle == NULL || media == NULL || media->name == NULL)
    return RTSP_NULL_POINTER;

  report(" ? RTSP Client [%d] adding media (%s)", handle->nr, media->name);
  media->creator = handle;

  bstmap_setp(&handle->media, media->name, media);
  return RTSP_SUCCESS;
}

/**
 * Retrieves the media-control matching the given name from the
 * client's list of active media-controls if available
 */
t_rtsp_media* get_media(t_rtsp_client* handle, char* name)
{
  if (handle == NULL || name == NULL)
    return NULL;

  return (t_rtsp_media*)bstmap_get(handle->media, name);
}

/**
 * removes the media-control matching the given name from the
 * client's list of active media-controls
 *
 * This method also checks whether this was the last active
 * media-control for the client.
 */
void remove_media(t_rtsp_client* handle, t_rtsp_media* media)
{
  if (handle == NULL || media == NULL || media->name == NULL)
    return;

  report(INFO "RTSP Client [%d] removing media (%s)", handle->nr, media->name);

  if (media->sessionid != NULL) {
    free(media->sessionid);
    media->sessionid = NULL;
  }

  media->state = RTSP_STATE_INIT;
}

/**
 * Execute rmcr_teardown for the media-control and remove it from
 * the client's list of active media-controls.
 *
 * sa rmcr_teardown
 * sa remove_media
 */
void teardown_media(t_rtsp_client* client, t_rtsp_media* media, u_rtsp_header* buf)
{
  if (client == NULL || media == NULL || media->name == NULL)
    return;

#ifdef REPORT_RTSP_CLIENT
  report(" i RTSP Client [%d] teardown media (%s)", client->nr, media->name);
#endif
  rmcr_teardown(media, buf);

  remove_media(client, media);
}

/**
 * A BST traversing method invoking teardown_media.
 *
 * sa teardown_media
 */
void detach_media(char UNUSED *key, char* value, void* data)
{
  t_rtsp_media *media = NULL;
  if (value == NULL || data == NULL)
    return;

  media = (t_rtsp_media*)value;

  teardown_media((t_rtsp_client*)data, media, NULL);
  media->creator = NULL;
}

/**
 * A BST traversing method invoking the method specified
 * in t_client_data.handler (stored in the data parameter).
 *
 * param key Name of the media-control
 * param value pointer to t_rtsp_media structure
 * param data pointer to t_client_data structure
 */
void request_dispatcher(char *key, char* value, void* data)
{
  t_rtsp_media *media = (t_rtsp_media*)value;
  t_client_data *clientData = (t_client_data*)data;
  t_rtsp_client *client = clientData->client;

  if (key == NULL || client == NULL || media == NULL || clientData->handler == NULL)
    return;

  clientData->handler(client, media, clientData->data);
}

int request(t_rtsp_client* client, char* mediaName, void* data, request_handler handler)
{
  t_client_data* clientData = NULL;
  t_rtsp_media* media = NULL;
  if (client == NULL || handler == NULL)
    return RTSP_NULL_POINTER;

  if (mediaName == NULL)
  {
    clientData = (t_client_data*)malloc(sizeof(t_client_data));
    clientData->client = client;
    clientData->handler = handler;
    clientData->data = data;

    bstmap_traverse(client->media, request_dispatcher, clientData);
    free(clientData);
  }
  else
  {
    media = get_media(client, mediaName);
    if (media == NULL)
      return RTSP_NULL_POINTER;

    handler(client, media, data);
  }

  return RTSP_SUCCESS;
}

void request_play(t_rtsp_client* client, t_rtsp_media* media, void* data)
{
  int n;
  u_rtsp_header buf;
  t_rtsp_rtp_format* fmt = (t_rtsp_rtp_format*)data;

  if (client == NULL || media == NULL)
    return;

#ifdef REPORT_RTSP
  report(" > RTSP Client [%d] PLAY", client->nr);
#endif

  rtsp_request_play(&client->con, client->uri, media->sessionid, media->name, fmt);

  rtsp_default_response_play((void*)&buf);

  // response
  n = rtsp_parse_response(&client->con, tab_response_play, (void*)&buf, 0, CFG_RSP_TIMEOUT);

  if (n == RTSP_SUCCESS) {
      rmcr_play(media, (void*)&buf, &client->con);
  } else if (n == RTSP_RESPONSE_ERROR) {
      if (media->error) media->error(media, (void*)n, &client->con);
  } else {
      perrno("client play internal failure (%d)", n);
  }
}

void request_pause(t_rtsp_client* client, t_rtsp_media* media, void UNUSED *data)
{
  if (client == NULL || media == NULL)
    return;

  rtsp_request_pause(&client->con, client->uri, media->sessionid, media->name);
}

void request_event(t_rtsp_client* client, t_rtsp_media* media, void* data)
{
  uint32_t event;
  if (client == NULL || media == NULL || data == NULL)
    return;

  event = *((uint32_t*)data);
  rtsp_media_event(media, event);

  if (client->task & E_RTSP_CLIENT_PLAY) {
    rtsp_media_play(media);
  }
}

void request_update(t_rtsp_client* client, t_rtsp_media* media, void* data)
{
  t_rtsp_rtp_format fmt;
  uint32_t event;
  char* s;

  if (client == NULL || media == NULL || data == NULL)
    return;

  event = *((uint32_t*)data);
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

  rtsp_request_update(&client->con, client->uri, media->sessionid, media->name, event, &fmt);
}

void request_teardown(t_rtsp_client* client, t_rtsp_media* media, void* data)
{
  u_rtsp_header buf;

  if (client == NULL || media == NULL)
    return;

#ifdef REPORT_RTSP
  report(" > RTSP Client [%d] TEARDOWN (%s)", client->nr, media->name);
#endif

  if (media->sessionid != NULL && strlen(media->sessionid) > 0) {
    // request teardown
    rtsp_request_teardown(&client->con, client->uri, media->sessionid, media->name);

    rtsp_default_response_teardown((void*)&buf);

    // response
    rtsp_parse_response(&client->con, tab_response_teardown, (void*)&buf, 0, CFG_RSP_TIMEOUT);
  }

  // call rmcr_teardown
  teardown_media(client, media, (void*)&buf);
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
    size_t read;

#ifdef REPORT_RTSP_CLIENT
    report(" + RTSP Client [%d] handler", client->nr);
#endif

    while ((n = rtsp_receive(&client->con1, &line, 0, 0, &read)) == RTSP_SUCCESS) {
        tst = line;
        if (!str_starts_with(&tst, RTSP_VERSION)) { // if response
            do {
                msgprintf(&client->con2, "%s\r\n", line);
                if (*line == 0) break;
            } while ((n = rtsp_receive(&client->con1, &line, 0, 0, &read)) == RTSP_SUCCESS);
            if (n != RTSP_SUCCESS) {
                report(ERROR "rtsp client filter receive error on request");
                break;
            }

            rtsp_write(&client->con2);

        } else { // if request

            do {
                msgprintf(&client->con1, "%s\r\n", line);
                if (*line == 0) break;
            } while ((n = rtsp_receive(&client->con1, &line, 0, 0, &read)) == RTSP_SUCCESS);
            if (n != RTSP_SUCCESS) {
                report("rtsp client filter receive error on response");
                break;
            }
            rtsp_write(&client->con1);  //write to pipe 1 ??
        }
    }
    client->task = E_RTSP_CLIENT_KILL;

#ifdef REPORT_RTSP_CLIENT
    report(" - RTSP Client [%d] handler: (%d) %s", client->nr, n, strerror(errno));
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
    t_rtsp_media *media;

    pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);
    pthread_setcanceltype  (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);


    //lock("rtsp_client_req_thread-start");
#ifdef REPORT_RTSP_CLIENT
    report(" + RTSP Client [%d] receiving requests", client->nr);
#endif
    //unlock("rtsp_client_req_thread-start");

    while (!(client->task & E_RTSP_CLIENT_KILL)) {
        memset(&buf, 0, sizeof(u_rtsp_header));
        n = rtsp_parse_request(&client->con2, rtsp_client_methods, &method, (void*)&buf, &common);

        // connection closed...
        if (n) break;

        // find media
        if ((media = get_media(client, common.uri.name)) == NULL)
        {
            report(" ? RTSP Client [%d] no matching media (%s) found", client->nr, common.uri.name);
            rtsp_ommit_body(&client->con, 0, common.content_length);
            rtsp_response_error(&client->con, RTSP_STATUS_DESTINATION_UNREACHABLE, NULL);
            continue;
        }

        lock("rtsp_client_req_thread");

        if(!(client->task & E_RTSP_CLIENT_KILL)) {
#ifdef REPORT_RTSP_RX
            report(" < RTSP Client [%d] %s", client->nr, common.rq.method);
#endif
            // process request (function responses for itself)
            n = ((frtspm*)method->fnc)(media, (void*)&buf, &client->con);
            if (n != RTSP_SUCCESS) {
                report(" ? RTSP Client [%d] execute method \"%s\" error (%d) media (%s)", client->nr, common.rq.method, n, media->name);
                unlock("rtsp_client_req_thread");
                break;
            }
        }
        unlock("rtsp_client_req_thread");

    }

    //lock("rtsp_client_req_thread-close");
#ifdef REPORT_RTSP_CLIENT
    report(" - RTSP Client [%d] request thread ended", client->nr);
#endif
    //unlock("rtsp_client_req_thread-close");

    return 0;
}

/** Opens a new rtsp client connection
 *
 * !!! hdoipd must be locked !!! (because media is manipulated)
 */
t_rtsp_client* rtsp_client_open(char* address, t_rtsp_media *media)
{
    static int nr = 1;
    char buf[200];
    int fd, port, res, ret = RTSP_SUCCESS;
    long arg;
    t_str_uri uri;
    struct hostent* host;
    struct in_addr addr;
    struct sockaddr_in dest_addr;
    fd_set myset;
    struct timeval timeout;
    int valopt;
    socklen_t lon;

    if (media != NULL && media->creator != NULL) {
      report(ERROR "rtsp_client_open: media (%s) already belongs to a client", media->name);
      return NULL;
    }

    t_rtsp_client* client = malloc(sizeof(t_rtsp_client));
    if (client == NULL) {
        report(ERROR "rtsp_client_open(): out of memory");
        return NULL;
    }

    memset(client, 0, sizeof(t_rtsp_client));
    client->task = 0;
    client->nr = nr++;
    client->timeout.alive_ping = 1;

#ifdef REPORT_RTSP_CLIENT
    report(" + RTSP Client [%d] open %s", client->nr, address);
#endif

    memcpy(client->uri, address, strlen(address)+1);
    memcpy(buf, address, strlen(address)+1);

    if (!str_split_uri(&uri, buf)) {
        report("uri error: rtsp://server[:port][/name][/control]");
        ret = RTSP_CLIENT_ERROR;
    }

    if (ret == RTSP_SUCCESS) {
        host = gethostbyname(uri.server);
        if (!host) {
            report(ERROR "rtsp_client_open(): unknown host in uri");
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
        // Set non-blocking
        if ((arg = fcntl(fd, F_GETFL, NULL)) < 0) {
            close(fd);
            report(ERROR "RTSP Client [%d]: error getting socket's access mode: %s", client->nr, strerror(errno));
            ret = RTSP_CLIENT_ERROR;
        }
        else {
            arg |= O_NONBLOCK;
            if (fcntl(fd, F_SETFL, arg) < 0) {
                close(fd);
                report(ERROR "RTSP Client [%d]: error setting socket to non-blocking mode: %s", client->nr, strerror(errno));
                ret = RTSP_CLIENT_ERROR;
            }
        }
    }

    if (ret == RTSP_SUCCESS)
    {
      // setup own address
      memset(&dest_addr, 0, sizeof(struct sockaddr_in));
      dest_addr.sin_family = AF_INET;
      dest_addr.sin_port = port;
      dest_addr.sin_addr.s_addr = addr.s_addr;        // remote address

      if (connect(fd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_in)) < 0) {
          ret = RTSP_TIMEOUT;
          if (errno == EINPROGRESS) {
              timeout.tv_sec = reg_get_int("rtsp-timeout");
              timeout.tv_usec = 0;
              FD_ZERO(&myset);
              FD_SET(fd, &myset);

              res = select(fd + 1, NULL, &myset, NULL, &timeout);
              if (res < 0 && errno != EINTR) {
                  report(ERROR "RTSP Client [%d]: connection refused: %s", client->nr, strerror(errno));
              }
              else if (res > 0) {
                  // socket selected for write
                  lon = sizeof(int);
                  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) {
                      report(ERROR "RTSP Client [%d]: error in getsockopt: %s", client->nr, strerror(errno));
                  }
                  // check the returned value
                  else if (valopt) {
                      report(ERROR "RTSP Client [%d]: error in delayed connection (%d): %s", client->nr, valopt, strerror(valopt));
                  }
                  else
                    ret = RTSP_SUCCESS;
              }
              else  {
                  report(ERROR "RTSP Client [%d]: connection timed out", client->nr);
              }
          }

          if (ret != RTSP_SUCCESS) {
              close(fd);
              ret = RTSP_CLIENT_ERROR;
          }
      }
    }

    if (ret == RTSP_SUCCESS)
    {
        // Set to blocking mode again...
        if ((arg = fcntl(fd, F_GETFL, NULL)) < 0) {
            close(fd);
            report(ERROR "RTSP Client [%d]: error getting socket's access mode: %s", client->nr, strerror(errno));
            ret = RTSP_CLIENT_ERROR;
        }
        else {
            arg &= (~O_NONBLOCK);
            if (fcntl(fd, F_SETFL, arg) < 0) {
                close(fd);
                report(ERROR "RTSP Client [%d]: error setting socket to blocking mode: %s", client->nr, strerror(errno));
                ret = RTSP_CLIENT_ERROR;
            }
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
        if (media) {
            add_media(client, media);

            unlock("rtsp_client_split");
                pthread(client->th1, rtsp_client_thread, client);
                pthread(client->th2, rtsp_client_req_thread, client);
            lock("rtsp_client_split");
        }
        client->open = true;
    } else {
        free(client);
        client = NULL;
        // tell media open failed
        if (media && media->error) media->error(media, 0, 0);
    }

    return client;
}

int rtsp_client_add_media(t_rtsp_client* client, t_rtsp_media *media)
{
  if (client == NULL || media == NULL)
    return RTSP_NULL_POINTER;

  if (rtsp_media_active(media))
    return RTSP_WRONG_STATE;

  if (media->creator) {
    report(" ? RTSP Client [%d]: media (%s) already has a client", client->nr, media->name);
    return RTSP_CLIENT_ERROR;
  }

  add_media(client, media);
  return RTSP_SUCCESS;
}

/** Closes and removes a rtsp client connection
 *
 * !!! hdoipd must be locked !!! (because client-list is manipulated)
 *
 * param client     The client to close
 * param freeClient Whether to free the client structure or not. This is necessary
 *                  because we can't always reset hdoipd.client to NULL so we simply
 *                  don't auto-free it but set t_rtsp_client::open to false and let
 *                  hdoipd_fsm handle the freeing.
 */
int rtsp_client_close(t_rtsp_client* client, bool freeClient)
{
    if (client == NULL)
        return RTSP_NULL_POINTER;

    if (!client->open)
      return RTSP_SUCCESS;

    client->task = E_RTSP_CLIENT_KILL;
    client->open = false;
    shutdown(client->con.fdw, SHUT_RDWR);
    close(client->con.fdw);

#ifdef REPORT_RTSP_CLIENT
        report(" i RTSP Client [%d] closing", client->nr);
#endif

    // detach client from media
    if (client->media != NULL) {
        bstmap_traverse(client->media, detach_media, client);
        bstmap_freep(&client->media);
        client->media = NULL;

#ifdef REPORT_RTSP_CLIENT
        report(" i RTSP Client [%d] waiting for handler to finish", client->nr);
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
    report(" - RTSP Client [%d] closed", client->nr);
#endif

    if (freeClient)
        free(client);

    return RTSP_SUCCESS;
}

// returns response code
int rtsp_client_setup(t_rtsp_client* client, t_rtsp_media* media, t_rtsp_transport* transport, t_rtsp_edid *edid, t_rtsp_hdcp *hdcp)
{
    int n;
    u_rtsp_header buf;
    t_rtsp_header_common common;

    if (media == NULL)
      return RTSP_NULL_POINTER;

    if (get_media(client, media->name) == NULL)
      add_media(client, media);

#ifdef REPORT_RTSP
    report(" > RTSP Client [%d] SETUP media-control %s", client->nr, media->name);
#endif

    // send setup message
    rtsp_request_setup(&client->con, client->uri, media->name, transport, edid, hdcp);

    rtsp_default_response_setup((void*)&buf);

    // response
    n = rtsp_parse_response(&client->con, tab_response_setup, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    if (n == RTSP_SUCCESS) {
        size_t length = strlen(common.session);
        media->sessionid = (char*)malloc(length + 1);
        if (media->sessionid != NULL)
        {
          memset(media->sessionid, 0, length + 1);
          strcpy(media->sessionid, common.session);
        }

        rmcr_setup(media, (void*)&buf);
    } else if (n == RTSP_RESPONSE_ERROR) {
        if (media->error) media->error(media, (void*)n, &client->con);
    } else {
        perrno("internal failure (%d)", n);
    }

    return n;
}

int rtsp_client_play(t_rtsp_client* client, t_rtsp_rtp_format* fmt, char* mediaName)
{
  return request(client, mediaName, fmt, request_play);
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
    // see if there's a video-control available for this client
    t_rtsp_media *media = get_media(client, "video");
    // if not there's nothing to do here
    if (media == NULL)
      return RTSP_NULL_POINTER;

    media->hdcp_var.repeater = 0;  //repeater value
    t_rtsp_rsp_hdcp* p;
    p = (t_rtsp_rsp_hdcp*)&buf;

    /* Get the encrypted HDCP keys from flash and decrypt them*/
    hdcp_decrypt_flash_keys();
    report(INFO "Ask server to start session key exchange");

    /* send start hdcp */
    rtsp_request_hdcp(&client->con, client->uri, media->sessionid, media->name, id[0], content);
    rtsp_default_response_hdcp((void*)&buf);
    // response
    n = rtsp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "02")) return RTSP_HDCP_ERROR;  //check if correct message was received
    strcpy(media->hdcp_var.rtx, p->content);  //save rtx

    /* Add repeater value to certificate */
    memmove(hdoipd.hdcp.certrx + 1,hdoipd.hdcp.certrx,1045);
    if (media->hdcp_var.repeater == 1) hdoipd.hdcp.certrx[0]='1';
    else hdoipd.hdcp.certrx[0]='0';

    /* send certificate */
    rtsp_request_hdcp(&client->con, client->uri, media->sessionid, media->name, id[3], hdoipd.hdcp.certrx);
    rtsp_default_response_hdcp((void*)&buf);
    // response
    n = rtsp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "04")) return RTSP_HDCP_ERROR;  //check if correct message was received

    /* decrypt km */
    rsaes_decrypt(p->content, media->hdcp_var.km, hdoipd.hdcp.p, hdoipd.hdcp.q,hdoipd.hdcp.dp,hdoipd.hdcp.dq,hdoipd.hdcp.qInv);
    //report(INFO "km: %s",client->media->hdcp_var.km); //SECRET VALUE, SHOW ONLY TO DEBUG
    report(INFO "rtx: %s",media->hdcp_var.rtx);

    /* send rrx  */
    hdcp_generate_random_nr(media->hdcp_var.rrx);
    rtsp_request_hdcp(&client->con, client->uri, media->sessionid, media->name, id[6], media->hdcp_var.rrx);
    rtsp_default_response_hdcp((void*)&buf);
    /* response (only acknowledge, therefore ignore content) */
    n = rtsp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    if (strcmp(p->id, "00")) return RTSP_HDCP_ERROR;  //check if correct message was received

    /* send AKE send H_prime */
    report(INFO "rtx: %s", media->hdcp_var.rtx);
    hdcp_calculate_h(media->hdcp_var.rtx, &media->hdcp_var.repeater, H, media->hdcp_var.km, media->hdcp_var.kd);
    report(INFO "H: %s", H);
    rtsp_request_hdcp(&client->con, client->uri, media->sessionid, media->name, id[7], H);
    rtsp_default_response_hdcp((void*)&buf);
    // response LC_init
    n = rtsp_parse_response(&client->con, tab_response_hdcp, (void*)&buf, &common, CFG_RSP_TIMEOUT);

    report(INFO "ID: %s",p->id);
    report(INFO "Content: %s",p->content);
    if (strcmp(p->id, "09")) return RTSP_HDCP_ERROR;  //check if correct message was received
    strcpy(media->hdcp_var.rn, p->content);

    /* generate HMAC of rn and send it back*/
    report(INFO "rn: %s", media->hdcp_var.rn);
    report(INFO "rrx: %s", media->hdcp_var.rrx);
    hdcp_calculate_l(media->hdcp_var.rn, media->hdcp_var.rrx, media->hdcp_var.kd, L);
    report(INFO "L: %s", L);
    rtsp_request_hdcp(&client->con, client->uri, media->sessionid, media->name, id[10], L);
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
    hdcp_ske_dec_ks(media->hdcp_var.rn, ks, p->content, media->hdcp_var.rtx, media->hdcp_var.rrx, media->hdcp_var.km);
    //report(INFO "THE SESSION KEY: %s", ks); //SECRET VALUE; SHOW ONLY TO DEBUG
    report(INFO "RIV: %s", riv);
  /* xor session key with lc128 */
  xor_strings(ks, hdoipd.hdcp.lc128, ks,32);
    /* write keys to HW and enable encryption*/
    hdcp_convert_sk_char_int(ks, riv, hdoipd.hdcp.keys);
    hdoipd.hdcp.ske_executed = HDCP_SKE_EXECUTED;
    report(INFO "SESSION KEY EXCHANGE SUCCESSFUL!");
    return RTSP_SUCCESS;
}

int rtsp_client_get_parameter(t_rtsp_client* client)
{
  u_rtsp_header buf;
  int ret = RTSP_SUCCESS;

  if (client == NULL)
    return RTSP_NULL_POINTER;

#ifdef REPORT_RTSP_HELLO
  report(" > RTSP Client [%d] GET_PARAMETER", client->nr);
#endif

  rtsp_request_get_parameter(&client->con, client->uri);
  rtsp_default_response((void*)&buf);
  if ((ret = rtsp_parse_response(&client->con, tab_response_get_parameter, (void*)&buf, 0, hdoipd.eth_timeout)) != RTSP_SUCCESS)
  {
    rtsp_client_set_kill(client);
    osd_permanent(true);
    osd_printf("vrb.video connection lost...\n");
  }

  return ret;
}

int rtsp_client_event(t_rtsp_client* client, uint32_t event)
{
  int n;
  if (client == NULL)
    return RTSP_NULL_POINTER;

  if ((n = request(client, NULL, &event, request_event)) != RTSP_SUCCESS) {
    report(" ? rtsp_client_event(%d): failed (%d)", client->nr, n);
    client->task = 0;
    return n;
  }

  if (client->task & E_RTSP_CLIENT_TEARDOWN)
    n = rtsp_client_teardown(client, NULL);
  if (client->task & E_RTSP_CLIENT_KILL)
    n = rtsp_client_close(client, false);
  client->task = 0;

  return n;
}

int rtsp_client_update(t_rtsp_client* client, uint32_t event, char* mediaName)
{
  return request(client, mediaName, &event, request_update);
}

int rtsp_client_pause(t_rtsp_client* client, char* mediaName)
{
  return request(client, mediaName, NULL, request_pause);
}

int rtsp_client_teardown(t_rtsp_client* client, char* mediaName)
{
  return request(client, mediaName, NULL, request_teardown);
}

int rtsp_client_set_play(t_rtsp_client* client)
{
  client->task |= E_RTSP_CLIENT_PLAY;

  return RTSP_SUCCESS;
}

int rtsp_client_set_teardown(t_rtsp_client* client)
{
  client->task |= E_RTSP_CLIENT_TEARDOWN;

  return RTSP_SUCCESS;
}

int rtsp_client_set_kill(t_rtsp_client* client)
{
  client->task |= E_RTSP_CLIENT_KILL;

  return RTSP_SUCCESS;
}
