/*
 * hdoip_log.c
 *
 *  Created on: Jun 14, 2011
 *      Author: stth
 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hdoip_log.h"
#include "hdoipd.h"

static void hdoip_log_header(t_hdoip_log *handle)
{
    hdoip_report(handle, "*--------------------------------*");
    hdoip_report(handle, "| Log #%d", handle->log_cnt++);
    hdoip_report(handle, "*--------------------------------*");
}

int hdoip_log_init(t_hdoip_log *handle, char* log1, char* log2, uint32_t log_size)
{
    FILE* fd;

    handle->log_cnt = 0;

    if(log_size > 0) {
        handle->log_max_size = log_size;
        handle->log_limit_flag = true;
    } else {
        handle->log_max_size = 0;
        handle->log_limit_flag = false;
    }

    if(strcmp(log1, "") == 0) {
        handle->log_ptrs[0] = stdout;
    } else {
        fd = fopen(log1, "w+");
        if(fd == NULL) {
            return -1;
        } else {
            handle->log_ptrs[0] = fd;
        }
    }
    strcpy(handle->log_paths[0], log1);

    if(strcmp(log2, "") == 0) {
        handle->log_ptrs[1] = stdout;

    } else {
        fd = fopen(log2, "w+");
        if(fd == NULL) {
            return -1;
        } else {
            handle->log_ptrs[1] = fd;
        }
    }
    strcpy(handle->log_paths[1], log2);


    handle->log_activ = 0;
    handle->log_ptr = handle->log_ptrs[handle->log_activ];

    if(handle->log_ptr != stdout) {
        hdoip_log_header(handle);
    }

    report("logging %s started", handle->log_paths[0]);
    report("logging %s started", handle->log_paths[1]);

    return 0;
}

int hdoip_log_close(t_hdoip_log *handle)
{
    fclose(handle->log_ptrs[0]);
    fclose(handle->log_ptrs[1]);

    return 0;
}

int hdoip_log_switch(t_hdoip_log *handle, bool clear)
{
    bool clear_flag = ((clear == true) && (handle->log_ptrs[handle->log_activ] != stdout));
    if(handle->log_activ > 0) {
        handle->log_activ = 0;
    } else {
        handle->log_activ = 1;
    }

    if(clear_flag){
        fclose(handle->log_ptrs[handle->log_activ]);
        handle->log_ptrs[handle->log_activ] = fopen(handle->log_paths[handle->log_activ], "w+");
        if(handle->log_ptrs[handle->log_activ] == NULL) {
            report("hdoip_log_switch() failed");
            return -1;
        }
    }

    handle->log_ptr = handle->log_ptrs[handle->log_activ];

    if(clear_flag) {
        hdoip_log_header(handle);
    }

    return 0;
}

int hdoip_log_handler(t_hdoip_log *handle)
{
    struct stat buf;

    if(handle->log_limit_flag == true){
        if(handle->log_ptr != stdout) {
            if(fstat(fileno(handle->log_ptr), &buf)) { // get file size
                return -1;
            }

            if(buf.st_size > handle->log_max_size) {
                hdoip_log_switch(handle, true);
            }
        } else {
            handle->log_limit_flag = false;
        }
    }

    return 0;
}

int hdoip_report(t_hdoip_log *handle, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    vfprintf(handle->log_ptr, fmt, ap);
    fprintf(handle->log_ptr, "\n");
    fflush(handle->log_ptr);

    va_end(ap);

    return 0;
}

int hdoip_reportn(t_hdoip_log *handle, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    vfprintf(handle->log_ptr, fmt, ap);

    va_end(ap);

    return 0;
}

int hdoip_perrno(t_hdoip_log *handle, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    fprintf(handle->log_ptr, "[error] %s (%d): %s\n", __FILE__, __LINE__, strerror(errno));
    vfprintf(handle->log_ptr, fmt, ap);
    fprintf(handle->log_ptr, "\n");
    fflush(handle->log_ptr);

    va_end(ap);

    return 0;
}
