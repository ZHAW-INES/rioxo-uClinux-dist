/*
 * hdoipd.c
 *
 * deamon program. opens a pipe for read command with the given names.
 * hdoipd pipe1 [pipe2] [...] [pipeN]
 * two pipes are opend: 'name.cmd'(in) and 'name.rsp'(out)
 *
 *  Created on: 03.11.2010
 *      Author: alda
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "hoi_drv_user.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "bstmap.h"

#define DEV_NODE        "/dev/hdoip_core"

t_hdoipd hdoipd;

FILE* report_fd;
FILE* rscp_fd;

typedef struct {
    int in, out;
    char *cfifo;    // command (read)
    char *rfifo;    // response (write)
} t_pipe;

int pipe_read(t_pipe* p, uint8_t *buf, size_t size)
{
    ssize_t ret = 0;
    size_t offset = 0;

    while ((offset<size)&&p->in) {
        ret = read(p->in, buf+offset, size-offset);
        if (ret == 0) {
            close(p->out);
            close(p->in);
            if ((p->in = open(p->cfifo, O_RDONLY)) == -1) {
                perrno(" open(%s) failed", p->cfifo);
                return -1;
            }
            if ((p->out = open(p->rfifo, O_WRONLY)) == -1) {
                perrno(" open(%s) failed", p->rfifo);
                return -1;
            }
        } else if (ret < 0) {
            return -1;
        }
        offset = offset + ret;
    };

    return 0;
}

void pipe_rm_existing(char* file)
{
    struct stat buffer;
    int l = strlen(file) + 17;
    char *rm_cmd;

    if(!stat(file, &buffer)) {
        rm_cmd = malloc(l); 
        snprintf(rm_cmd, l, "/bin/busybox rm %s", file);
        system(rm_cmd);
        free(rm_cmd);
    }
}

void* pipe_read_thread(void* arg)
{
    bool run = true;
    t_pipe pipe;
    uint32_t buf[256];
    int l = strlen(arg) + 5;

    pipe.cfifo = malloc(l); snprintf(pipe.cfifo, l, "%s.cmd", (char*)arg);
    pipe.rfifo = malloc(l); snprintf(pipe.rfifo, l, "%s.rsp", (char*)arg);

    // Delete existing files 
    pipe_rm_existing(pipe.cfifo);
    pipe_rm_existing(pipe.rfifo);

    // Create new pipes
    report("open <%s | %s>", pipe.cfifo, pipe.rfifo);

    if (mkfifo(pipe.cfifo, 0770) == -1) {
        perrno(" mkfifo(%s) failed", pipe.cfifo);
        return 0;
    }
    if (mkfifo(pipe.rfifo, 0770) == -1) {
        perrno(" mkfifo(%s) failed", pipe.rfifo);
        return 0;
    }
    if ((pipe.in = open(pipe.cfifo, O_RDONLY)) == -1) {
        perrno(" open(%s) failed", pipe.cfifo);
        return 0;
    }
    if ((pipe.out = open(pipe.rfifo, O_WRONLY)) == -1) {
        perrno(" open(%s) failed", pipe.rfifo);
        return 0;
    }

    if (pipe.in && pipe.out) {

        do {
            // read type/size
            if (pipe_read(&pipe, (void*)buf, 8) == -1) break;
            // read rest of message
            if (pipe_read(&pipe, (void*)(buf+2), buf[1]-8) == -1) break;

            lock("pipe_read_thread");

                if (buf[0] == 0) {
                    // close
                    run = false;
                } else {
                    hdoipd_request(buf, pipe.out);
                }

            unlock("pipe_read_thread");

        } while (run);

        report("close <%s | %s>", pipe.cfifo, pipe.rfifo);

        close(pipe.out);
        close(pipe.in);
        unlink(pipe.cfifo);
        unlink(pipe.rfifo);

    } else {
        report("could not open <%s>", arg);
    }

    return 0;
}

void* event_read_thread(void UNUSED *d)
{
    uint32_t event;
    while(1) {
        if (read(hdoipd.drv, &event, 4)==4) {
            hdoipd_event(event);
        } else {
            printf("event_read_thread failed\n");
        }
    }
}

void* poll_thread(void UNUSED *d)
{
    int ret;

    while (1) {
        struct timespec ts = {
            .tv_sec = POLL_THREAD_INTERVAL_SEC,
            .tv_nsec = POLL_THREAD_INTERVAL_NSEC
        };
        nanosleep(&ts, 0);

        lock("poll_thread()");
        {
            hoi_drv_poll();

            hdoipd_task();
        }
        unlock("poll_thread()");
    }
}

int main(int argc, char **argv)
{
    int drv;
    pthread_t the, thp;
    pthread_t* th = malloc(sizeof(pthread_t)* (argc-1));

    report_fd = stdout;
    rscp_fd = stdout;

#ifndef DBGCONSOLE
    if (!(report_fd = fopen("/tmp/hdoipd.log", "w"))) {
        return 0;
    }
#endif

#ifndef DBGCONSOLERSCP
    if (!(rscp_fd = fopen("/tmp/rscp.log", "w"))) {
        return 0;
    }
#endif

    report("/tmp/hdoipd.log started");

    if ((drv = open(DEV_NODE, O_RDWR))) {

        if (hdoipd_init(drv)) {

            pthread(the, event_read_thread, 0);
            pthread(thp, poll_thread, 0);

            for (int i=1; i<argc; i++) {
                report(" [%d] open named pipe <%s>",i,argv[i]);
                pthread(th[i-1], pipe_read_thread, argv[i]);
            }

            // start timer?


            for (int i=1; i<argc; i++) {
                pthread_join(th[i-1], NULL);
            }

            lock("main-close");
                close(hdoipd.drv);

                if(hdoipd.amx.enable) {
                	shutdown(hdoipd.amx.socket, SHUT_RDWR);
                    close(hdoipd.amx.socket);
                }

                report("hdoipd closed");
            unlock("main-close");

        } else {
            report("hdoipd init failed");
        }

    } else {
        printf("could not open <%s>\n", DEV_NODE);
    }

    fclose(report_fd);
    fclose(rscp_fd);

    pthread_mutex_destroy(&hdoipd.mutex);

    return 0;
}

