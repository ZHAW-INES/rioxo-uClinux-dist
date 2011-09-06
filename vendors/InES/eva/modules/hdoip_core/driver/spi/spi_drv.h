#ifndef SPI_DRV_H_
#define SPI_DRV_H_

#include "std.h"
#include "spi_reg.h"
#include "hoi_msg.h"

#define read_spi_rxdata(p)              HOI_RD8((p), SPI_RXDATA_REG) 
#define write_spi_rxdata(p, v)          HOI_WR8((p), SPI_RXDATA_REG, (v))

#define read_spi_txdata(p)              HOI_RD8((p), SPI_TXDATA_REG) 
#define write_spi_txdata(p, v)          HOI_WR8((p), SPI_TXDATA_REG, (v))

#define read_spi_status(p)              HOI_RD8((p), SPI_STATUS_REG) 
#define write_spi_status(p, v)          HOI_WR8((p), SPI_STATUS_REG, (v))

#define read_spi_control(p)             HOI_RD8((p), SPI_CONTROL_REG) 
#define write_spi_control(p, v)         HOI_WR8((p), SPI_CONTROL_REG, (v))

#define read_spi_slave_sel(p)           HOI_RD8((p), SPI_SLAVE_SEL_REG) 
#define write_spi_slave_sel(p, v)       HOI_WR8((p), SPI_SLAVE_SEL_REG, (v))

// control flags

/* If you need to make multiple accesses to the same slave then you should 
 * set the merge bit in the flags for all of them except the first.
 */
#define FLAG_SPI_COMMAND_MERGE (0x01)

/*
 * If you need the slave select line to be toggled between words then you
 * should set the toggle bit in the flag.
 */
#define FLAG_SPI_COMMAND_TOGGLE_SS_N (0x02)


int alt_avalon_spi_command(uint32_t base, uint32_t slave, uint32_t write_length, const uint32_t *write_data, uint32_t read_length, uint32_t *read_data, uint32_t flags);
uint16_t spi_read_reg_16(void *ptr_spi, uint16_t offset);
void spi_write_reg_16(void *ptr_spi, uint16_t offset, uint16_t value);
#endif /* SPI_DRV_H_ */
