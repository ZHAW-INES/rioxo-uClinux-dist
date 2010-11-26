#ifndef ESO_HAL_H_
#define ESO_HAL_H_

#include "stdhal.h"
#include "eso_reg.h"
#include "eso_cfg.h"


/** eso hal macros
 * 
 */
#define eso_enable(p)               HOI_REG_SET(p, ESO_OFF_CFG, ESO_CFG_RUN)
#define eso_disable(p)              HOI_REG_CLR(p, ESO_OFF_CFG, ESO_CFG_RUN)
#define eso_enable_prio(p)          HOI_REG_SET(p, ESO_OFF_CFG, ESO_CFG_PRIO_EN)
#define eso_disable_prio(p)         HOI_REG_CLR(p, ESO_OFF_CFG, ESO_CFG_PRIO_EN)
#define eso_reset_stat(p)           HOI_REG_SET(p, ESO_OFF_CFG, ESO_CFG_STAT_RESET)

#define eso_get_register(p, o)      HOI_RD32(p, o)
#define eso_set_register(p, o, v)   HOI_WR32(p, o, v)

#define eso_get_idle(p)             HOI_REG_TST(p, ESO_OFF_STATUS, ESO_STATUS_IDLE)
#define eso_get_error(p)            HOI_REG_TST(p, ESO_OFF_STATUS, ESO_STATUS_ERR)
#define eso_get_size_error1(p)      HOI_REG_TST(p, ESO_OFF_STATUS, ESO_STATUS_SIZE_ERR1) // frame size = 0
#define eso_get_size_error2(p)      HOI_REG_TST(p, ESO_OFF_STATUS, ESO_STATUS_SIZE_ERR2) // frame size > MTU


/** eso hal function prototypes
 * 
 */
char* eso_get_str(void* p);
void eso_report(void* p);
bool eso_init_rbf(void* p, void* cpu_buffer, size_t cpu_size, void* vid_buffer, size_t vid_size, void* aud_buffer, size_t aud_size);
bool eso_init(void* p);
void eso_set_burst_sz(void* p, uint32_t v);
uint32_t eso_get_burst_sz(void* p);
void eso_set_fifo_th(void* p, uint32_t v);
uint32_t eso_get_fifo_th(void* p);




#endif /*ESO_HAL_H_*/
