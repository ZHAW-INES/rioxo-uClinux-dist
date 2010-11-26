/*
 * hdoipd.c
 *
 * deamon program. opens a pipe for read command with the given names.
 * hdoipd pipe1 [pipe2] [...] [pipeN]
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

#define DEV_NODE        "/dev/hdoip_core"

t_hdoipd handle;

ssize_t pipe_read(int* fd, char* arg, uint8_t *buf, size_t size)
{
    ssize_t ret;
    size_t offset = 0;
    do {
        ret = read(*fd, buf+offset, size-offset);
        if (ret == 0) {
            close(*fd);
            *fd = open(arg, O_RDONLY);
            printf("reopen <%s>\n", arg);
        } else if (ret < 0) {
            return ret;
        }
        offset = offset + ret;
    } while ((offset<size)&&*fd);
    return ret;
}

void* pipe_read_thread(void* arg)
{
    size_t size;
    bool run = true;
    int fd;
    int rsp = 0;
    uint32_t buf[256];

    mkfifo(arg, 0770);
    fd = open(arg, O_RDONLY);

    if (fd) {
        printf("open <%s>\n", arg);
        do {
            // read type/size
            pipe_read(&fd, arg, buf, 8);
            // read rest of message
            pipe_read(&fd, arg, buf+2, buf[1]-8);

            pthread_mutex_lock(&handle.mutex);
printf("cmd: %08x|%d Byte\n", buf[0], buf[1]);

                if (buf[0] == 0) {
                    // close
                    run = false;
                } else {
                    hdoipd_request(&handle, buf, rsp);
                }

            pthread_mutex_unlock(&handle.mutex);
        } while (run);

        printf("close <%s>\n", arg);

        close(fd);
        unlink(arg);

    } else {
        printf("could not open <%s>\n", arg);
    }

}

int main(int argc, char **argv)
{
    pthread_t* th = malloc(sizeof(pthread_t)* (argc-1));

    pthread_mutex_init(&handle.mutex, NULL);

    if ((handle.drv = open(DEV_NODE, O_RDWR))) {

        hdoipd_init(&handle);
        printf("hdoipd started\n");

        for (int i=1; i<argc; i++) {
            printf(" [%d] open named pipe <%s>\n",i,argv[i]);
            pthread_create(&th[i-1], NULL, pipe_read_thread, argv[i]);
        }

        for (int i=1; i<argc; i++) {
            pthread_join(th[i-1], NULL);
        }

        close(handle.drv);

        printf("hdoipd closed\n");

    } else {
        printf("could not open <%s>\n", DEV_NODE);
    }

    pthread_mutex_destroy(&handle.mutex);

    return 0;
}

