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
    HOI_WR32(p, TMR_OFF_CONFIG, 0);
    HOI_WR32(p, TMR_OFF_DIVISION, SFREQ/AVFREQ-1);
    tmr_set_inc(p, 1 << TMR_INC_FRACTION_WIDTH);
    tmr_set_master(p, 0);
    tmr_set_slave(p, 0);
}
