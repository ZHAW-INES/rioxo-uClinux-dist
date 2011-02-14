#ifndef ESC_DRV_CFG_H_
#define ESC_DRV_CFG_H_

/* ESC configuration */

#define ESC_MDIO_OFFSET                     (0x00000200)

#define ESC_TYPE_C0                         4
#define ESC_TYPE_C1                         5
#define ESC_TYPE_C2                         6
#define ESC_TYPE_C3                         7
#define ESC_TYPE_M                          1
#define ESC_TYPE_N                          0

#define ESC_PARAM_HIGH_COUNT                0
#define ESC_PARAM_LOW_COUNT                 1
#define ESC_PARAM_BYPASS                    4
#define ESC_PARAM_MODE                      5

#define ESC_PLL_125_MHZ                     2
#define ESC_PLL_25_MHZ                      10
#define ESC_PLL_2_5_MHZ                     100
#define ESC_PLL_M                           5

typedef struct {
    void*       ptr;
    void*       mac_ptr;
    void*       phy_ptr;
    uint32_t    eth_speed;
    uint32_t    link;
    uint32_t    duplex;
    uint32_t    phy_speed;
} t_esc;

#endif /* ESC_DRV_CFG_H_ */
