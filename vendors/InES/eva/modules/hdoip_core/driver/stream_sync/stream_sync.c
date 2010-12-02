/*
 * stream_sync.c
 *
 *  Created on: 13.10.2010
 *      Author: zent
 */

#include "stream_sync.h"

/** set timer difference slave-master
 * 
 * @param ptr_tmr pointer to the timer
 * @param diff_rx_tx difference rx_timestamp - tx_timestamp
 */
void stream_sync_set_diff(t_sync_means* sm, int32_t diff_rx_tx)
{
    tmr_set_slave(sm->ptr_tmr, (tmr_get_master(sm->ptr_tmr) + diff_rx_tx) );
}

/** put in a value in the ringbuffer for the arithmetic mean
 * 
 * @param am pointer to the ringbuffer
 * @param v new value to put in the ringbuffer
 */
void stream_sync_am_put(t_arith_mean* am, int32_t v)
{
    uint32_t write = am->write;
    uint32_t read = am->read;
    uint32_t cnt = am->cnt;
    int64_t sum = am->sum;

    sum = sum + (int64_t)v;
    cnt = cnt + 1;
    am->buf[write] = v;

    write++;
    if (write >= am->size) write = 0;

    if (write == read) {
        sum = sum - (int64_t)am->buf[read];
        cnt = cnt - 1;

        read++;
        if (read >= am->size) read = 0;
    }

    am->write = write;
    am->read = read;
    am->sum = sum;
    am->cnt = cnt;
}

/** get the arithmetic mean of the values in the ringbuffer 
 * 
 * @param am pointer to the ringbuffer
 * @return arithmetic mean of the values in the ringbuffer
 */
int32_t stream_sync_am_get(t_arith_mean* am)
{
    if (am->cnt == 0) return 0;

    return (int32_t)(am->sum / am->cnt);
}

/** start the stream synchronization for the values in the statistic fifo
 * 
 * @param sm pointer to the stream synchronization struct
 */
void stream_sync(t_sync_means* sm)
{
    t_arith_mean* means = sm->means; 
    t_arith_mean* rises = sm->rises; 
    void* ptr_act = sm->ptr_act;
    void* ptr_tmr = sm->ptr_tmr;
    uint32_t dead_time = sm->dead_time;
    int32_t mean_old = sm->mean_old;
    int32_t kp = sm->kp;
    int32_t ki = sm->ki;
    int64_t diff3_sum = sm->diff3_sum;
    
    int32_t mean_neu;
    int64_t diff1;
    int32_t diff2;
    int64_t diff3;
    int64_t inc;

    #ifdef STREAM_SYNC_DEBUG
        t_arith_mean* buf_diff1 = sm->buf_diff1;
        t_arith_mean* buf_diff2 = sm->buf_diff2;
        t_arith_mean* buf_diff3 = sm->buf_diff3;
        t_arith_mean* buf_inc = sm->buf_inc;
    #endif
   
 
    // mean of the values in the statistic fifo
    while (sta_has_more(ptr_act)){   
        stream_sync_am_put(means, sta_get_delay(ptr_act));
        mean_neu = stream_sync_am_get(means);
        if(stream_sync_am_get_cnt(means) > 1) stream_sync_am_put(rises, (mean_neu - mean_old));
        mean_old = mean_neu;
    }
    
    // PI controller
    if(stream_sync_am_get_cnt(rises) > dead_time){
        //Diff1 = rx_timestamp - tx_timestamp
        diff1 = (int64_t)mean_neu + (( ((int64_t)(stream_sync_am_get_cnt(means) - 1)) * ((int64_t)stream_sync_am_get(rises)) )>>1);
        
        //Diff2 = timer_slave - timer_master
        diff2 = tmr_get_diff(ptr_tmr);
        
        //Diff3 = Diff1 + Diff2
        diff3 = (diff1) + ((int64_t)diff2);
        
        diff3_sum = diff3_sum + diff3;
        
        inc = (1<<TMR_INC_FRACTION_WIDTH) - kp * diff3 - ki * diff3_sum;
        
        if (inc > sm->max_inc) inc = sm->max_inc;
        if (inc < sm->min_inc) inc = sm->min_inc;
        
        tmr_set_inc(ptr_tmr, inc); // write increment to Slavetimer

        #ifdef STREAM_SYNC_DEBUG
            stream_sync_am_put(buf_diff1, diff1);
            stream_sync_am_put(buf_diff2, diff2);
            stream_sync_am_put(buf_diff3, diff3);
            stream_sync_am_put(buf_inc, inc);
        #endif
    }
    sm->mean_old = mean_old;
    sm->diff3_sum = diff3_sum;
}

/** initialize a ringbuffer for the arithmetic mean
 * 
 * @param size size off the ringbuffer
 * @return am ringbuffer with the size 'size'
 */
t_arith_mean* stream_sync_am_init(uint32_t size)
{
    size++;
    t_arith_mean* am = kzalloc(sizeof(t_arith_mean) + size * sizeof(int32_t), GFP_KERNEL);

    if (!am) return 0;

    for (int i=0;i<size;i++) am->buf[i] = 0;

    am->sum = 0;
    am->cnt = 0;
    am->read = 0;
    am->write = 0;
    am->size = size;

    return am;
}

/** initialize stream synchronization
 * 
 * @param sm pointer to the stream synchronization struct
 * @param size_means size of the ringbuffer for the mean difference between rx_timestamp-tx_timestamp
 * @param size_rises size of the ringbuffer for the mean rises between the mean differences
 * @param ptr_act pointer to the actual used syncstream of audio(ASO) or video(VSO)
 * @param ptr_aso pointer to the audio(ASO)
 * @param ptr_vso pointer to the video(VSO)
 * @param ptr_tmr pointer to the timer
 * @param dead_time dead-time of the stream synchronization, PI controller starts after 'dead_time' samples
 * @param kp proportional gain for the PI controller, format: <32-TMR_INC_FRACTION_WIDTH>.<TMR_INC_FRACTION_WIDTH>
 * @param ki integral gain for the PI controller, format: <32-TMR_INC_FRACTION_WIDTH>.<TMR_INC_FRACTION_WIDTH>
 * @param inc_ppm tolerance for the increment value (1 +/- inc_ppm), format: <32-TMR_INC_FRACTION_WIDTH>.<TMR_INC_FRACTION_WIDTH>
 */
void stream_sync_init(t_sync_means* sm, uint32_t size_means, uint32_t size_rises, void*  ptr_act, void*  ptr_aso, void*  ptr_vso, void* ptr_tmr, uint32_t dead_time, uint32_t kp, uint32_t ki, uint32_t inc_ppm)
{
    sm->means = stream_sync_am_init(size_means);
    //&(sm->means) = stream_sync_am_init(size_means);
    sm->rises = stream_sync_am_init(size_rises);
    //&(sm->rises) = stream_sync_am_init(size_rises);
    sm->ptr_act = ptr_act;
    sm->ptr_aso = ptr_aso;
    sm->ptr_vso = ptr_vso;
    sm->ptr_tmr = ptr_tmr;
    tmr_init(ptr_tmr);
    sm->dead_time = dead_time; 
    sm->mean_old = 0;
    sm->kp = kp;
    sm->ki = ki;
    sm->diff3_sum = 0;
    sm->max_inc = (1<<TMR_INC_FRACTION_WIDTH) + inc_ppm;
    sm->min_inc = (1<<TMR_INC_FRACTION_WIDTH) - inc_ppm;
    #ifdef STREAM_SYNC_DEBUG
        sm->buf_diff1 = stream_sync_am_init(size_means);
        //&(sm->buf_diff1) = stream_sync_am_init(size_means);
        sm->buf_diff2 = stream_sync_am_init(size_means);
        //&(sm->buf_diff2) = stream_sync_am_init(size_means);
        sm->buf_diff3 = stream_sync_am_init(size_means);
        //&(sm->buf_diff3) = stream_sync_am_init(size_means);
        sm->buf_inc = stream_sync_am_init(size_means);
        //&(sm->buf_inc) = stream_sync_am_init(size_means);
        printk("\n<stream_sync_init>\ndead_time = %u\nsm->dead_time = %u\n",dead_time, sm->dead_time);
    #endif
}
