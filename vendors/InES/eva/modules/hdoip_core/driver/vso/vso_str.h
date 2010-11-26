#ifndef VSO_STR_H_
#define VSO_STR_H_

#include "std.h"
#include "vso_hal.h"
#include "vso_drv_cfg.h"

#define vso_report_dsc(p)           hoi_report_rbf(p, VSO_OFF_DSC_START_RO)

char* vso_str_err(int err);
void vso_report_timing(t_vso* handle);

#endif /* VSO_STR_H_ */
