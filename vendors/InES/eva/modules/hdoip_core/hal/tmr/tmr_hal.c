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
    HOI_WR32(p, TMR_OFF_DIVISION, 0);
    tmr_set_inc(p, 1.0);
}