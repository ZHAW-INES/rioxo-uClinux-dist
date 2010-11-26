#ifndef ASI_STR_H_
#define ASI_STR_H_

#include "std.h"
#include "asi_drv_cfg.h"

char* asi_str_err(int err);
int asi_str_report_eth(t_asi* handle); 
int asi_str_report_aud(t_asi* handle);

#endif /* ASI_STR_H_ */
