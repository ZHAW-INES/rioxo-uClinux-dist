#ifndef VSI_HAL_H_
#define VSI_HAL_H_

#include "std.h"
#include "stdrbf.h"
#include "stdeth.h"
#include "vsi_reg.h"

#define vsi_set_ctrl(p, m)          HOI_REG_SET(p, VSI_OFF_CONTROL_WR, (m))
#define vsi_clr_ctrl(p, m)          HOI_REG_SET((p), VSI_OFF_CONTROL_CLR, (m))
#define vsi_get_ctrl(p, m)          HOI_REG_RD((p), VSI_OFF_CONTROL_RD, (m))
#define vsi_clr_status(p, m)        HOI_REG_SET((p), VSI_OFF_STATUS_CLR, (m))
#define vsi_get_status(p, m)        HOI_REG_RD((p), VSI_OFF_STATUS_RD, (m))

#define vsi_enable(p)               HOI_REG_SET((p), VSI_OFF_CONTROL_WR, (VSI_CFG_RUN))
#define vsi_disable(p)              HOI_REG_SET((p), VSI_OFF_CONTROL_CLR, (VSI_CFG_RUN))

#define vsi_set_eth_word_len(p, v)  HOI_WR32(p, VSI_OFF_ETH_WORD_LEN_RW, v)
#define vsi_get_eth_word_len(p)     HOI_RD32(p, VSI_OFF_ETH_WORD_LEN_RW)

#define vsi_set_burst_size(p, v)    HOI_WR32(p, VSI_OFF_BURST_SIZE_RW, v)
#define vsi_get_burst_size(p)       HOI_RD32(p, VSI_OFF_BURST_SIZE_RW)

#define vsi_get_disc_vframes(p)     HOI_RD32(p, VSI_OFF_DISC_VFRAMES_RW)

#endif /* VSI_HAL_H_ */
