#ifndef VSI_STR_H_
#define VSI_STR_H_

#include "std.h"
#include "vsi_drv_cfg.h"
#include "vsi_hal.h"

#define vsi_report_dsc(p)       hoi_report_rbf(p, VSI_OFF_DSC_START_RD)

char* vsi_str_err(int err);
void vsi_report_eth(t_vsi* handle); 

#endif /* VSI_STR_H_ */
