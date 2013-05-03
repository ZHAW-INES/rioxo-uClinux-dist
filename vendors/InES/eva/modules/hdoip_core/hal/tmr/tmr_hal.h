#ifndef TMR_HAL_H_
#define TMR_HAL_H_

#include "stdhal.h"
#include "tmr_reg.h"

/** tmr hal macros
 * 
 */
#define tmr_set_mux(p, v)               HOI_WR32(p, TMR_OFF_CONFIG, v)
#define tmr_set_div(p, v)               HOI_WR32(p, TMR_OFF_DIVISION, v)
#define tmr_set_master(p, v)            HOI_WR32(p, TMR_OFF_MASTER, v)
#define tmr_set_slave(p, slave_nr, v)   HOI_WR32(p, tmr_reg2off(slave_nr, TMR_REG_SLAVE), v)
#define tmr_set_inc(p, slave_nr, v)     HOI_WR32(p, tmr_reg2off(slave_nr, TMR_REG_INCREMENT), v)

#define tmr_get_master(p)               HOI_RD32(p, TMR_OFF_MASTER)
#define tmr_get_slave(p, slave_nr)      HOI_RD32(p, tmr_reg2off(slave_nr, TMR_REG_SLAVE))
#define tmr_get_diff(p, slave_nr)       HOI_RD32(p, tmr_reg2off(slave_nr, TMR_REG_DIFF))
#define tmr_get_inc(p, slave_nr)        HOI_RD32(p, tmr_reg2off(slave_nr, TMR_REG_INCREMENT))

#define TMR_SLAVE_CNT                   2

/** tmr hal function prototypes
 * 
 */
void tmr_init(void* p);

#endif /*TMR_HAL_H_*/
