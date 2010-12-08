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

#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "bstmap.h"

#define DEV_NODE        "/dev/hdoip_core"

t_hdoipd hdoipd;

FILE* report_fd;
FILE* rtsp_fd;

typedef struct {
    int in, out;
    char *cfifo;    // command (read)
    char *rfifo;    // response (write)
} t_pipe;

ssize_t pipe_read(t_pipe* p, uint8_t *buf, size_t size)
{
    ssize_t ret = 0;
    size_t offset = 0;

    while ((offset<size)&&p->in) {
        ret = read(p->in, buf+offset, size-offset);
        if (ret == 0) {
            close(p->out);
            close(p->in);
            if (!(p->in = open(p->cfifo, O_RDONLY))) {
                perrno(" open(%s) failed: %s", p->cfifo);
                return 0;
            }
            if (!(p->out = open(p->rfifo, O_WRONLY))) {
                perrno(" open(%s) failed: %s", p->rfifo);
                return 0;
            }
        } else if (ret < 0) {
            return ret;
        } else {
            for (int x=0;x<ret;x++) printf("%02x ", buf[offset+x]);
            printf("\n");
        }
        offset = offset + ret;
    };

    return ret;
}

void* pipe_read_thread(void* arg)
{
    size_t size;
    bool run = true;
    t_pipe pipe;
    uint32_t buf[256];
    int l = strlen(arg) + 5;

    pipe.cfifo = malloc(l); snprintf(pipe.cfifo, l, "%s.cmd", (char*)arg);
    pipe.rfifo = malloc(l); snprintf(pipe.rfifo, l, "%s.rsp", (char*)arg);

    report("open <%s | %s>", pipe.cfifo, pipe.rfifo);

    if (mkfifo(pipe.cfifo, 0770) == -1) {
        perrno(" mkfifo(%s) failed", pipe.cfifo);
        return 0;
    }
    if (mkfifo(pipe.rfifo, 0770) == -1) {
        perrno(" mkfifo(%s) failed: %s", pipe.rfifo);
        return 0;
    }
    if (!(pipe.in = open(pipe.cfifo, O_RDONLY))) {
        perrno(" open(%s) failed: %s", pipe.cfifo);
        return 0;
    }
    if (!(pipe.out = open(pipe.rfifo, O_WRONLY))) {
        perrno(" open(%s) failed: %s", pipe.rfifo);
        return 0;
    }

    if (pipe.in && pipe.out) {

        do {
            // read type/size
            pipe_read(&pipe, buf, 8);
            // read rest of message
            pipe_read(&pipe, buf+2, buf[1]-8);

            lock();

            report(" < %x: %d Bytes", buf[0], buf[1]);

                if (buf[0] == 0) {
                    // close
                    run = false;
                } else {
                    hdoipd_request(buf, pipe.out);
                }

            unlock();

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

int main(int argc, char **argv)
{
    int drv;
    pthread_t* th = malloc(sizeof(pthread_t)* (argc-1));

    if (!(report_fd = fopen("/tmp/hdoipd.log", "w"))) {
        return 0;
    }

    if (!(rtsp_fd = fopen("/tmp/rtsp.log", "w"))) {
        return 0;
    }

    report("/tmp/hdoipd.log started");

    if ((drv = open(DEV_NODE, O_RDWR))) {

        if (hdoipd_init(drv)) {

            report("hdoipd started");

            for (int i=1; i<argc; i++) {
                report(" [%d] open named pipe <%s>",i,argv[i]);
                pthread_create(&th[i-1], NULL, pipe_read_thread, argv[i]);
            }

            for (int i=1; i<argc; i++) {
                pthread_join(th[i-1], NULL);
            }

            close(hdoipd.drv);

            report("hdoipd closed");

        } else {
            report("hdoipd init failed");
        }

    } else {
        printf("could not open <%s>\n", DEV_NODE);
    }

    fclose(report_fd);
    fclose(rtsp_fd);

    pthread_mutex_destroy(&hdoipd.mutex);

    return 0;
}

