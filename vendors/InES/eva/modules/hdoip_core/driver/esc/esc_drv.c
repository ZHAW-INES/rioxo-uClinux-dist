/*
 * esc_drv.c
 *
 *  Created on: 03.02.2011
 *      Author: buan
 */

#include "esc_drv.h"


/** Set ETH-PLL to 125MHz
 *  (Configuration: N=Bypass, M=5, C0=2 --> 50MHz * 5 / 2 = 125MHz)
 */
int esc_set_pll_1000(t_esc* handle) {

    esc_set_pll_param(handle, ESC_TYPE_C0, ESC_PARAM_HIGH_COUNT, ESC_PLL_125_MHZ);
    esc_set_pll_param(handle, ESC_TYPE_C0, ESC_PARAM_LOW_COUNT, ESC_PLL_125_MHZ);
    esc_set_pll_param(handle, ESC_TYPE_C1, ESC_PARAM_HIGH_COUNT, ESC_PLL_125_MHZ);
    esc_set_pll_param(handle, ESC_TYPE_C1, ESC_PARAM_LOW_COUNT, ESC_PLL_125_MHZ);
    esc_reconfig_pll_param(handle);

    return 0;
}


/** Set ETH-PLL to 25MHz
 *  (Configuration: N=Bypass, M=5, C0=10 --> 50MHz * 5 / 10 = 25MHz)
 */
int esc_set_pll_100(t_esc* handle) {

    esc_set_pll_param(handle, ESC_TYPE_C0, ESC_PARAM_HIGH_COUNT, ESC_PLL_25_MHZ);
    esc_set_pll_param(handle, ESC_TYPE_C0, ESC_PARAM_LOW_COUNT, ESC_PLL_25_MHZ);
    esc_set_pll_param(handle, ESC_TYPE_C1, ESC_PARAM_HIGH_COUNT, ESC_PLL_25_MHZ);
    esc_set_pll_param(handle, ESC_TYPE_C1, ESC_PARAM_LOW_COUNT, ESC_PLL_25_MHZ);
    esc_reconfig_pll_param(handle);

    return 0;
}


/** Set ETH-PLL to 2.5MHz
 *  (Configuration: N=Bypass, M=5, C0=100 --> 50MHz * 5 / 100 = 2.5MHz)
 */
int esc_set_pll_10(t_esc* handle) {

    esc_set_pll_param(handle, ESC_TYPE_C0, ESC_PARAM_HIGH_COUNT, ESC_PLL_2_5_MHZ);
    esc_set_pll_param(handle, ESC_TYPE_C0, ESC_PARAM_LOW_COUNT, ESC_PLL_2_5_MHZ);
    esc_set_pll_param(handle, ESC_TYPE_C1, ESC_PARAM_HIGH_COUNT, ESC_PLL_2_5_MHZ);
    esc_set_pll_param(handle, ESC_TYPE_C1, ESC_PARAM_LOW_COUNT, ESC_PLL_2_5_MHZ);
    esc_reconfig_pll_param(handle);

    return 0;
}


/** Set ethernet speed, controlled by phy
 *
 */
int esc_handler(t_esc* handle) {

    static int speed_old    = 0;
    uint32_t speed          = esc_get_link_speed (handle->phy_ptr);
    uint32_t link           = esc_get_phy_spec_status_link(handle->phy_ptr);
    uint32_t duplex         = esc_get_phy_spec_status_duplex(handle->phy_ptr);


    // If ethernet-phy changes speed, transmit mode or link-status (due to autonegotiation)
    if(link != handle->link) {
        handle->link = link;
        if(link) {
            if(speed != handle->phy_speed) {
                handle->phy_speed = speed;
                handle->eth_speed = speed;
            }
            if(duplex != handle->duplex) {
                handle->duplex = duplex;
            }
        }
    }


    if (speed_old != handle->eth_speed) {

        // Set speed select input pins of TSE-MAC
        if (handle->eth_speed == 1000) {
            esc_set_eth_speed_1000(handle->ptr);
            esc_clr_eth_speed_100(handle->ptr);
            esc_clr_eth_speed_10(handle->ptr);
        }
        if (handle->eth_speed == 100) {
            esc_clr_eth_speed_1000(handle->ptr);
            esc_set_eth_speed_100(handle->ptr);
            esc_clr_eth_speed_10(handle->ptr);
        }
        if (handle->eth_speed == 10) {
            esc_clr_eth_speed_1000(handle->ptr);
            esc_clr_eth_speed_100(handle->ptr);
            esc_set_eth_speed_10(handle->ptr);
        }
        
        // Set PLL configuration due to speed select output pins of TSE-MAC
        if (esc_get_eth_mode(handle->ptr))
            esc_set_pll_1000(handle);
        else {
            if (esc_get_ena_10(handle->ptr))
                esc_set_pll_10(handle);
            else
                esc_set_pll_100(handle);
        }

        speed_old = handle->eth_speed;
    }

    return 0;
}


/** Ethernet Speed Control Initialization
 *
 */
int esc_init(t_esc* handle, void* ptr, void* mac_ptr) {

    handle->ptr = ptr;
    handle->mac_ptr = mac_ptr;
    handle->phy_ptr = mac_ptr + ESC_MDIO_OFFSET;
    handle->link = 0;
    handle->duplex = 0;
    handle->eth_speed = 1000;

    return 0;
}


/** Read parameters of reconfiguration block
 *
 */
int esc_read_pll_param(t_esc* handle, int32_t type, int32_t param) {

    esc_set_type (handle->ptr, type);
    esc_set_param (handle->ptr, param);
    esc_set_read_param(handle->ptr);
    while (esc_get_busy(handle->ptr));

    return esc_get_data_out (handle->ptr);
}


/** Set parameters of reconfiguration block
 *
 */
int esc_set_pll_param(t_esc* handle, int32_t type, int32_t param, int32_t data_in) {

    esc_set_type (handle->ptr, type);
    esc_set_param (handle->ptr, param);
    esc_set_data_in (handle->ptr, data_in);
    esc_set_write_param(handle->ptr);
    while (esc_get_busy(handle->ptr));

    return 0;
}


/** Reconfigure ethernet PLL with values stored in reconfiguration block
 *
 */
int esc_reconfig_pll_param(t_esc* handle) {

    esc_set_reconfig(handle->ptr);
    while (esc_get_busy(handle->ptr));

    return 0;
}

