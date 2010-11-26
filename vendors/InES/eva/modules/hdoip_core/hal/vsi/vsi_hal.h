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

#define vsi_set_dsc(p, dsc)         rbf_set_dsc_w(p, VSI_OFF_DSC_START_RO, dsc)
#define vsi_get_dsc(p, dsc)         rbf_get_dsc(p, VSI_OFF_DSC_START_RO, dsc)

#define vsi_set_eth_word_len(p, v)  HOI_WR32(p, VSI_OFF_ETH_WORD_LEN_RW, v)
#define vsi_get_eth_word_len(p)     HOI_RD32(p, VSI_OFF_ETH_WORD_LEN_RW)
#define vsi_set_src_ip(p, v)        HOI_WR32(p, VSI_OFF_SRC_IP_RW, v)
#define vsi_get_src_ip(p)           HOI_RD32(p, VSI_OFF_SRC_IP_RW)
#define vsi_set_dst_ip(p, v)        HOI_WR32(p, VSI_OFF_DST_IP_RW, v)
#define vsi_get_dst_ip(p)           HOI_RD32(p, VSI_OFF_DST_IP_RW)
#define vsi_set_src_port(p, v)      HOI_WR32(p, VSI_OFF_SRC_PORT_RW, v)
#define vsi_get_src_port(p)         HOI_RD32(p, VSI_OFF_SRC_PORT_RW)
#define vsi_set_dst_port(p, v)      HOI_WR32(p, VSI_OFF_DST_PORT_RW, v)
#define vsi_get_dst_port(p)         HOI_RD32(p, VSI_OFF_DST_PORT_RW)
#define vsi_set_ssrc(p, v)          HOI_WR32(p, VSI_OFF_SSRC_RW, v)
#define vsi_get_ssrc(p)             HOI_RD32(p, VSI_OFF_SSRC_RW)
#define vsi_set_burst_size(p, v)    HOI_WR32(p, VSI_OFF_BURST_SIZE_RW, v)
#define vsi_get_burst_size(p)       HOI_RD32(p, VSI_OFF_BURST_SIZE_RW)

#define vsi_get_disc_vframes(p)     HOI_RD32(p, VSI_OFF_DISC_VFRAMES_RW)

/* function prototypes */
void vsi_set_dst_mac(void* p, uint16_t upper16, uint32_t lower32);
void vsi_get_dst_mac(void* p, uint16_t* upper16, uint32_t* lower32);
void vsi_set_src_mac(void* p, uint16_t upper16, uint32_t lower32);
void vsi_get_src_mac(void* p, uint16_t* upper16, uint32_t* lower32);
void vsi_set_ttl(void* p, uint8_t ttl);
uint8_t vsi_get_ttl(void* p);
void vsi_set_tos(void* p, uint8_t tos);
uint8_t vsi_get_tos(void* p);
void vsi_set_eth_params(void* p, struct hdoip_eth_params* eth_params);
void vsi_get_eth_params(void* p, struct hdoip_eth_params* eth_params);


#endif /* VSI_HAL_H_ */
