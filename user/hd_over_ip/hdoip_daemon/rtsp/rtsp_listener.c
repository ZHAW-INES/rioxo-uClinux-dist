/*
 * rtsp_listener.c
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#include "rtsp.h"
#include "rtsp_listener.h"
#include "rtsp_server.h"


/* lock mutex for connection list when run
 *
 * lock mutex for connection list only when run flag is true.
 *
 * @param handle listener struct
 * @return true when mutex locked and run is true, otherwise false
 */
static bool lock_run(t_rtsp_listener* handle)
{
    bool ret;
    pthread_mutex_lock(&handle->mutex_run);
    ret = handle->run;
    if (handle->run) {
        pthread_mutex_lock(&handle->mutex);
    }
    pthread_mutex_unlock(&handle->mutex_run);
    return ret;
}

/* lock mutex for connection list
 *
 * @param handle listener struct
 */
static void lock(t_rtsp_listener* handle)
{
    pthread_mutex_lock(&handle->mutex_run);
    pthread_mutex_lock(&handle->mutex);
    pthread_mutex_unlock(&handle->mutex_run);
}

/* unlock mutex for connection list
 *
 * @param handle listener struct
 */
static void unlock(t_rtsp_listener* handle)
{
    pthread_mutex_lock(&handle->mutex_run);
    pthread_mutex_unlock(&handle->mutex);
    pthread_mutex_unlock(&handle->mutex_run);
}

/* clear the run flag
 *
 * @param handle listener struct
 */
static void stop(t_rtsp_listener* handle)
{
    pthread_mutex_lock(&handle->mutex_run);
    handle->run = false;
    pthread_mutex_unlock(&handle->mutex_run);
}

int rtsp_listener_run_server(t_rtsp_server* con)
{
    int ret = rtsp_server_thread(con);

    close(con->con.fd);

    // self cleanup if parent process still running
    if (lock_run(con->owner)) {
        free(list_remove(con->idx));
        unlock(con->owner);
    }

    return ret;
}

int rtsp_listener_start_server(t_rtsp_listener* handle, int fd, struct sockaddr_in* addr)
{
    int ret = -1;
    t_rtsp_server* con = 0;

    printf("rtsp server connection from %s\n", inet_ntoa(addr->sin_addr));

    con = malloc(sizeof(t_rtsp_server));
    memset(con, 0, sizeof(t_rtsp_server));

    if (!con) return RTSP_ERRORNO;

    // server initialization
    rtsp_server_init(con, fd, addr->sin_addr.s_addr);

    if (lock_run(handle)) {
        con->idx = list_add(handle->cons, con);
        con->owner = handle;
        unlock(handle);
    }

    pthread_create(&con->th, NULL, rtsp_listener_run_server, con);

    return RTSP_SUCCESS;
}

int rtsp_listener_close_server(t_rtsp_server* con, t_rtsp_server* handle)
{
    if (shutdown(con->con.fd, SHUT_RDWR) == -1) {
        printf("close connection error: %s\n", strerror(errno));
    }
    pthread_join(con->th, 0);
    return 0;
}

/* listener thread
 *
 * @param handle listener struct
 * @return rtsp error code
 */
int rtsp_listener_thread(t_rtsp_listener* handle)
{
    struct sockaddr_in this_addr;
    struct sockaddr_in remote_addr;
    socklen_t sin_size;
    int fd;

    if ((handle->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("rtsp listener socket error: %s\n", strerror(errno));
        return RTSP_ERRORNO;
    }

    // setup own address
    memset(&this_addr, 0, sizeof(struct sockaddr_in));
    this_addr.sin_family = AF_INET;
    this_addr.sin_port = htons(handle->port);
    this_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(handle->sockfd, (struct sockaddr*)&this_addr, sizeof(struct sockaddr)) == -1) {
        printf("rtsp listener bind error: %s\n", strerror(errno));
        return RTSP_ERRORNO;
    }

    if (listen(handle->sockfd, RTSP_BACKLOG) == -1) {
        printf("rtsp listener listen error: %s\n", strerror(errno));
        return RTSP_ERRORNO;
    }

    printf("rtsp listener started\n");

    // run...
    while (handle->run) {
        sin_size = sizeof(struct sockaddr_in);
        memset(&remote_addr, 0, sizeof(struct sockaddr_in));

        if ((fd = accept(handle->sockfd, (struct sockaddr*)&remote_addr, &sin_size)) != -1) {
            rtsp_listener_start_server(handle, fd, &remote_addr);
        } else if (fd == -1) {
            printf("rtsp listener accept error: %s\n", strerror(errno));
        }
    }

    printf("rtsp listener cleanup...\n");

    // cleanup child threads...
    lock(handle);
    list_traverse_free(handle->cons, rtsp_listener_close_server, handle);
    unlock(handle);

    printf("rtsp listener closed\n");

    return RTSP_SUCCESS;
}

/* listener thread start
 *
 * @param handle listener struct
 * @param port port where to listen
 * @return rtsp error code
 */
int rtsp_listener_start(t_rtsp_listener* handle, int port)
{
    handle->port = port;
    handle->run = true;
    handle->cons = list_create();

    pthread_mutex_init(&handle->mutex, NULL);
    pthread_mutex_init(&handle->mutex_run, NULL);
    pthread_create(&handle->th, NULL, rtsp_listener_thread, handle);

    return RTSP_SUCCESS;
}

/* listener thread close
 *
 * @param handle listener struct
 * @return rtsp error code
 */
int rtsp_listener_close(t_rtsp_listener* handle)
{
    stop(handle);

    if (shutdown(handle->sockfd, SHUT_RDWR) == -1) {
        printf("close socket error: %s\n", strerror(errno));
    }

    return pthread_join(handle->th, 0);
}
