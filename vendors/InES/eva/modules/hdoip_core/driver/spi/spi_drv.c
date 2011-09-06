/*
 * spi_drv.c
 *
 *  Created on: 01.09.2011
 *      Author: buan
 */

#include "spi_drv.h"

uint16_t spi_read_reg_16(void *ptr_spi, uint16_t offset)
{
    uint16_t read_data = 0x0000;
    uint16_t write_data = 0x0000;
    uint16_t command_word = SPI_COMMAND_RW_READ | (offset & 0x0FFF);

    spi_command(ptr_spi, command_word);
    read_data = spi_command(ptr_spi, write_data);

    return read_data;
}

void spi_write_reg_16(void *ptr_spi, uint16_t offset, uint16_t value)
{
    uint16_t read_data = 0x0000;
    uint16_t write_data = value;
    uint16_t command_word = SPI_COMMAND_RW_WRITE | (offset & 0x0FFF);

    spi_command(ptr_spi, command_word);
    spi_command(ptr_spi, write_data);
}

uint16_t spi_command(uint32_t p_spi, uint16_t write_data)
{
    //wait until gateware is ready to transmit
    while ((read_spi_status(p_spi) & SPI_STATUS_TRDY_MSK) == 0); 
    //transmit data
    write_spi_txdata(p_spi, (write_data));
    //wait until received data is ready to be read out
    while ((read_spi_status(p_spi) & SPI_STATUS_RRDY_MSK) == 0); 
    
    return read_spi_rxdata(p_spi);
}
