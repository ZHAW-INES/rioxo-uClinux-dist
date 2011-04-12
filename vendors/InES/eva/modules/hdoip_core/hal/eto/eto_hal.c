
#include "eto_hal.h"

void eto_prio_init(void* p, uint8_t shift, uint32_t clock_div)
{
    uint32_t tmp = shift;
    tmp = (clock_div & ETO_PRIO_CFG_CLOCK_DIV_MASK) | ((tmp << 28) & ETO_PRIO_CFG_SHIFT_MASK);

    HOI_WR32((p), ETO_PRIORITY_CONFIG_REG, tmp);
}
