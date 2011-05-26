#ifndef WDG_HAL_H_
#define WDG_HAL_H_

#include "stdhal.h"
#include "wdg_reg.h"


#define wdg_set_cfg(p, m)       HOI_REG_SET((p), WDG_OFF_CONFIG_REG, (m))
#define wdg_clr_cfg(p, m)       HOI_REG_CLR((p), WDG_OFF_CONFIG_REG, (m))
#define wdg_get_cfg(p, m)       HOI_REG_RD((p), WDG_OFF_CONFIG_REG, (m))

#define wdg_enable(p)           wdg_set_cfg((p), WDG_BIT_CFG_EN)
#define wdg_disable(p)          wdg_clr_cfg((p), WDG_BIT_CFG_EN)
#define wdg_reset(p)            wdg_set_cfg((p), WDG_BIT_CFG_RESET)

#define wdg_set_time(p, v)      HOI_WR32((p), WDG_OFF_SERVICE_TIME, (v))
#define wdg_get_time(p)         HOI_RD32((p), WDG_OFF_SERVICE_TIME)


#endif /* WDG_HAL_H_ */
