/*
 * rtsp_listener.c
 *
 *  Created on: 19.11.2010
 *      Author: alda
 *
 * Functions to start server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtsp.h"
#include "rtsp_net.h"
#include "rtsp_listener.h"
#include "rtsp_server.h"
#include "hdoipd.h"

/* listener_lock mutex for connection list
 *
 * @param handle listener struct
 */
static void listener_lock(t_rtsp_listener* handle, const char* s MAYBE_UNUSED)
{
    pthread_mutex_lock(&handle->mutex);
    report2("rtsp_listener:pthread_mutex_lock(%x:%d, %s)", handle, pthread_self(), s);
}

/* listener_unlock mutex for connection list
 *
 * @param handle listener struct
 */
static void listener_unlock(t_rtsp_listener* handle, const char* s MAYBE_UNUSED)
{
    report2("rtsp_listener:pthread_mutex_unlock(%x:%d, %s)", handle, pthread_self(), s);
    pthread_mutex_unlock(&handle->mutex);
}

typedef struct {
  t_rtsp_server* server;
  t_node*        sessions;
} t_rtsp_listener_server_sessions;

void rtsp_listener_traverse_sessions_remove(char *key, char* value, void* d)
{
  t_rtsp_server* server = NULL;
  t_rtsp_listener_server_sessions* data = NULL;

  if (key == NULL || value == NULL || d == NULL)
    return;

  server = (t_rtsp_server*)value;
  data = (t_rtsp_listener_server_sessions*)d;

  if (server == data->server)
    queue_put(data->sessions, key);
}

/**
 * lock must already be held!!!
 */
void rtsp_listener_remove_sessions_from_server(t_rtsp_listener* handle, t_rtsp_server* server)
{
  t_rtsp_listener_server_sessions data;
  char *sessionid = NULL;

  if (handle == NULL || server == NULL)
    return;

  data.server = server;
  data.sessions = queue_create();

  // go through all sessions and compare the master
  bstmap_traverse(handle->sessions, rtsp_listener_traverse_sessions_remove, &data);

  while((sessionid = queue_get(data.sessions)) != NULL)
    rtsp_listener_remove_session(handle, sessionid);

  free(data.sessions);
}

void* rtsp_listener_run_server(t_rtsp_server* _server)
{
	t_rtsp_server *server = _server;
	t_rtsp_listener *listener = server->owner;

	rtsp_server_thread(server);

    // remove from list & delete server
    lock("rtsp_listener_run_server");
		listener_lock(listener, "rtsp_listener_run_server");
            list_remove(server->idx);
			if (server->con.fdr != -1) {
	            shutdown(server->con.fdr, SHUT_RDWR);
                close(server->con.fdr);
				server->con.fdr = -1;
			}

            rtsp_listener_remove_sessions_from_server(listener, server);
			free(server);
		listener_unlock(listener, "rtsp_listener_run_server");
    unlock("rtsp_listener_run_server");
    report("shut down server");
    return 0;
}

/** start server
 *
 * */
int rtsp_listener_start_server(t_rtsp_listener* handle, int fd, struct sockaddr_in* addr)
{
    pthread_t th;
    pthread_attr_t attr;
    t_rtsp_server* con = 0;

    if (!(con = rtsp_server_create(fd, addr->sin_addr.s_addr))) {
        return RTSP_ERRORNO;
    }

    // add a server to the list
    listener_lock(handle, "rtsp_listener_start_server");
        con->idx = list_add(handle->cons, con);
        con->owner = handle;
    listener_unlock(handle, "rtsp_listener_start_server");

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthreada(th, &attr, rtsp_listener_run_server, con);

    return RTSP_SUCCESS;
}

void rtsp_listener_close_server(t_rtsp_server* con, t_rtsp_listener *handle)
{
    listener_lock(handle, "rtsp_listener_close_server");
        if (list_contains(handle->cons, con)) {
            if (shutdown(con->con.fdw, SHUT_RDWR) == -1) {
                report(ERROR "close connection error: %s", strerror(errno));
            }
            close(con->con.fdw);
            con->con.fdw = -1;
        }
    listener_unlock(handle, "rtsp_listener_close_server");
}

/* listener thread
 * create socket and wait for connections
 *
 * @param handle listener struct
 * @return rtsp error code
 */
void* rtsp_listener_thread(void *_handle)
{
	t_rtsp_listener *handle = _handle;
    struct sockaddr_in this_addr;
    struct sockaddr_in remote_addr;
    int fd;
    socklen_t length;

    length = sizeof(struct sockaddr_in);

    report(" + RTSP Listener [%d] thread", handle->nr);

    if ((handle->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        report(ERROR "rtsp listener socket error: %s", strerror(errno));
        return (void*)RTSP_ERRORNO;
    }

    // setup own address
    memset(&this_addr, 0, length);
    this_addr.sin_family = AF_INET;
    this_addr.sin_port = htons(handle->port);
    this_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(handle->sockfd, (struct sockaddr*)&this_addr, length) == -1) {
        report(ERROR "rtsp listener bind error: %s", strerror(errno));
        return (void*)RTSP_ERRORNO;
    }

    if (listen(handle->sockfd, RTSP_BACKLOG) == -1) {
        report(ERROR "rtsp listener listen error: %s", strerror(errno));
        return (void*)RTSP_ERRORNO;
    }

    // run... and make new thread for every incoming connection
    while (handle->run) {

        memset(&remote_addr, 0, length);

        if ((fd = accept(handle->sockfd, (struct sockaddr*)&remote_addr, &length)) != -1) {
            rtsp_listener_start_server(handle, fd, &remote_addr);
        } else if (fd == -1) {
            report(ERROR "rtsp listener accept error: %s", strerror(errno));
        }

    }

    // cleanup child threads...
    listener_lock(handle, "rtsp_listener_thread end");
        t_node* cons = list_copy(handle->cons);
    listener_unlock(handle, "rtsp_listener_thread end");

    // the thread removes itself from handle->cons
    list_traverse_free(cons, (f_node_callback*)rtsp_listener_close_server, handle);

    report(DEL "RTSP Listener [%d] thread", handle->nr);

    return (void*)RTSP_SUCCESS;
}

/* listener thread start
 *
 * !!! hdoipd may not be locked !!!
 *
 * @param handle listener struct
 * @param port port where to listen
 * @return rtsp error code
 */
int rtsp_listener_start(t_rtsp_listener* handle, int port)
{
    static int nr = 1;

    if (handle == NULL)
      return RTSP_NULL_POINTER;

    if (handle->run)
      return RTSP_SUCCESS;

    handle->media = NULL;
    handle->sessions = NULL;
    handle->port = port;
    handle->run = true;
    handle->cons = list_create();
    handle->kills = queue_create();
    handle->nr = nr++;

    report(NEW "RTSP Listener [%d]", handle->nr);

    pthread_mutex_init(&handle->mutex, NULL);
    pthread(handle->th, rtsp_listener_thread, handle);

    return RTSP_SUCCESS;
}

/* listener thread close
 *
 * @param handle listener struct
 * @return rtsp error code
 */
int rtsp_listener_close(t_rtsp_listener* handle)
{
    handle->run = false;

    if (shutdown(handle->sockfd, SHUT_RDWR) == -1) {
        report(ERROR "close socket error: %s", strerror(errno));
    }
    close(handle->sockfd);

    report(DEL "RTSP Listener [%d]", handle->nr);

    pthread_join(handle->th, 0);

    pthread_mutex_destroy(&handle->mutex);

    return 0;
}

int rtsp_listener_add_media(t_rtsp_listener* handle, t_rtsp_media* media)
{
    if (media->state == RTSP_STATE_NONE){
        media->state = RTSP_STATE_INIT;
    }
    

    listener_lock(handle, "rtsp_listener_add_media");
        bstmap_setp(&handle->media, media->name, media);
        report(NEW "RTSP Listener [%d] add \"%s\"", handle->nr, media->name);
    listener_unlock(handle, "rtsp_listener_add_media");
    return RTSP_SUCCESS;
}

int rtsp_listener_add_kill(t_rtsp_listener* handle, t_rtsp_server* server)
{
    if (handle == NULL || server == NULL)
        return RTSP_NULL_POINTER;

    queue_put(handle->kills, server);
    report(DEL "RTSP Listener [%d] add server (%d) to kill list", handle->nr, server->nr);
    return RTSP_SUCCESS;
}

t_rtsp_media* rtsp_listener_get_media(t_rtsp_listener* handle, char* name)
{
    t_rtsp_media* ret = NULL;
    listener_lock(handle, "rtsp_listener_get_media");
        ret = (t_rtsp_media*)bstmap_get(handle->media, name);
    listener_unlock(handle, "rtsp_listener_get_media");
    return ret;
}

void rtsp_listener_free_media(t_rtsp_listener* handle)
{
    listener_lock(handle, "rtsp_listener_free_media");
        bstmap_freep(&handle->media);
    listener_unlock(handle, "rtsp_listener_free_media");
}

int rtsp_listener_add_session(t_rtsp_listener* handle, char* id, void* owner)
{
    listener_lock(handle, "rtsp_listener_add_session");
        report("add session: handle->sessions @ %d from owner: %d", handle->sessions, owner);
        bstmap_setp(&handle->sessions, id, owner);
        report("add session: handle->sessions @ %d", handle->sessions);
        report(NEW "RTSP Listener [%d] add session %s", handle->nr, id);
    listener_unlock(handle, "rtsp_listener_add_session");
    return RTSP_SUCCESS;
}

/**
 * lock must already be held!!!
 */
int rtsp_listener_remove_session(t_rtsp_listener* handle, char* id)
{
//    listener_lock(handle, "rtsp_listener_remove_session");    
    bstmap_removep(&handle->sessions, id);
    report(DEL "RTSP Listener [%d] remove session %s", handle->nr, id);
//    listener_unlock(handle, "rtsp_listener_remove_session");
    return RTSP_SUCCESS;
}

void* rtsp_listener_get_session(t_rtsp_listener* handle, char* id)
{
    void* ret = NULL;
    listener_lock(handle, "rtsp_listener_get_session");
        ret = (void*)bstmap_getp(handle->sessions, id);
    listener_unlock(handle, "rtsp_listener_get_session");
    return ret;
}

void rtsp_listener_create_sessionid(t_rtsp_listener* handle, char* id)
{
    do {
       // memset(id, 0, sizeof(id));
        snprintf(id, 19, "%d", rand());
    } while (rtsp_listener_get_session(handle, id) != NULL);
}

typedef void ftra(t_rtsp_server* media);

void rtsp_listener_traverse_servers(void* elem, void* data)
{
  t_rtsp_server* server = NULL;
  if (elem == NULL || data == NULL)
    return;

  server = (t_rtsp_server*)elem;
  ((ftra*)data)(server);
}

void rtsp_listener_traverse_sessions(char *key UNUSED, char* value, void* f)
{
  t_rtsp_server* server = NULL;
  if (value == NULL || f == NULL)
    return;

  server = (t_rtsp_server*)value;
  ((ftra*)f)(server);
}

struct t_listener_event {
	uint32_t event;
	t_rtsp_listener* handle;
};

void rtsp_listener_traverse_event(void* elem, void* data)
{
	struct t_listener_event *cookie = data;
	t_rtsp_server* server = (t_rtsp_server*)elem;

	if (elem == NULL || data == NULL)
	  return;

	if (server && server->kill) {
	    rtsp_listener_add_kill(cookie->handle, server);
	} else {
        rtsp_server_event(server, cookie->event);
	}
}

void rtsp_listener_event(t_rtsp_listener* handle, uint32_t event)
{
    struct t_listener_event le;
    t_rtsp_server* server = NULL;

    listener_lock(handle, "rtsp_listener_event");
        le.event = event;
        le.handle = handle;
        list_traverse(handle->cons, rtsp_listener_traverse_event, &le);

        // event may add medias to the kill list
        while ((server = queue_get(handle->kills))) {
            rtsp_server_close(server);
            rtsp_listener_remove_sessions_from_server(handle, server);
        }
    listener_unlock(handle, "rtsp_listener_event");
}

void rtsp_listener_close_all(t_rtsp_listener* handle)
{
    listener_lock(handle, "rtsp_listener_close_all");
        // instances are removed from the list in rtsp_listener_run_server
        list_traverse(handle->cons, rtsp_listener_traverse_servers, rtsp_server_close);
    listener_unlock(handle, "rtsp_listener_close_all");
}

/** Teardown all connections
 *
 * sends a teardown notice to all connected clients.
 *
 * @param handle rtsp listener struct
 */
void rtsp_listener_teardown_all(t_rtsp_listener* handle)
{
    report(INFO "rtsp_listener_teardown_all()");
    listener_lock(handle, "rtsp_listener_teardown_all");
        bstmap_traverse_freep(&handle->sessions, rtsp_listener_traverse_sessions, rtsp_server_teardown);
    listener_unlock(handle, "rtsp_listener_teardown_all");
}

void rtsp_listener_session_traverse(t_rtsp_listener* handle, void (*f)(char*, char*, void*), void* d)
{
    listener_lock(handle, "rtsp_listener_session_traverse");
        bstmap_traverse(handle->sessions, f, d);
    listener_unlock(handle, "rtsp_listener_session_traverse");
}

/* find a connection based on the remote ip address
 *
 * @param connection node in the connection list
 * @param data pointer to the remote ip address
 * @return 0 if the node matches the remote ip address otherwise non-zero
 */
int find_connection_by_ip(void* connection, void* data)
{
  t_node* node = NULL;
  t_rtsp_server* server = NULL;
  uint32_t remote_address;

  if (connection == NULL || data == NULL)
    return -1;

  node = (t_node*)connection;
  if (node->elem == NULL)
    return -1;

  server = (t_rtsp_server*)node->elem;
  remote_address = *(uint32_t*)data;

  // check if the connection address matches the given remote address
  if (server->con.address == remote_address)
    return 0;

  return -1;
}

/* Close the connection matching the given remote ip address
 *
 * @param handle listener instance
 * @param remote_address remote ip address of the connection
 * @param teardown whether or not to teardown any open sessions before closing the connection
 */
void rtsp_listener_close_connection(t_rtsp_listener* handle, uint32_t remote_address, bool teardown)
{
  t_node* node = NULL;
  t_rtsp_server* server = NULL;

  if (handle == NULL)
    return;

  listener_lock(handle, "rtsp_listener_close_connection");
  // try to find the server matching the given remote address
  node = list_find(handle->cons, (f_node_compare*)find_connection_by_ip, &remote_address);
  if (node == NULL || node->elem == NULL){
    listener_unlock(handle, "rtsp_listener_close_connection");
    return;
  }

  server = (t_rtsp_server*)node->elem;

  // first teardown the media-controls if requested
  if (teardown)
    rtsp_server_teardown(server);
  rtsp_listener_remove_sessions_from_server(handle, server);

  // close the connection
  // this will also lead to the removal of the connection from the listener
  rtsp_server_close(server);
  listener_unlock(handle, "rtsp_listener_close_connection");
}
