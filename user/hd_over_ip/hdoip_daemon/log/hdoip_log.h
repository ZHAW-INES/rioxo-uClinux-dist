/*
 * hdoip_log.h
 *
 *  Created on: Jun 14, 2011
 *      Author: stth
 */

#ifndef HDOIP_LOG_H_
#define HDOIP_LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool                log_limit_flag;
    uint32_t            log_max_size;   // in bytes
    uint32_t            log_activ;
    uint32_t            log_cnt;
    char                log_paths[2][50];
    FILE*               log_ptrs[2];
    FILE*               log_ptr;
} t_hdoip_log;

int hdoip_log_init(t_hdoip_log *handle, char* log1, char* log2, uint32_t log_size);
int hdoip_log_close(t_hdoip_log *handle);
int hdoip_log_switch(t_hdoip_log *handle, bool clear);
int hdoip_log_handler(t_hdoip_log *handle);
int hdoip_report(t_hdoip_log *handle, const char *fmt, ...);
int hdoip_reportn(t_hdoip_log *handle, const char *fmt, ...);
int hdoip_perrno(t_hdoip_log *handle, const char *fmt, ...);

#endif /* HDOIP_LOG_H_ */
