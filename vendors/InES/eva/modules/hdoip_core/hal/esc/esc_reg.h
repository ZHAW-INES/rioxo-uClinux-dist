/*
 * esc_reg.h
 *
 * Created on: 17.01.2011
 *     Author: buan
 */

#ifndef ESC_REG_H_
#define ESC_REG_H_


/** register byte offset
 *
 */

#define ESC_CONTROL_REG                     0
#define ESC_STATUS_REG                      4

#define ESC_ADDR_SPACE                      8

#define PHY_AUTO_NEG_ADVERTISE_REG          16              // Page 0, Register 4
#define PHY_LINK_PARTNER_ABILITY_REG        20              // Page 0, Register 5
#define PHY_AUTO_NEG_EXPANSION_REG          24              // Page 0, Register 6
#define PHY_1000_BASE_T_CONTROL_REG         36              // Page 0, Register 9
#define PHY_1000_BASE_T_STATUS_REG          40              // Page 0, Register 10
#define PHY_SPEC_STATUS_REG                 68              // Page 0, Register 17

#define ESC_MAC_COMMAND_CONFIG_REG          8


/** bit indexes 
 *
 */

// Control Register //
#define ESC_CONTROL_SPEED_1000              0x00000010
#define ESC_CONTROL_SPEED_100               0x00000020
#define ESC_CONTROL_SPEED_10                0x00000040
#define ESC_CONTROL_TYPE_SHIFT              8               // range 11 downto 8;
#define ESC_CONTROL_PARAM_SHIFT             12              // range 14 downto 12;
#define ESC_CONTROL_DATA_IN_SHIFT           15              // range 23 downto 15;
#define ESC_CONTROL_WRITE_PARAM             0x01000000
#define ESC_CONTROL_RECONFIG                0x02000000
#define ESC_CONTROL_ARESET                  0x04000000
#define ESC_CONTROL_READ_PARAM              0x08000000

// Status Register //
#define ESC_STATUS_SPEED_1000               0x00000010
#define ESC_STATUS_SPEED_100                0x00000020
#define ESC_STATUS_SPEED_10                 0x00000040
#define ESC_STATUS_ENA_10                   0x00000080
#define ESC_STATUS_ETH_MODE                 0x00000100
#define ESC_STATUS_LOCKED                   0x00000200
#define ESC_STATUS_TEST                     0x00000400
#define ESC_STATUS_DATA_OUT_SHIFT           11              // range 19 downto 11;
#define ESC_STATUS_BUSY                     0x00100000

// PHY: Link partner ability register //
#define PHY_10_BASE_T_FULL_DUPLEX           0x00000040
#define PHY_10_BASE_T_HALF_DUPLEX           0x00000020
#define PHY_100_BASE_T_FULL_DUPLEX          0x00000100
#define PHY_100_BASE_T_HALF_DUPLEX          0x00000080

// PHY: 1000 Base-T status Register //
#define PHY_1000_BASE_T_FULL_DUPLEX         0x00000800
#define PHY_1000_BASE_t_HALF_DUPLEX         0x00000400

// PHY: Specific Status Register //
#define PHY_SPEC_STATUS_SPEED_SHIFT         14
#define PHY_SPEC_STATUS_DUPLEX              0x00002000
#define PHY_SPEC_STATUS_LINK                0x00000400
#define PHY_SPEC_STATUS_SPEED_MASK          3
#define PHY_SPEC_STATUS_SPEED_1000          2
#define PHY_SPEC_STATUS_SPEED_100           1
#define PHY_SPEC_STATUS_SPEED_10            0


/** bit masks 
 *
 */

#define ESC_CONTROL_PARAM_MASK              0x00000007
#define ESC_CONTROL_TYPE_MASK               0x0000000F
#define ESC_CONTROL_DATA_IN_MASK            0x000001FF
#define ESC_STATUS_DATA_OUT_MASK            0x000001FF

#endif /* ESC_REG_H_ */
