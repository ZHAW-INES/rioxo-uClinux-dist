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


typedef void* (pf)(void*);


/* listener_lock mutex for connection list
 *
 * @param handle listener struct
 */
static void listener_lock(t_rtsp_listener* handle, const char* s)
{
    pthread_mutex_lock(&handle->mutex);
    report2("rtsp_listener:pthread_mutex_lock(%x:%d, %s)", handle, pthread_self(), s);
}

/* listener_unlock mutex for connection list
 *
 * @param handle listener struct
 */
static void listener_unlock(t_rtsp_listener* handle, const char* s)
{
    report2("rtsp_listener:pthread_mutex_unlock(%x:%d, %s)", handle, pthread_self(), s);
    pthread_mutex_unlock(&handle->mutex);
}


void* rtsp_listener_run_server(t_rtsp_server* server)
{
	t_rtsp_listener* listener = server->owner;
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

			if(server->media) {
				bstmap_removep(&listener->sessions, server->media->sessionid);
				rmsr_teardown(server->media, 0);
			}

			free(server);
		listener_unlock(listener, "rtsp_listener_run_server");
    unlock("rtsp_listener_run_server");
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
    pthreada(th, &attr, (pf*)rtsp_listener_run_server, con);

    return RTSP_SUCCESS;
}

void rtsp_listener_close_server(t_rtsp_server* con, t_rtsp_listener *handle)
{
    bool valid = false;
    listener_lock(handle, "rtsp_listener_close_server");
        if (list_contains(handle->cons, con)) {
            valid = true;
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
void* rtsp_listener_thread(t_rtsp_listener* handle)
{
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
    handle->media = 0;
    handle->sessions = 0;
    handle->port = port;
    handle->run = true;
    handle->cons = list_create();
    handle->kills = queue_create();
    handle->nr = nr++;

    report(NEW "RTSP Listener [%d]", handle->nr);

    pthread_mutex_init(&handle->mutex, NULL);
    pthread(handle->th, (pf*)rtsp_listener_thread, handle);

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
    if (media->state == RTSP_STATE_NONE) media->state = RTSP_STATE_INIT;

    listener_lock(handle, "rtsp_listener_add_media");
        bstmap_setp(&handle->media, media->name, media);
        report(NEW "RTSP Media add \"%s\"", media->name);
    listener_unlock(handle, "rtsp_listener_add_media");
    return RTSP_SUCCESS;
}

int rtsp_listener_add_kill(t_rtsp_listener* handle, t_rtsp_media* media)
{
    queue_put(handle->kills, media);
    report(DEL "RTSP Media add %s:%s to kill list", media->name, media->sessionid);
    return RTSP_SUCCESS;
}

t_rtsp_media* rtsp_listener_get_media(t_rtsp_listener* handle, char* name)
{
    t_rtsp_media* ret = 0;
    listener_lock(handle, "rtsp_listener_get_media");
        ret = (t_rtsp_media*)bstmap_get(handle->media, name);
    listener_unlock(handle, "rtsp_listener_get_media");
    return ret;
}

void rtsp_listener_free_media(t_rtsp_listener* handle)
{
    report(INFO "rtsp_listener_free_media()");
    listener_lock(handle, "rtsp_listener_free_media");
        bstmap_freep(&handle->media);
    listener_unlock(handle, "rtsp_listener_free_media");
}

int rtsp_listener_add_session(t_rtsp_listener* handle, t_rtsp_media* media)
{
    listener_lock(handle, "rtsp_listener_add_session");
        bstmap_setp(&handle->sessions, media->sessionid, media);
        report(NEW "RTSP Media add session %s:%s", media->name, media->sessionid);
    listener_unlock(handle, "rtsp_listener_add_session");
    return RTSP_SUCCESS;
}

int rtsp_listener_remove_session(t_rtsp_listener* handle, t_rtsp_media* media)
{
    listener_lock(handle, "rtsp_listener_remove_session");
        bstmap_removep(&handle->sessions, media->sessionid);
        report(DEL "RTSP Media remove session(1) %s:%s", media->name, media->sessionid);
    listener_unlock(handle, "rtsp_listener_remove_session");
    return RTSP_SUCCESS;
}

t_rtsp_media* rtsp_listener_get_session(t_rtsp_listener* handle, char* id)
{
    t_rtsp_media* ret = 0;
    listener_lock(handle, "rtsp_listener_get_session");
        ret = (t_rtsp_media*)bstmap_get(handle->sessions, id);
    listener_unlock(handle, "rtsp_listener_get_session");
    return ret;
}

void rtsp_listener_create_sessionid(t_rtsp_listener* handle, char* id)
{
    do {
        snprintf(id, 20, "%d", rand());
    } while (rtsp_listener_get_session(handle, id));
}


typedef void ftra(t_rtsp_media* media);
void rtsp_listener_traverse(char UNUSED *key, char* value, void* f)
{
    ((ftra*)f)((t_rtsp_media*)value);
}
int rtsp_listener_cond(char UNUSED *key, char* value, void* d)
{
    return ((t_rtsp_media*)value)->owner == d;
}

struct t_listener_event {
	uint32_t event;
	t_rtsp_listener* handle;
};

void rtsp_listener_traverse_event(char UNUSED *key, char* value, void* _cookie)
{
	struct t_listener_event *cookie = _cookie;
	t_rtsp_media* media = (t_rtsp_media*)value;
	t_rtsp_server* server = media->creator;

	if (server && server->kill) {
	    rtsp_listener_add_kill(cookie->handle, media);
	} else {
	    rtsp_media_event(media, cookie->event);
	}
}

void rtsp_listener_event(t_rtsp_listener* handle, uint32_t event)
{
	struct t_listener_event le;
    t_rtsp_media* media;
    listener_lock(handle, "rtsp_listener_event");
    	le.event = event;
    	le.handle = handle;
        bstmap_traverse(handle->sessions, rtsp_listener_traverse_event, &le);
        // event may add medias to the kill list
        while ((media = queue_get(handle->kills))) {
            // when we close the sessions, we also must remove it from the listener
            if (bstmap_removep(&handle->sessions, media->sessionid)) {
                report(DEL "RTSP Media remove session(2) %s:%s", media->name, media->sessionid);
                rtsp_server_close(media);
            }
        }
    listener_unlock(handle, "rtsp_listener_event");
}

void rtsp_listener_close_all(t_rtsp_listener* handle)
{
    report(INFO "rtsp_listener_close_all()");
    listener_lock(handle, "rtsp_listener_teardown_all");
        bstmap_traverse_freep(&handle->sessions, rtsp_listener_traverse, rtsp_server_close);
    listener_unlock(handle, "rtsp_listener_teardown_all");
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
        bstmap_traverse_freep(&handle->sessions, rtsp_listener_traverse, rtsp_server_teardown);
    listener_unlock(handle, "rtsp_listener_teardown_all");
}

void rtsp_listener_pause_all(t_rtsp_listener* handle)
{
    report(INFO "rtsp_listener_pause_all()");
    listener_lock(handle, "rtsp_listener_pause_all");
        bstmap_traverse(handle->sessions, rtsp_listener_traverse, rtsp_server_pause);
    listener_unlock(handle, "rtsp_listener_pause_all");
}

void rtsp_listener_session_traverse(t_rtsp_listener* handle, void (*f)(char*, char*, void*), void* d)
{
    listener_lock(handle, "rtsp_listener_session_traverse");
        bstmap_traverse(handle->sessions, f, d);
    listener_unlock(handle, "rtsp_listener_session_traverse");
}
