/*
 * rscp_listener.c
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rscp.h"
#include "rscp_net.h"
#include "rscp_listener.h"
#include "rscp_server.h"
#include "hdoipd.h"


typedef void* (pf)(void*);


/* listener_lock mutex for connection list
 *
 * @param handle listener struct
 */
static void listener_lock(t_rscp_listener* handle, const char* s)
{
    pthread_mutex_lock(&handle->mutex);
    report2("rscp_listener:pthread_mutex_lock(%x:%d, %s)", handle, pthread_self(), s);
}

/* listener_unlock mutex for connection list
 *
 * @param handle listener struct
 */
static void listener_unlock(t_rscp_listener* handle, const char* s)
{
    report2("rscp_listener:pthread_mutex_unlock(%x:%d, %s)", handle, pthread_self(), s);
    pthread_mutex_unlock(&handle->mutex);
}

void* rscp_listener_run_server(t_rscp_server* con)
{
    rscp_server_thread(con);

    shutdown(con->con.fdw, SHUT_RDWR);

    // remove from list & delete server
    listener_lock(con->owner, "rscp_listener_run_server");
        list_remove(con->idx);
        rscp_server_free(con);
    listener_unlock(con->owner, "rscp_listener_run_server");

    return 0;
}

int rscp_listener_start_server(t_rscp_listener* handle, int fd, struct sockaddr_in* addr)
{
    pthread_t th;
    pthread_attr_t attr;
    t_rscp_server* con = 0;

    if (!(con = rscp_server_create(fd, addr->sin_addr.s_addr))) {
        return RSCP_ERRORNO;
    }

    // add a server to the list
    listener_lock(handle, "rscp_listener_start_server");
        con->idx = list_add(handle->cons, con);
        con->owner = handle;
    listener_unlock(handle, "rscp_listener_start_server");

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthreada(th, &attr, (pf*)rscp_listener_run_server, con);

    return RSCP_SUCCESS;
}

void rscp_listener_close_server(t_rscp_server* con, t_rscp_listener *handle)
{
    bool valid = false;
    listener_lock(handle, "rscp_listener_close_server");
        if (list_contains(handle->cons, con)) {
            valid = true;
            if (shutdown(con->con.fdw, SHUT_RDWR) == -1) {
                report("close connection error: %s", strerror(errno));
            }
        }
    listener_unlock(handle, "rscp_listener_close_server");
}

/* listener thread
 *
 * @param handle listener struct
 * @return rscp error code
 */
void* rscp_listener_thread(t_rscp_listener* handle)
{
    struct sockaddr_in this_addr;
    struct sockaddr_in remote_addr;
    socklen_t sin_size;
    int fd;

    report(" + RSCP Listener [%d] thread", handle->nr);

    if ((handle->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        report(" ? rscp listener socket error: %s", strerror(errno));
        return (void*)RSCP_ERRORNO;
    }

    // setup own address
    memset(&this_addr, 0, sizeof(struct sockaddr_in));
    this_addr.sin_family = AF_INET;
    this_addr.sin_port = htons(handle->port);
    this_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(handle->sockfd, (struct sockaddr*)&this_addr, sizeof(struct sockaddr)) == -1) {
        report(" ? rscp listener bind error: %s", strerror(errno));
        return (void*)RSCP_ERRORNO;
    }

    if (listen(handle->sockfd, RSCP_BACKLOG) == -1) {
        report(" ? rscp listener listen error: %s", strerror(errno));
        return (void*)RSCP_ERRORNO;
    }

    // run...
    while (handle->run) {
        sin_size = sizeof(struct sockaddr_in);
        memset(&remote_addr, 0, sizeof(struct sockaddr_in));

        if ((fd = accept(handle->sockfd, (struct sockaddr*)&remote_addr, &sin_size)) != -1) {
            rscp_listener_start_server(handle, fd, &remote_addr);
        } else if (fd == -1) {
            report(" ? rscp listener accept error: %s", strerror(errno));
        }
    }

    // cleanup child threads...
    listener_lock(handle, "rscp_listener_thread end");
        t_node* cons = list_copy(handle->cons);
    listener_unlock(handle, "rscp_listener_thread end");

    // the thread removes itself from handle->cons
    list_traverse_free(cons, (f_node_callback*)rscp_listener_close_server, handle);

    report(" - RSCP Listener [%d] thread", handle->nr);

    return (void*)RSCP_SUCCESS;
}

/* listener thread start
 *
 * !!! hdoipd may not be locked !!!
 *
 * @param handle listener struct
 * @param port port where to listen
 * @return rscp error code
 */
int rscp_listener_start(t_rscp_listener* handle, int port)
{
    static int nr = 1;
    handle->media = 0;
    handle->sessions = 0;
    handle->port = port;
    handle->run = true;
    handle->cons = list_create();
    handle->kills = queue_create();
    handle->nr = nr++;

    report(" + RSCP Listener [%d]", handle->nr);

    pthread_mutex_init(&handle->mutex, NULL);
    pthread(handle->th, (pf*)rscp_listener_thread, handle);

    return RSCP_SUCCESS;
}

/* listener thread close
 *
 * @param handle listener struct
 * @return rscp error code
 */
int rscp_listener_close(t_rscp_listener* handle)
{
    handle->run = false;

    if (shutdown(handle->sockfd, SHUT_RDWR) == -1) {
        report("close socket error: %s", strerror(errno));
    }

    report(" - RSCP Listener [%d]", handle->nr);

    pthread_join(handle->th, 0);

    pthread_mutex_destroy(&handle->mutex);

    return 0;
}

int rscp_listener_add_media(t_rscp_listener* handle, t_rscp_media* media)
{
    listener_lock(handle, "rscp_listener_add_media");
        bstmap_setp(&handle->media, media->name, media);
        report(NEW "RSCP Media add \"%s\"", media->name);
    listener_unlock(handle, "rscp_listener_add_media");
    return RSCP_SUCCESS;
}

int rscp_listener_add_kill(t_rscp_listener* handle, t_rscp_media* media)
{
    queue_put(handle->kills, media);
    report(DEL "RSCP Media add %s:%s to kill list", media->name, media->sessionid);
    return RSCP_SUCCESS;
}

t_rscp_media* rscp_listener_get_media(t_rscp_listener* handle, char* name)
{
    t_rscp_media* ret = 0;
    listener_lock(handle, "rscp_listener_get_media");
        ret = (t_rscp_media*)bstmap_get(handle->media, name);
    listener_unlock(handle, "rscp_listener_get_media");
    return ret;
}

void rscp_listener_free_media(t_rscp_listener* handle)
{
    report("rscp_listener_free_media()");
    listener_lock(handle, "rscp_listener_free_media");
        bstmap_freep(&handle->media);
    listener_unlock(handle, "rscp_listener_free_media");
}

int rscp_listener_add_session(t_rscp_listener* handle, t_rscp_media* media)
{
    listener_lock(handle, "rscp_listener_add_session");
        bstmap_setp(&handle->sessions, media->sessionid, media);
        report(NEW "RSCP Media add session %s:%s", media->name, media->sessionid);
    listener_unlock(handle, "rscp_listener_add_session");
    return RSCP_SUCCESS;
}

int rscp_listener_remove_session(t_rscp_listener* handle, t_rscp_media* media)
{
    listener_lock(handle, "rscp_listener_remove_session");
        bstmap_removep(&handle->sessions, media->sessionid);
        report(DEL "RSCP Media remove session(1) %s:%s", media->name, media->sessionid);
    listener_unlock(handle, "rscp_listener_remove_session");
    return RSCP_SUCCESS;
}

t_rscp_media* rscp_listener_get_session(t_rscp_listener* handle, char* id)
{
    t_rscp_media* ret = 0;
    listener_lock(handle, "rscp_listener_get_session");
        ret = (t_rscp_media*)bstmap_get(handle->sessions, id);
    listener_unlock(handle, "rscp_listener_get_session");
    return ret;
}

void rscp_listener_create_sessionid(t_rscp_listener* handle, char* id)
{
    do {
        snprintf(id, 20, "%d", rand());
    } while (rscp_listener_get_session(handle, id));
}


typedef void ftra(t_rscp_media* media);
void rscp_listener_traverse(char UNUSED *key, char* value, void* f)
{
    ((ftra*)f)((t_rscp_media*)value);
}
int rscp_listener_cond(char UNUSED *key, char* value, void* d)
{
    return ((t_rscp_media*)value)->owner == d;
}

struct t_listener_event {
	uint32_t event;
	t_rscp_listener* handle;
};

void rscp_listener_traverse_event(char UNUSED *key, char* value, void* _cookie)
{
	struct t_listener_event *cookie = _cookie;
	t_rscp_media* media = (t_rscp_media*)value;
	t_rscp_server* server = media->creator;

	if (server && server->kill) {
		rscp_listener_add_kill(cookie->handle, media);
	} else {
		rscp_media_event(media, cookie->event);
	}
}

void rscp_listener_event(t_rscp_listener* handle, uint32_t event)
{
	struct t_listener_event le;
    t_rscp_media* media;
    listener_lock(handle, "rscp_listener_event");
    	le.event = event;
    	le.handle = handle;
        bstmap_traverse(handle->sessions, rscp_listener_traverse_event, &le);
        // event may add medias to the kill list
        while ((media = queue_get(handle->kills))) {
            // when we close the sessions, we also must remove it from the listener
            if (bstmap_removep(&handle->sessions, media->sessionid)) {
                report(DEL "RSCP Media remove session(2) %s:%s", media->name, media->sessionid);
                rscp_server_close(media);
            }
        }
    listener_unlock(handle, "rscp_listener_event");
}

void rscp_listener_close_all(t_rscp_listener* handle)
{
    report("rscp_listener_close_all()");
    listener_lock(handle, "rscp_listener_teardown_all");
        bstmap_traverse_freep(&handle->sessions, rscp_listener_traverse, rscp_server_close);
    listener_unlock(handle, "rscp_listener_teardown_all");
}

/** Teardown all connections
 *
 * sends a teardown notice to all connected clients.
 *
 * @param handle rscp listener struct
 */
void rscp_listener_teardown_all(t_rscp_listener* handle)
{
    report("rscp_listener_teardown_all()");
    listener_lock(handle, "rscp_listener_teardown_all");
        bstmap_traverse_freep(&handle->sessions, rscp_listener_traverse, rscp_server_teardown);
    listener_unlock(handle, "rscp_listener_teardown_all");
}

void rscp_listener_pause_all(t_rscp_listener* handle)
{
    report("rscp_listener_pause_all()");
    listener_lock(handle, "rscp_listener_pause_all");
        bstmap_traverse(handle->sessions, rscp_listener_traverse, rscp_server_pause);
    listener_unlock(handle, "rscp_listener_pause_all");
}

void rscp_listener_session_traverse(t_rscp_listener* handle, void (*f)(char*, char*, void*), void* d)
{
    listener_lock(handle, "rscp_listener_session_traverse");
        bstmap_traverse(handle->sessions, f, d);
    listener_unlock(handle, "rscp_listener_session_traverse");
}
