/*
 * stream_sync.c
 *
 *  Created on: 13.10.2010
 *      Author: zent
 */

#include "stream_sync.h"

/** enables the synchronization TIMER SLAVE_0
 * 
 * @param sm pointer to the stream synchronization struct
 */
void stream_sync_start_slave_0(t_sync_means* sm) {
    sm->running = 1;

    // clear timestamp fifo
    while (sta_has_more(sm->ptr_act)){
        sta_get_delay(sm->ptr_act);
    }
}

/** enables the synchronization TIMER SLAVE_1
 * 
 * @param sm pointer to the stream synchronization struct
 */
void stream_sync_start_slave_1(t_sync_means* sm) {
    sm->running = 1;

    // clear timestamp fifo
    while (sta_has_more(sm->ptr_act)){
        sta_get_delay(sm->ptr_act);
    }
}


/** stop synchronization and reset registers TIMER SLAVE_0
 * 
 * @param sm pointer to the stream synchronization struct
 * @param size_means size of the ringbuffer for the mean difference between rx_timestamp-tx_timestamp
 */
void stream_sync_stop_slave_0(t_sync_means* sm) {
    sm->running = 0;
    sm->delay_sum = 0;
    sm->diff3_sum = 0;
    sm->means = stream_sync_am_reset(sm->means);
    sm->rises = stream_sync_am_reset(sm->rises);

    tmr_set_inc(sm->ptr_tmr, 0, (1<<TMR_INC_FRACTION_WIDTH));
}

/** stop synchronization and reset registers TIMER SLAVE_1
 * 
 * @param sm pointer to the stream synchronization struct
 * @param size_means size of the ringbuffer for the mean difference between rx_timestamp-tx_timestamp
 */
void stream_sync_stop_slave_1(t_sync_means* sm) {
    sm->running = 0;
    sm->delay_sum = 0;
    sm->diff3_sum = 0;
    sm->means = stream_sync_am_reset(sm->means);
    sm->rises = stream_sync_am_reset(sm->rises);

    tmr_set_inc(sm->ptr_tmr, 1, (1<<TMR_INC_FRACTION_WIDTH));
}

/** change synchronization source (timestamps) to audio or video
 * 
 * @param sm pointer to the stream synchronization struct
 * @param source is audio or video
 */
void stream_sync_chg_source(t_sync_means* sm, int32_t source) {
    
    //change source of audio embedded  
        stream_sync_stop_slave_0(sm);

        if (source == SYNC_SOURCE_AUDIO) {
                sm->ptr_act = sm->ptr_aso_emb;
        }
    
        if (source == SYNC_SOURCE_VIDEO) {
                sm->ptr_act = sm->ptr_vso;
        }
        stream_sync_start_slave_0(sm);
}


/** put in a value in the ringbuffer for the arithmetic mean
 * 
 * @param am pointer to the ringbuffer
 * @param v new value to put in the ringbuffer
 */
void stream_sync_am_put(t_arith_mean* am, int32_t v) {
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
int32_t stream_sync_am_get(t_arith_mean* am) {
    if (am->cnt == 0) return 0;

    return (int32_t)(am->sum / am->cnt);
}

/** start the stream synchronization for the values in the statistic fifo TIMER SLAVE_0
 * 
 * @param sm pointer to the stream synchronization struct
 */
void stream_sync_slave_0(t_sync_means* sm) {
    t_arith_mean* means = sm->means; 
    t_arith_mean* rises = sm->rises;
    void* ptr_act = sm->ptr_act;
    void* ptr_tmr = sm->ptr_tmr;
    uint32_t dead_time = sm->dead_time;
    int32_t mean_old = sm->mean_old;
    int32_t kp = sm->kp;
    int32_t ki = sm->ki;
    int64_t diff3_sum = sm->diff3_sum;
    int32_t delay = sm->delay;
    int64_t delay_sum = sm->delay_sum;

    int32_t delay_vtb_vrb = 0;
    int32_t mean_neu = sm->mean_old;
    int64_t diff1;
    int32_t diff2;
    int64_t diff3;
    int32_t inc;
    int32_t timestamp;
    int32_t tmp_delay;
    int32_t rise_offset;


    if (sm->running) {

        while (sta_has_more(ptr_act)){
            
            // mean of the values in the statistic fifo
            timestamp = sta_get_delay(ptr_act); //rx-master (ETI) - tx-master (ETO) diffX => from stream statistic
            stream_sync_am_put(means, timestamp);
            mean_neu = stream_sync_am_get(means); //mean of timestamp(ETI-ETO)
            if(stream_sync_am_get_cnt(means) < dead_time)
            {   // calculation mean delay of timer until dead-time runout -> delay for controller...
                tmp_delay = -(timestamp + tmr_get_diff(ptr_tmr, 0/*slave_nr*/) /*rx-slave0 - rx-master => Timerdifferenz*/);
                delay_sum += tmp_delay;
                delay = (int32_t)(delay_sum/(stream_sync_am_get_cnt(means)-1));

                // delay = mean of delay between timestamp-diff and timer-diff (at startup)
            }

            // mean of the rises
            if(stream_sync_am_get_cnt(means) > 1)
                stream_sync_am_put(rises, (mean_neu - mean_old)<<20);
                //(diffX-diffX)*1048576 (2²⁰) -> mean of mean of timestamp(ETI_ETO) * 1048576
            mean_old = mean_neu;
        }

        // PI controller
        if(stream_sync_am_get_cnt(rises) > dead_time){
            //Diff1 = rx_timestamp - tx_timestamp
            rise_offset = (( ((int64_t)stream_sync_am_get_cnt(means)) * ((int64_t)stream_sync_am_get(rises)) )>>21);
                                //{cnt.mean(ETI-ETO)  *  mea(rises)}/2097152
            diff1 = delay + mean_neu + rise_offset;

            //Diff2 = timer_slave - timer_master
            diff2 = tmr_get_diff(ptr_tmr, 0/*slave_nr*/);

            //Diff3 = Diff1 + Diff2
            diff3 = ((int64_t)diff2) + (diff1);

            diff3_sum = diff3_sum + diff3;

            if (diff3_sum > 1000)
                diff3_sum = 1000;

            if (diff3_sum < -1000)
                diff3_sum = -1000;
        

            inc = (1<<TMR_INC_FRACTION_WIDTH) - ((kp * diff3 + ki * diff3_sum)>>10);

            if (inc > sm->max_inc) inc = sm->max_inc;
            if (inc < sm->min_inc) inc = sm->min_inc;
  
            tmr_set_inc(ptr_tmr, 0/*slave_nr*/, inc);                                  // write increment to Slavetimer

            delay_vtb_vrb = mean_neu + rise_offset + tmr_get_diff(ptr_tmr, 0/*slave_nr*/); //-delay + diff3;

            // debug for matlab //

            //printk("\n ,%i,", mean_neu + rise_offset + tmr_get_diff(ptr_tmr));          // delay
            //printk("%i,", (int)diff3);                                                  // Regelabweichung
            //printk("%i;", inc-(1<<TMR_INC_FRACTION_WIDTH));                             //
            // ---------------- //

        }
    }

    sm->delay           = delay;
    sm->delay_vtb_vrb   = delay_vtb_vrb;
    sm->delay_sum       = delay_sum;
    sm->mean_old        = mean_old;
    sm->diff3_sum       = diff3_sum;

}


/** start the stream synchronization for the values in the statistic fifo TIMER SLAVE_1
 * 
 * @param sm pointer to the stream synchronization struct
 */
void stream_sync_slave_1(t_sync_means* sm) {
    t_arith_mean* means = sm->means; 
    t_arith_mean* rises = sm->rises;
    void* ptr_act = sm->ptr_act;
    void* ptr_tmr = sm->ptr_tmr;
    uint32_t dead_time = sm->dead_time;
    int32_t mean_old = sm->mean_old;
    int32_t kp = sm->kp;
    int32_t ki = sm->ki;
    int64_t diff3_sum = sm->diff3_sum;
    int32_t delay = sm->delay;
    int64_t delay_sum = sm->delay_sum;

    int32_t delay_vtb_vrb = 0;
    int32_t mean_neu = sm->mean_old;
    int64_t diff1;
    int32_t diff2;
    int64_t diff3;
    int32_t inc;
    int32_t timestamp;
    int32_t tmp_delay;
    int32_t rise_offset;

    //static int count = 100;       //just for testoutput "Regelabweichung"

    if (sm->running) {
        while (sta_has_more(ptr_act)){

            // mean of the values in the statistic fifo
            timestamp = sta_get_delay(ptr_act); //rx-master (ETI) - tx-master (ETO) :diffX => from stream statistic

            stream_sync_am_put(means, timestamp);
            mean_neu = stream_sync_am_get(means); //mean of timestamp(ETI-ETO)
            if(stream_sync_am_get_cnt(means) < dead_time)
            {   // berechnet mittelwertsdelay der Timer bis dead_time abgelaufen -> delay für Regelwert...
                tmp_delay = -(timestamp + tmr_get_diff(ptr_tmr, 1/*slave_nr*/) /*-(rx-master - tx-master)+(rx-slave1 - rx-master) => Timerdifferenz*/);
                delay_sum += tmp_delay;
                delay = (int32_t)(delay_sum/(stream_sync_am_get_cnt(means)-1));

                // delay = mean of delay master_tx<->slave1_rx

            }
            // mean of the rises
            if(stream_sync_am_get_cnt(means) > 1)
                stream_sync_am_put(rises, (mean_neu - mean_old)<<20);
                //(diffX-diffX)*1048576 (2²⁰)

            mean_old = mean_neu;
        }

        // PI controller
        if(stream_sync_am_get_cnt(rises) > dead_time){
            //Diff1 = rx_timestamp - tx_timestamp
            rise_offset = (( ((int64_t)stream_sync_am_get_cnt(means)) * ((int64_t)stream_sync_am_get(rises)) )>>21);
                                //{Anz.Mittelwerte(ETI-ETO)  *  mittelwert(rises)}/2097152
            diff1 = delay + mean_neu + rise_offset;

            //Diff2 = timer_slave - timer_master
            diff2 = tmr_get_diff(ptr_tmr, 1/*slave_nr*/);

            //Diff3 = Diff1 + Diff2
            diff3 = ((int64_t)diff2) + (diff1);

            diff3_sum = diff3_sum + diff3;

            if (diff3_sum > 1000)
                diff3_sum = 1000;

            if (diff3_sum < -1000)
                diff3_sum = -1000;
        

            inc = (1<<TMR_INC_FRACTION_WIDTH) - ((kp * diff3 + ki * diff3_sum)>>10);

            if (inc > sm->max_inc) inc = sm->max_inc;
            if (inc < sm->min_inc) inc = sm->min_inc;
  
       //     tmr_set_inc(ptr_tmr, 1/*slave_nr*/, inc);                           // write increment to Slavetimer

            delay_vtb_vrb = mean_neu + rise_offset + tmr_get_diff(ptr_tmr, 1/*slave_nr*/); //-delay + diff3;

            // debug for matlab //

            //printk("\n ,%i,", mean_neu + rise_offset + tmr_get_diff(ptr_tmr, 1));        // delay
            /*if(count>20){
                printk("Regelabweichung Slave_1: %i,\n", (int)diff3);                        // Regelabweichung
                count = 0;
            }
            else count++;*/
            //printk("%i;", inc-(1<<TMR_INC_FRACTION_WIDTH));
            // ---------------- //

        }
    }

    sm->delay           = delay;
    sm->delay_vtb_vrb   = delay_vtb_vrb;
    sm->delay_sum       = delay_sum;
    sm->mean_old        = mean_old;
    sm->diff3_sum       = diff3_sum;

}


/** Get the delay between vtb and vrb
 * 
 * @param pointer
 * @return delay value
 */
uint32_t stream_sync_get_delay(t_sync_means* sm) {
    return sm->delay_vtb_vrb;
}


/** initialize a ringbuffer for the arithmetic mean
 * 
 * @param size size off the ringbuffer
 * @return am ringbuffer with the size 'size'sm->means = stream_sync_am_init(size_means);
 */
t_arith_mean* stream_sync_am_init(uint32_t size) {
    t_arith_mean *am;

    size++;
    am = kzalloc(sizeof(t_arith_mean) + size * sizeof(int32_t), GFP_KERNEL);

    if (!am) return 0;

    for (int i=0;i<size;i++) am->buf[i] = 0;

    am->sum = 0;
    am->cnt = 0;
    am->read = 0;
    am->write = 0;
    am->size = size;

    return am;
}

/** reset ringbuffer for the arithmetic mean
 * 
 * @param am pointer to the ringbuffer
 * @return am ringbuffer
 */
t_arith_mean* stream_sync_am_reset(t_arith_mean* am) {

    am->sum = 0;
    am->cnt = 0;
    am->read = 0;
    am->write = 0;

    return am;
}


/** initialize stream synchronization
 * 
 * @param sm pointer to the stream synchronization struct
 * @param size_means size of the ringbuffer for the mean difference between rx_timestamp-tx_timestamp
 * @param size_rises size of the ringbuffer for the mean rises between the mean differences
 * @param ptr_stat pointer to the statistic registers
 * @param ptr_tmr pointer to the timer
 * @param dead_time dead-time of the stream synchronization, PI controller starts after 'dead_time' samples
 * @param kp proportional gain for the PI controller, f uint32_t size_means,ormat: <32-TMR_INC_FRACTION_WIDTH>.<TMR_INC_FRACTION_WIDTH>
 * @param ki integral gain for the PI controller, format: <32-TMR_INC_FRACTION_WIDTH>.<TMR_INC_FRACTION_WIDTH>
 * @param inc_ppm tolerance for the increment value (1 +/- inc_ppm), format: <32-TMR_INC_FRACTION_WIDTH>.<TMR_INC_FRACTION_WIDTH>
 */
void stream_sync_init(t_sync_means* sm, uint32_t size_means, uint32_t size_rises, void* ptr_stat, void* ptr_tmr, uint32_t stream, uint32_t dead_time, uint32_t kp, uint32_t ki, uint32_t inc_ppm) {
    sm->means = stream_sync_am_init(size_means);
    sm->rises = stream_sync_am_init(size_rises);
    sm->ptr_vso       = ptr_stat + STATISTIC_REG_OFFSET_VID;
    sm->ptr_aso_emb   = ptr_stat + STATISTIC_REG_OFFSET_AUD_EMB;
    sm->ptr_aso_board = ptr_stat + STATISTIC_REG_OFFSET_AUD_BOARD;

    if(stream == AUD_STREAM_NR_EMBEDDED){
        sm->ptr_act = ptr_stat + STATISTIC_REG_OFFSET_VID;;           //for embedded Stream VID default
    }
    if(stream == AUD_STREAM_NR_IF_BOARD){
        sm->ptr_act = ptr_stat + STATISTIC_REG_OFFSET_AUD_BOARD;;           //for if_board Stream AUD fix
    }

    sm->ptr_tmr = ptr_tmr;
 // tmr_init(ptr_tmr);
    sm->dead_time = dead_time;
    sm->mean_old = 0;
    sm->kp = kp;
    sm->ki = ki;
    sm->diff3_sum = 0;
    sm->max_inc = (1<<TMR_INC_FRACTION_WIDTH) + inc_ppm;
    sm->min_inc = (1<<TMR_INC_FRACTION_WIDTH) - inc_ppm;
    sm->delay_sum = 0;
    sm->running = 0;
    sm->size_means = size_means;

    printk("\nStream-synchronization initialized\n");
}

