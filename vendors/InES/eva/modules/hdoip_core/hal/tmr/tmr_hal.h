#ifndef TMR_HAL_H_
#define TMR_HAL_H_

#include "stdhal.h"
#include "tmr_reg.h"

/** tmr hal macros
 * 
 */
#define tmr_set_avmux(p, v)     HOI_REG_LD(p, TMR_OFF_CONFIG, TMR_CFG_AVMUX_MASK, v)
#define tmr_set_div(p, v)       HOI_WR32(p, TMR_OFF_DIVISION, v)
#define tmr_set_master(p, v)    HOI_WR32(p, TMR_OFF_MASTER, v)
#define tmr_set_slave(p, v)     HOI_WR32(p, TMR_OFF_SLAVE, v)
#define tmr_set_inc(p, v)       HOI_WR32(p, TMR_OFF_INCREMENT, v)

#define tmr_get_master(p)       HOI_RD32(p, TMR_OFF_MASTER)
#define tmr_get_slave(p)        HOI_RD32(p, TMR_OFF_SLAVE)
#define tmr_get_avin(p)         HOI_RD32(p, TMR_OFF_AVIN)
#define tmr_get_avout(p)        HOI_RD32(p, TMR_OFF_AVOUT)
#define tmr_get_eth(p)          HOI_RD32(p, TMR_OFF_ETH)
#define tmr_get_diff(p)         HOI_RD32(p, TMR_OFF_DIFF)
#define tmr_get_inc(p)         HOI_RD32(p, TMR_OFF_INCREMENT)


/** tmr hal function prototypes
 * 
 */
void tmr_init(void* p);

#endif /*TMR_HAL_H_*/
