#ifndef VIO_DRV_PLL_H_
#define VIO_DRV_PLL_H_

#include "vio_drv.h"
#include "si598.h"

int vio_drv_pll_setup(void* p, t_vio_pll* pll, t_si598* si598, int in, int out, int rel, int mode, uint32_t device, int advcnt);

#endif /*VIO_DRV_PLL_H_*/
