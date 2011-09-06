#ifndef SPI_DRV_CFG_H_
#define SPI_DRV_CFG_H_

// register
#define SPI_RXDATA_REG                    (0x00000000)
#define SPI_TXDATA_REG                    (0x00000004)
#define SPI_STATUS_REG                    (0x00000008)
#define SPI_CONTROL_REG                   (0x0000000C)
#define SPI_SLAVE_SEL_REG                 (0x00000010)

// status
#define SPI_STATUS_ROE_MSK                (0x8)
#define SPI_STATUS_ROE_OFST               (3)
#define SPI_STATUS_TOE_MSK                (0x10)
#define SPI_STATUS_TOE_OFST               (4)
#define SPI_STATUS_TMT_MSK                (0x20)
#define SPI_STATUS_TMT_OFST               (5)
#define SPI_STATUS_TRDY_MSK               (0x40)
#define SPI_STATUS_TRDY_OFST              (6)
#define SPI_STATUS_RRDY_MSK               (0x80)
#define SPI_STATUS_RRDY_OFST              (7)
#define SPI_STATUS_E_MSK                  (0x100)
#define SPI_STATUS_E_OFST                 (8)

// control
#define SPI_CONTROL_IROE_MSK              (0x8)
#define SPI_CONTROL_IROE_OFST             (3)
#define SPI_CONTROL_ITOE_MSK              (0x10)
#define SPI_CONTROL_ITOE_OFST             (4)
#define SPI_CONTROL_ITRDY_MSK             (0x40)
#define SPI_CONTROL_ITRDY_OFS             (6)
#define SPI_CONTROL_IRRDY_MSK             (0x80)
#define SPI_CONTROL_IRRDY_OFS             (7)
#define SPI_CONTROL_IE_MSK                (0x100)
#define SPI_CONTROL_IE_OFST               (8)
#define SPI_CONTROL_SSO_MSK               (0x400)
#define SPI_CONTROL_SSO_OFST              (10)

#define SPI_COMMAND_RW_READ               (0x8000)
#define SPI_COMMAND_RW_WRITE              (0x0000)
#define SPI_COMMAND_AUTOINC_ON            (0x1000)

#endif /* SPI_DRV_CFG_H_ */
