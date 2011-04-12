/*
 * eti_hal.h
 *
 * Created on: 08.12.2010
 *     Author: buan
 */

#ifndef ETI_HAL_H_
#define ETI_HAL_H_

#include "stdhal.h"
#include "stdrbf.h"
#include "esc_reg.h"

/* constants of hardware */

/* macros set*/
#define esc_set_control_reg(p, v)                   HOI_WR32((p), ESC_CONTROL_REG, (v))
#define esc_set_eth_speed_1000(p)                   HOI_REG_SET((p), ESC_CONTROL_REG, ESC_CONTROL_SPEED_1000)
#define esc_clr_eth_speed_1000(p)                   HOI_REG_CLR((p), ESC_CONTROL_REG, ESC_CONTROL_SPEED_1000)
#define esc_set_eth_speed_100(p)                    HOI_REG_SET((p), ESC_CONTROL_REG, ESC_CONTROL_SPEED_100)
#define esc_clr_eth_speed_100(p)                    HOI_REG_CLR((p), ESC_CONTROL_REG, ESC_CONTROL_SPEED_100)
#define esc_set_eth_speed_10(p)                     HOI_REG_SET((p), ESC_CONTROL_REG, ESC_CONTROL_SPEED_10)
#define esc_clr_eth_speed_10(p)                     HOI_REG_CLR((p), ESC_CONTROL_REG, ESC_CONTROL_SPEED_10)
#define esc_set_write_param(p)                      HOI_REG_SET((p), ESC_CONTROL_REG, ESC_CONTROL_WRITE_PARAM)
#define esc_set_reconfig(p)                         HOI_REG_SET((p), ESC_CONTROL_REG, ESC_CONTROL_RECONFIG)
#define esc_set_areset(p)                           HOI_REG_SET((p), ESC_CONTROL_REG, ESC_CONTROL_ARESET)
#define esc_set_read_param(p)                       HOI_REG_SET((p), ESC_CONTROL_REG, ESC_CONTROL_READ_PARAM)

#define esc_set_phy_reg(p, r, v)                    HOI_WR32((p), (r), (v))   
#define esc_set_phy_auto_neg_advertise_reg(p, v)    HOI_WR32((p), PHY_AUTO_NEG_ADVERTISE_REG, (v))
#define esc_set_phy_link_partner_ability_reg(p, v)  HOI_WR32((p), PHY_LINK_PARTNER_ABILITY_REG, (v))
#define esc_set_phy_auto_neg_expansion_reg(p, v)    HOI_WR32((p), PHY_AUTO_NEG_EXPANSION_REG, (v))
#define esc_set_phy_1000_base_t_control_reg(p, v)   HOI_WR32((p), PHY_1000_BASE_T_CONTROL_REG, (v))
#define esc_set_phy_1000_base_t_status_reg(p, v)    HOI_WR32((p), PHY_1000_BASE_T_STATUS_REG, (v))
#define esc_set_phy_spec_status_reg(p, v)           HOI_WR32((p), PHY_SPEC_STATUS_REG, (v))


/* macros get*/
#define esc_get_status(p)                           HOI_RD32((p), ESC_STATUS_REG)
#define esc_get_eth_mode(p)                         HOI_REG_TST(p, ESC_STATUS_REG, ESC_STATUS_ETH_MODE)
#define esc_get_ena_10(p)                           HOI_REG_TST(p, ESC_STATUS_REG, ESC_STATUS_ENA_10)
#define esc_get_pll_locked(p)                       HOI_REG_TST(p, ESC_STATUS_REG, ESC_STATUS_LOCKED)
#define esc_get_control(p)                          HOI_RD32((p), ESC_CONTROL_REG)
#define esc_get_busy(p)                             HOI_REG_TST(p, ESC_STATUS_REG, ESC_STATUS_BUSY)

#define esc_get_phy_reg(p, v)                       HOI_RD32((p), (v)) 
#define esc_get_phy_link_partner(p)                 HOI_RD32((p), PHY_LINK_PARTNER_ABILITY_REG)  
#define esc_get_phy_auto_neg_advertise_reg(p)       HOI_RD32((p), PHY_AUTO_NEG_ADVERTISE_REG)
#define esc_get_phy_link_partner_ability_reg(p)     HOI_RD32((p), PHY_LINK_PARTNER_ABILITY_REG)
#define esc_get_phy_auto_neg_expansion_reg(p)       HOI_RD32((p), PHY_AUTO_NEG_EXPANSION_REG)
#define esc_get_phy_1000_base_t_control_reg(p)      HOI_RD32((p), PHY_1000_BASE_T_CONTROL_REG)
#define esc_get_phy_1000_base_t_status_reg(p)       HOI_RD32((p), PHY_1000_BASE_T_STATUS_REG)
#define esc_get_phy_spec_status_reg(p)              HOI_RD32((p), PHY_SPEC_STATUS_REG)

#define esc_get_phy_10_full_duplex(p)               HOI_REG_TST(p, PHY_LINK_PARTNER_ABILITY_REG, PHY_10_BASE_T_FULL_DUPLEX)
#define esc_get_phy_10_half_duplex(p)               HOI_REG_TST(p, PHY_LINK_PARTNER_ABILITY_REG, PHY_10_BASE_T_HALF_DUPLEX)
#define esc_get_phy_100_full_duplex(p)              HOI_REG_TST(p, PHY_LINK_PARTNER_ABILITY_REG, PHY_100_BASE_T_FULL_DUPLEX)
#define esc_get_phy_100_half_duplex(p)              HOI_REG_TST(p, PHY_LINK_PARTNER_ABILITY_REG, PHY_100_BASE_T_HALF_DUPLEX)
#define esc_get_phy_1000_full_duplex(p)             HOI_REG_TST(p, PHY_1000_BAST_T_STATUS_REG, PHY_1000_BASE_T_FULL_DUPLEX)
#define esc_get_phy_1000_half_duplex(p)             HOI_REG_TST(p, PHY_1000_BAST_T_STATUS_REG, PHY_1000_BASE_t_HALF_DUPLEX)

#define esc_get_phy_spec_status_duplex(p)           HOI_REG_TST(p, PHY_SPEC_STATUS_REG, PHY_SPEC_STATUS_DUPLEX)
#define esc_get_phy_spec_status_link(p)             HOI_REG_TST(p, PHY_SPEC_STATUS_REG, PHY_SPEC_STATUS_LINK)

#define esc_get_mac_command_config(p)               HOI_RD32((p), ESC_MAC_COMMAND_CONFIG_REG)


    
/* function prototypes */
void esc_set_type (void* p, uint32_t type);
void esc_set_param (void* p, uint32_t param);
void esc_set_data_in (void* p, uint32_t data_in);
uint32_t esc_get_data_out (void* p);
uint32_t esc_get_link_speed (void* p);

#endif /* ETI_HAL_H_ */

