//! \file ver_hal.h
#ifndef VER_HAL_H_
#define VER_HAL_H_

#include "std.h"
#include "ver_reg.h"

#define ver_get_fpga_date(p)            HOI_RD32((p), VER_OFF_FPGA_DATE)
#define ver_get_fpga_svn(p)             HOI_RD32((p), VER_OFF_FPGA_SVN)

#define ver_get_sopc_date(p)            HOI_RD32((p), VER_OFF_SOPC_DATE)
#define ver_get_sopc_id(p)              HOI_RD32((p), VER_OFF_SOPC_ID)

#endif /* VER_HAL_H_ */
