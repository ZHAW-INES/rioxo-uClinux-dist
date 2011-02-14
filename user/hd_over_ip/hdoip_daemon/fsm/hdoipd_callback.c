/*
 * hdoipd_callback.c
 *
 *  Created on: 20.01.2011
 *      Author: alda
 */

#include "hdoipd_callback.h"


typedef struct {
    int (*f)(void* d);
    void* d;
    struct timespec ts;
    int repeat;
    struct timespec rts;
} t_delayed_task;


bool strscmp(char **p, char *s)
{
    int l2 = strlen(s);

    while ((**p) == ' ') (*p)++;

    int n = strncmp(*p, s, l2);

    if (n == 0) {
        *p += l2;
        return 1;
    }

    return 0;
}


/** Callback each required media
 *
 * @param media 0: choose from registry, 1: active, 2: all, ptr*: one specific
 */
int hdoipd_media_callback(t_rscp_media* media, int (*f)(t_rscp_media*, void*), void* d)
{
    char *s;
    int ret = 0;
    if (media == (void*)0) {
        // Choose media based on registry
        s = reg_get("mode-media");
        while (s && *s) {
                 if (strscmp(&s, "video")) ret |= f(&vrb_video, d);
            else if (strscmp(&s, "audio")) ret |= f(&vrb_audio, d);
            else { report("unsupported media: %s", s); break; }
        }
    } else if (media == (void*)1) {
        // Choose all active media
        if (rscp_media_active(&vrb_video)) ret |= f(&vrb_video, d);
        if (rscp_media_active(&vrb_audio)) ret |= f(&vrb_audio, d);
    } else if (media == (void*)2) {
        // Choose all media
        ret |= f(&vrb_video, d);
        ret |= f(&vrb_audio, d);
    } else {
        // Choose media specified by *media
        ret |= f(media, d);
    }
    return ret;
}


void* hdoipd_delayed_thread(void* d)
{
    t_delayed_task* task = (t_delayed_task*)d;
    int ret = -1;

    nanosleep(&task->ts, 0);
    do {

        lock("hdoipd_delayed_thread");
        ret = task->f(task->d);
        unlock("hdoipd_delayed_thread");

        if (task->repeat && ret) {
            nanosleep(&task->rts, 0);
            task->repeat--;
        }
    } while (task->repeat && ret);

    free(task);

    return 0;
}

/** function launcher
 *
 * this calls a function with a predefined delay in a new thread.
 * if the called function doe not return SUCCESS (0) it is repeated.
 *
 * !!! No lock may be active when calling this function !!!
 */
void hdoipd_launch(int (*f)(void* d), void* d, int timeout_ms, int repeat, int intervall_ms)
{
#ifdef USE_LAUNCHER
    pthread_t th;
    t_delayed_task* task = malloc(sizeof(t_delayed_task));

    if (task) {
        memset(task, 0, sizeof(t_delayed_task));
        task->f = f;
        task->d = d;
        task->ts.tv_sec = timeout_ms / 1000;
        task->ts.tv_nsec = (unsigned long)(timeout_ms % 1000) * (unsigned long)1000000;
        task->repeat = repeat;
        task->rts.tv_sec = intervall_ms / 1000;
        task->rts.tv_nsec = (unsigned long)(intervall_ms % 1000) * (unsigned long)1000000;
        pthread(th, hdoipd_delayed_thread, task);
    } else {
        report("malloc(sizeof(t_delayed_task)) failed");
    }
#else
    report("launch not active");
#endif
}

