/*
 * stream_sync.h
 *
 *  Created on: 13.10.2010
 *      Author: zent
 */

#ifndef STREAM_SYNC_H_
#define STREAM_SYNC_H_
#undef STREAM_SYNC_DEBUG

#include "std.h"
#include "sta_hal.h"
#include "tmr_hal.h"
#include "stream_sync_cfg.h"

/** struct for a ringbuffer for arithmetical mean
 */
typedef struct {
    uint32_t    cnt;
    int64_t     sum;
    uint32_t    write;
    uint32_t    read;
    uint32_t    size;
    int32_t     buf[];
} t_arith_mean;

/** struct with the stream-synchronisation information
 */
typedef struct {
    t_arith_mean* means;
    //t_arith_mean means;
    t_arith_mean* rises;
    //t_arith_mean rises;
    void* ptr_act;
    void* ptr_aso;
    void* ptr_vso;
    void* ptr_tmr;
    uint32_t dead_time;
    int32_t mean_old;
    int32_t kp;
    int32_t ki;
    int64_t diff3_sum;
    int32_t inc_ppm;
    uint32_t max_inc;
    uint32_t min_inc;
    int32_t delay;
    int32_t delay_vtb_vrb;
    int64_t delay_sum;
    int32_t running;
    int32_t size_means;


    #ifdef STREAM_SYNC_DEBUG
        t_arith_mean* buf_diff1;
        //t_arith_mean buf_diff1;
        t_arith_mean* buf_diff2;
        //t_arith_mean buf_diff2;
        t_arith_mean* buf_diff3;
        //t_arith_mean buf_diff3;
        t_arith_mean* buf_inc;
        //t_arith_mean buf_inc;
    #endif
} t_sync_means;

/** struct with sync-debug-data
 */
#ifdef STREAM_SYNC_DEBUG
    typedef struct {
        int32_t diff1[11];
        int32_t diff2[11];
        int32_t diff3[11];
        int32_t inc[11];
    } t_sync_debug;
#endif

/** stream_sync function prototypes
 * 
 */
static inline uint32_t stream_sync_am_get_cnt(t_arith_mean* am)
{
    return am->cnt;
}

void stream_sync_init(t_sync_means* sm, uint32_t size_means, uint32_t size_rises, void*  ptr_aso, void*  ptr_vso, void* ptr_tmr, uint32_t dead_time, uint32_t kp, uint32_t ki, uint32_t inc_ppm);
void stream_sync(t_sync_means* sm);
t_arith_mean* stream_sync_am_init(uint32_t size);
void stream_sync_am_put(t_arith_mean* am, int32_t v);
int32_t stream_sync_am_get(t_arith_mean* am);
void stream_sync_start(t_sync_means* sm);
void stream_sync_stop(t_sync_means* sm);
void stream_sync_chg_source(t_sync_means* sm, int32_t source);
t_arith_mean* stream_sync_am_reset(t_arith_mean* am);
uint32_t stream_sync_get_delay(t_sync_means* sm);

#endif /* STREAM_SYNC_H_ */
