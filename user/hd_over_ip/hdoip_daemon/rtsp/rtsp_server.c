/*
 * rtsp_server.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "edid.h"
#include "edid_merge.h"
#include "factory_edid.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hoi_drv_user.h"
#include "map.h"
#include "multicast.h"
#include "rtsp_command.h"
#include "rtsp_listener.h"
#include "rtsp_net.h"
#include "rtsp_parse_header.h"
#include "rtsp_server.h"
#include "rtsp_string.h"

typedef void (*traverse_handler)(t_rtsp_server*, t_rtsp_media*, void*);

typedef struct {
  t_rtsp_server *server;
  traverse_handler handler;
  void *data;
} t_server_data;

void cleanup_media(t_rtsp_server* handle, bool remove_session)
{
  // if there are no more media-controls for this server we can clean it up
  if (handle->media == NULL || handle->media_session_count == 0) {
    if (handle->media != NULL) {
      bstmap_freep(&handle->media);
      handle->media = NULL;
    }

    if (remove_session && handle->owner != NULL && handle->sessionid != NULL && strlen(handle->sessionid) > 0) {
      rtsp_listener_remove_session(handle->owner, handle->sessionid);
      memset(handle->sessionid, 0, sizeof(handle->sessionid));
    }
  }
}

/**
 * add the given media to the server's list of active media-controls
 *
 * The name being used in the binary-search-tree for lookups is
 * taken from the "name" member of the media parameter.
 */
int rtsp_server_add_media(t_rtsp_server* handle, t_rtsp_media* media)
{
  if (handle == NULL || media == NULL || media->name == NULL)
    return RTSP_NULL_POINTER;

  report(" ? RTSP Server [%d] adding media (%s)", handle->nr, media->name);
  if (media->sessionid != NULL)
    handle->media_session_count++;
  bstmap_setp(&handle->media, media->name, media);
  return RTSP_SUCCESS;
}

/**
 * Retrieves the media-control matching the given name from the
 * server's list of active media-controls if available
 */
t_rtsp_media* rtsp_server_get_media(t_rtsp_server* handle, char* name)
{
  if (handle == NULL || name == NULL)
    return NULL;

  return (t_rtsp_media*)bstmap_get(handle->media, name);
}

/**
 * removes the media-control matching the given name from the
 * server's list of active media-controls
 */
void rtsp_server_remove_media(t_rtsp_server* handle, t_rtsp_media* media, bool remove_from_list)
{
  if (handle == NULL || media == NULL)
    return;

  report(INFO "RTSP Server [%d] removing media (%s)", handle->nr, media->name);
  if (remove_from_list) {
    if (media->sessionid != NULL)
      handle->media_session_count--;
    bstmap_removep(&handle->media, media->name);
    cleanup_media(handle, true);
  }

  media->creator = NULL;
  free(media);
}

/**
 * A BST traversing method invoking the method specified
 * in t_server_data.handler (stored in the data parameter).
 *
 * param key Name of the media-control
 * param value pointer to t_rtsp_media structure
 * param data pointer to t_server_data structure
 */
void traverse_dispatcher(char *key, char* value, void* data)
{
  t_rtsp_media *media = (t_rtsp_media*)value;
  t_server_data *serverData = (t_server_data*)data;
  t_rtsp_server *server = serverData->server;

  if (key == NULL || server == NULL || media == NULL || serverData->handler == NULL)
    return;

  serverData->handler(server, media, serverData->data);
}

int traverse(t_rtsp_server* server, char* mediaName, void* data, traverse_handler handler)
{
  t_server_data serverData;
  t_rtsp_media* media = NULL;
  if (server == NULL || server->media == NULL || handler == NULL)
    return RTSP_NULL_POINTER;

  if (bstmap_empty(server->media))
    return RTSP_SUCCESS;

  if (mediaName == NULL)
  {
    serverData.server = server;
    serverData.handler = handler;
    serverData.data = data;

    bstmap_traverse(server->media, traverse_dispatcher, &serverData);
  }
  else
  {
    media = rtsp_server_get_media(server, mediaName);
    if (media == NULL)
      return RTSP_NULL_POINTER;

    handler(server, media, data);
  }

  return RTSP_SUCCESS;
}

void traverse_remove(t_rtsp_server* server, t_rtsp_media* media, void* data)
{
  if (server == NULL || media == NULL)
    return;

  // call the teardown implementation of the media-control
  rmcr_teardown(media, NULL);

  // remove the media-control from this connection
  rtsp_server_remove_media(server, media, false);
}

void traverse_teardown(t_rtsp_server* server, t_rtsp_media* media, void* data)
{
  char *uri = RTSP_SCHEME "://255.255.255.255:65536";
  struct in_addr a1;

  if (server == NULL || media == NULL)
    return;

  a1.s_addr = server->con.address;
  sprintf(uri, "%s://%s", RTSP_SCHEME, inet_ntoa(a1));

  // a server connection is active for this media -> use it to send a teardown message
  rtsp_request_teardown(&server->con, uri, media->sessionid, media->name);

  traverse_remove(server, media, NULL);
}

void traverse_event(t_rtsp_server* server, t_rtsp_media* media, void* data)
{
  uint32_t event;

  if (server == NULL || media == NULL || data == NULL)
    return;

  event = *((uint32_t*)data);
  rtsp_media_event(media, event);
}

void traverse_update(t_rtsp_server* server, t_rtsp_media* media, void* data)
{
  t_rtsp_rtp_format fmt;
  char *s;
  char *uri = RTSP_SCHEME "://255.255.255.255:65536";
  struct in_addr a1;
  uint32_t event;

  if (server == NULL || media == NULL || data == NULL)
    return;

  event = *((uint32_t*)data);
  a1.s_addr = server->con.address;
  sprintf(uri, "%s://%s", RTSP_SCHEME, inet_ntoa(a1));

  s = reg_get("compress");
  if (strcmp(s, "jp2k") == 0)
    fmt.compress = FORMAT_JPEG2000;
  else
    fmt.compress = FORMAT_PLAIN;

  hoi_drv_get_mtime(&fmt.rtptime);
  fmt.value = reg_get_int("advcnt-min");

#ifdef REPORT_RTSP_UPDATE
  report(" > RTSP Server [%d] UPDATE", server->nr);
#endif

  rtsp_request_update(&server->con, uri, media->sessionid, media->name, event, &fmt);
}

void traverse_pause(t_rtsp_server* server, t_rtsp_media* media, void* data)
{
  char *uri = RTSP_SCHEME "://255.255.255.255:65536";
  struct in_addr a1;

  if (server == NULL || media == NULL)
    return;

  a1.s_addr = server->con.address;
  sprintf(uri, "%s://%s", RTSP_SCHEME, inet_ntoa(a1));

  rtsp_request_pause(&server->con, uri, media->sessionid, media->name);

  rmsr_pause(media, 0);
}

void remove_media_all(t_rtsp_server* server, bool remove_session)
{
  if (server == NULL)
    return;

  server->media_session_count = 0;
  cleanup_media(server, remove_session);
}

/**
 * create a new server instance and initialize the connections
 * necessary for RTSP
 *
 * sa rtsp_coninit
 */
t_rtsp_server* rtsp_server_create(int fd, uint32_t addr)
{
    static int nr = 1;
    t_rtsp_server* server = malloc(sizeof(t_rtsp_server));
    if (server) {
        memset(server, 0, sizeof(t_rtsp_server));
        rtsp_coninit(&server->con, fd, addr);
        server->nr = nr++;
        server->timeout.timeout = 0;
        server->kill = false;
        server->open = true;
#ifdef REPORT_RTSP_SERVER
        report(" + RTSP Server [%d] for %s", server->nr, str_ntoa(addr));
#endif
    } else {
        report(ERROR "rtsp_server_create.malloc: out of memory");
    }
    return server;
}

/**
 * process incoming RTSP messages
 *
 * Format is the following:
 *  - Request Line [CRLF]
 *  - n * Header Line [CRLF]
 *  - [CRLF]
 *  - { Optional Body }
 *
 * sa rtsp_parse_request
 * sa rtsp_server_get_media
 */
int rtsp_server_thread(t_rtsp_server* handle)
{
    int n;
    t_rtsp_header_common common;
    const t_map_set* method;
    t_rtsp_media *media = NULL, *media_box = NULL;
    u_rtsp_header buf;
    bool media_new = true;

#ifdef REPORT_RTSP_SERVER
    report(INFO "RTSP Server [%d] started", handle->nr);
#endif

    lock("rtsp_server_thread");

    // getting box_sys_vtb media control for this server
    media = rtsp_listener_get_media(handle->owner, "");
    if (media != NULL)
    {
        media_box = malloc(sizeof(t_rtsp_media));
        if (media_box != NULL)
        {
          memcpy(media_box, media, sizeof(t_rtsp_media));
          media_box->top = handle->owner;
          media_box->creator = handle;
          media_box->owner = media;
          media_box->state = RTSP_STATE_INIT;
          rtsp_server_add_media(handle, media_box);
        }

        media = NULL;
    }

    unlock("rtsp_server_thread");

    // receive request line
    while (handle->open) {
        memset(&buf, 0, sizeof(u_rtsp_header));
        n = rtsp_parse_request(&handle->con, rtsp_srv_methods, &method, &buf, &common);

        // request has already been handled (probably because an error occured)
        if (n == RTSP_HANDLED) continue;

        // connection closed...
        if (n) {
            if (n > RTSP_PAUSE)
                report(ERROR "RTSP Server [%d] failed to parse request (%d)", handle->nr, n);
            else
                report(INFO "RTSP Server [%d] closed by remote (%d)", handle->nr, n);
            break;
        }

        // find media
        media = rtsp_server_get_media(handle, common.uri.name);
        media_new = media == NULL;
        if (media_new) {
            // fall back to the general list of media-controls available
            media = rtsp_listener_get_media(handle->owner, common.uri.name);
        }

        // fail response
        if (media == NULL) {
            report(" ? RTSP Server [%d] no matching media (%s) found", handle->nr, common.uri.name);
            rtsp_ommit_body(&handle->con, 0, common.content_length);
            rtsp_response_error(&handle->con, RTSP_STATUS_DESTINATION_UNREACHABLE, NULL);
            continue;
        }

        // check if the request is valid (session, state, media-control)
        n = rtsp_media_check_request(method, media, &buf, &handle->con);
        if (n == RTSP_HANDLED)
        {
            report(" ? invalid request on method (%s) in media-control (%s)", method->name, media->name);
            rtsp_ommit_body(&handle->con, 0, common.content_length);
            continue;
        }
        if (n != RTSP_SUCCESS) {
            report(" ? RTSP Server [%d] request check failed on method (%s) in media-control (%s)", handle->nr, method->name, media->name);
            rtsp_response_error(&handle->con, RTSP_STATUS_INTERNAL_SERVER_ERROR, NULL);
            continue;
        }

        lock("rtsp_server_thread");

        if (media_new && media->creator == NULL)
            media->creator = handle;

        memcpy(&handle->con.common, &common, sizeof(t_rtsp_header_common));

#ifdef REPORT_RTSP
        report(" < RTSP Server [%d] %s", handle->nr, common.rq.method);
#endif
        // process request (function responses for itself)
        n = (method->fnc)(media, &buf, &handle->con);
        // media may be not valid anymore!
        if (n != RTSP_SUCCESS && n != RTSP_HANDLED) {
            report(" ? RTSP Server [%d] execute method \"%s\" error (%d) media (%s)", handle->nr, common.rq.method, n, media->name);
            rtsp_response_error(&handle->con, RTSP_STATUS_INTERNAL_SERVER_ERROR, NULL);
        }

        if (media_new)
          media->creator = NULL;

        unlock("rtsp_server_thread");
    }

    lock("rtsp_server_thread");
    traverse(handle, NULL, NULL, traverse_remove);
    remove_media_all(handle, true);
    unlock("rtsp_server_thread");

    handle->kill = true;

#ifdef REPORT_RTSP_SERVER
    report(" - RTSP Server [%d] ended", handle->nr);
#endif
    return n;
}

/**
 * Close TCP connection
 */
void rtsp_server_close(t_rtsp_server* handle)
{
  if (handle == NULL || !handle->open) {
    return;
  }

  handle->open = false;
  traverse(handle, NULL, NULL, traverse_remove);
  remove_media_all(handle, true);

  if (handle->con.fdr != -1) {
    // a server connection is active for this media -> shut it down
#ifdef REPORT_RTSP_SERVER
    report(DEL "RTSP Server [%d] close", handle->nr);
#endif

    if (shutdown(handle->con.fdr, SHUT_RDWR) == -1) {
      report(ERROR "close socket error: %s", strerror(errno));
    }
    close(handle->con.fdr);
    handle->con.fdr = -1;
  }
}

/**
 * Teardown the connection for all media-control
 *
 * This method sends a TEARDOWN request to the client
 * and removes the media-control from the servers list
 * of active media-controls
 *
 * sa rtsp_request_teardown
 * sa rtsp_server_remove_media
 */
void rtsp_server_teardown(t_rtsp_server* handle)
{
  if (handle == NULL)
    return;

  traverse(handle, NULL, NULL, traverse_teardown);
  remove_media_all(handle, false);
  memset(handle->sessionid, 0, sizeof(handle->sessionid));
}

void rtsp_server_event(t_rtsp_server* handle, uint32_t event)
{
  if (handle == NULL)
    return;

  traverse(handle, NULL, &event, traverse_event);
}

void rtsp_server_update(t_rtsp_server* handle, uint32_t event)
{
  if (handle == NULL) {
    report(ERROR "rtsp_server_update: no server");
    return;
  }

  traverse(handle, NULL, &event, traverse_update);
}

void rtsp_server_pause(t_rtsp_server* handle)
{
  if (handle == NULL) {
    report("rtsp server pause: no server");
    return;
  }

  traverse(handle, NULL, NULL, traverse_pause);
}

void rtsp_server_update_media(t_rtsp_media* media, uint32_t event)
{
  if (media == NULL || media->creator == NULL) {
    report(ERROR "rtsp_server_update: no server");
    return;
  }

  traverse(media->creator, media->name, &event, traverse_update);
}

int rtsp_server_handle_setup(t_rtsp_server* handle, t_rtsp_edid *edid)
{
  int edid_length = 256;
  int ret;
  t_edid edid_old;
  uint8_t edid_table[edid_length];

  if (handle == NULL)
    return -1;

  multicast_set_enabled(reg_test("multicast_en", "true"));

  if (edid == NULL || !edid->from_header)
      return 0;

  // use default edid if requested
  if (reg_test("edid-mode", "default")) {
    memcpy(edid_table, factory_edid, edid_length);
  }
  else
    memcpy(edid_table, edid->edid, edid_length);

  if (multicast_get_enabled()) { // multicast
    // we only need to do the edid merging if ...
    // we don't use the default edid and
    // the client has provided an edid and
    // the input is not SDI
    if (!reg_test("edid-mode", "default") && edid->from_header && !hdoipd_rsc(RSC_VIDEO_IN_SDI)) {
        // add connection to start list
        multicast_edid_add((t_edid *)edid_table, handle);
        // this call will block until the multicast setup procedure has been finished
        if (multicast_connection_add(handle) != 0)
          return -1;
    }

    return 0;
  }

  // unicast
  if (!hdoipd_rsc(RSC_VIDEO_IN_SDI)) {
    // read old edid
    ret = edid_read_file(&edid_old, EDID_PATH_VIDEO_IN);
    if (ret != -1) {
      edid_merge((t_edid *)edid_table, (t_edid *)edid_table); // modify edid

      // write edid only when it has changed
      // no previous E-EDID exists
      if (ret == -2)
        edid_write_function((t_edid *)edid_table, "unicast first edid");
      // a previous E-EDID exists
      else {
        // check if edid has changed
        if (multicast_edid_compare(&edid_old, (t_edid *)edid_table))
          edid_write_function((t_edid *)edid_table, "unicast edid changed");
      }
    }
    else
      report(ERROR "Failed to read file: %s", EDID_PATH_VIDEO_IN);
  }

  // response only when not already streaming
  if (!hdoipd_tstate(VTB_VIDEO | VTB_AUDIO)) {
    // make sure the EDID change was successful
    return check_input_after_edid_changed();
  }

  return 0;
}
