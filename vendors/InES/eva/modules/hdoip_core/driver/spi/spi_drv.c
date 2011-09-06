/*
 * spi_drv.c
 *
 *  Created on: 01.09.2011
 *      Author: buan
 *      (based on alteras "altera_avalon_spi.c")
 */

#include "spi_drv.h"

uint16_t spi_read_reg_16(void *ptr_spi, uint16_t offset)
{
    uint16_t read_data = 0x00;
    uint16_t write_data = 0x00;
    uint16_t command_word = SPI_COMMAND_RW_READ | (offset & 0x0FFF);

    alt_avalon_spi_command(ptr_spi, 0x00, 0x02, &command_word, 0x02, &read_data, 0x00);
    alt_avalon_spi_command(ptr_spi, 0x00, 0x02, &write_data,   0x02, &read_data, 0x00);

    return read_data;
}

void spi_write_reg_16(void *ptr_spi, uint16_t offset, uint16_t value)
{
    uint8_t read_data = 0x00;
    uint8_t write_data = value;
    uint16_t command_word = SPI_COMMAND_RW_WRITE | (offset & 0x0FFF);

    alt_avalon_spi_command(ptr_spi, 0x00, 0x02, &command_word, 0x02, &read_data, 0x00);
    alt_avalon_spi_command(ptr_spi, 0x00, 0x02, &write_data,   0x02, &read_data, 0x00);
}



int alt_avalon_spi_command(uint32_t base, uint32_t slave, uint32_t write_length, const uint32_t *write_data, uint32_t read_length, uint32_t *read_data, uint32_t flags)
{
    uint32_t *write_end = write_data + write_length;
    uint32_t *read_end  = read_data + read_length;

    uint32_t write_zeros = read_length;
    uint32_t read_ignore = write_length;
    uint32_t status;
    uint32_t rxdata;

    /* We must not send more than two bytes to the target before it has
     * returned any as otherwise it will overflow. */
    /* Unfortunately the hardware does not seem to work with credits > 1,
     * leave it at 1 for now. */
    uint32_t credits = 1;

    /* Warning: this function is not currently safe if called in a multi-threaded
     * environment, something above must perform locking to make it safe if more
     * than one thread intends to use it.
     */
    write_spi_slave_sel(base, 1 << slave);

    /* Set the SSO bit (force chipselect) only if the toggle flag is not set */
    if ((flags & FLAG_SPI_COMMAND_TOGGLE_SS_N) == 0) {
        write_spi_control(base, SPI_CONTROL_SSO_MSK);
    }

    /*
    * Discard any stale data present in the RXDATA register, in case
    * previous communication was interrupted and stale data was left
    * behind.
    */
    read_spi_rxdata(base);
    
    /* Keep clocking until all the data has been processed. */
    while(1) {
        do {
            status = read_spi_status(base);
        }
        while (((status & SPI_STATUS_TRDY_MSK) == 0 || credits == 0) && (status & SPI_STATUS_RRDY_MSK) == 0);

        if ((status & SPI_STATUS_TRDY_MSK) != 0 && credits > 0) {
            credits--;
            if (write_data < write_end) {
                write_spi_txdata(base, *write_data++);
            } else if (write_zeros > 0) {
                write_zeros--;
                write_spi_txdata(base, 0);
            } else {
                credits = -1024;
            }
        }

        if ((status & SPI_STATUS_RRDY_MSK) != 0) {
            rxdata = read_spi_rxdata(base);
            if (read_ignore > 0) {
                read_ignore--;
            } else {
                *read_data++ = (char)rxdata;
            }

            credits++;

            if (read_ignore == 0 && read_data == read_end) {
                break;
            }
        }
    }
    /* Wait until the interface has finished transmitting */
    do {
        status = read_spi_status(base);
    }
    while ((status & SPI_STATUS_TMT_MSK) == 0);

    /* Clear SSO (release chipselect) unless the caller is going to
     * keep using this chip
     */
    if ((flags & FLAG_SPI_COMMAND_MERGE) == 0) {
        write_spi_control(base, 0);
    }
    return read_length;
}
