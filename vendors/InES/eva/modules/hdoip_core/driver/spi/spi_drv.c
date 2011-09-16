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
    uint32_t tmp = read_spi_control(ptr_spi);

    //set chip select
    write_spi_control(ptr_spi, (tmp | SPI_CONTROL_SSO_MSK));
    //write command word
    spi_command(ptr_spi, command_word);
    //read data
    read_data = spi_command(ptr_spi, write_data);
    //clear chip select
    write_spi_control(ptr_spi, tmp);

    return read_data;
}

void spi_write_reg_16(void *ptr_spi, uint16_t offset, uint16_t value)
{
    uint16_t read_data = 0x0000;
    uint16_t write_data = value;
    uint16_t command_word = SPI_COMMAND_RW_WRITE | (offset & 0x0FFF);
    uint32_t tmp = read_spi_control(ptr_spi);

    //set chip select
    write_spi_control(ptr_spi, (tmp | SPI_CONTROL_SSO_MSK));
    //write command word
    spi_command(ptr_spi, command_word);
    //write data
    spi_command(ptr_spi, write_data);
    //clear chip select
    write_spi_control(ptr_spi, tmp);
}

uint16_t spi_command(uint32_t ptr_spi, uint16_t write_data)
{
    //wait until gateware is ready to transmit
    while ((read_spi_status(ptr_spi) & SPI_STATUS_TRDY_MSK) == 0); 
    //transmit data
    write_spi_txdata(ptr_spi, (write_data));
    //wait until received data is ready to be read out
    while ((read_spi_status(ptr_spi) & SPI_STATUS_RRDY_MSK) == 0); 

    return read_spi_rxdata(ptr_spi);
}
