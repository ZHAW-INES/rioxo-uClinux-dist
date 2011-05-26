#ifndef HDCP_HAL_H_
#define HDCP_HAL_H_

#include "stdhal.h"
#include "hdcp_reg.h"


#define hdcp_set_cfg(p, m)       HOI_REG_SET((p), HDCP_OFF_CONFIG_REG, (m))
#define hdcp_clr_cfg(p, m)       HOI_REG_CLR((p), HDCP_OFF_CONFIG_REG, (m))
#define hdcp_get_cfg(p, m)       HOI_REG_RD((p), HDCP_OFF_CONFIG_REG, (m))

#define hdcp_get_status(p, m)    HOI_REG_RD((p), HDCP_OFF_STATUS_REG, (m))

#define hdcp_enable(p)           hdcp_set_cfg((p), HDCP_BIT_CFG_EN)
#define hdcp_disable(p)          hdcp_clr_cfg((p), HDCP_BIT_CFG_EN)
#define hdcp_load(p)             hdcp_set_cfg((p), HDCP_BIT_CFG_LOAD)

#define hdcp_set_time(p, v)      HOI_WR32((p), HDCP_OFF_START_TIME, (v))
#define hdcp_get_time(p)         HOI_RD32((p), HDCP_OFF_START_TIME)
#define hdcp_get_master_key0(p)  HOI_RD32((p), HDCP_OFF_MASTER_KEY0)
#define hdcp_get_master_key1(p)  HOI_RD32((p), HDCP_OFF_MASTER_KEY1)
#define hdcp_get_master_key2(p)  HOI_RD32((p), HDCP_OFF_MASTER_KEY2)
#define hdcp_get_master_key3(p)  HOI_RD32((p), HDCP_OFF_MASTER_KEY3)

void hdcp_get_master_key(void *p, uint32_t master_key[4]);

#endif /* HDCP_HAL_H_ */
