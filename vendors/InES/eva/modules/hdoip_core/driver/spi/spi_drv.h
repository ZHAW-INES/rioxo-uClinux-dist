#ifndef SPI_DRV_H_
#define SPI_DRV_H_

#include "std.h"
#include "spi_reg.h"
#include "hoi_msg.h"

#define read_spi_rxdata(p)              HOI_RD32((p), SPI_RXDATA_REG) 
#define write_spi_rxdata(p, v)          HOI_WR32((p), SPI_RXDATA_REG, (v))

#define read_spi_txdata(p)              HOI_RD32((p), SPI_TXDATA_REG) 
#define write_spi_txdata(p, v)          HOI_WR32((p), SPI_TXDATA_REG, (v))

#define read_spi_status(p)              HOI_RD32((p), SPI_STATUS_REG) 
#define write_spi_status(p, v)          HOI_WR32((p), SPI_STATUS_REG, (v))

#define read_spi_control(p)             HOI_RD32((p), SPI_CONTROL_REG) 
#define write_spi_control(p, v)         HOI_WR32((p), SPI_CONTROL_REG, (v))

#define read_spi_slave_sel(p)           HOI_RD32((p), SPI_SLAVE_SEL_REG) 
#define write_spi_slave_sel(p, v)       HOI_WR32((p), SPI_SLAVE_SEL_REG, (v))


uint16_t spi_read_reg_16(void *ptr_spi, uint16_t offset);
void spi_write_reg_16(void *ptr_spi, uint16_t offset, uint16_t value);
uint16_t spi_command(void* p_spi, uint16_t write_data);

#endif /* SPI_DRV_H_ */
