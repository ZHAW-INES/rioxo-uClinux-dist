/** \file tmr_hal.c
 * 
 */
#include "tmr_hal.h"


/** initialize timer hw
 * 
 * @param p hardware pointer
 */
void tmr_init(void* p)
{
    tmr_set_mux(p, 0);
    tmr_set_div(p, SFREQ/AVFREQ-1);
    tmr_set_master(p, 0);

    for (int sl = 0; sl < TMR_SLAVE_CNT; sl++) {
        tmr_set_slave(p, sl, 0);
        tmr_set_inc(p, sl, 1 << TMR_INC_FRACTION_WIDTH);
    }
}
