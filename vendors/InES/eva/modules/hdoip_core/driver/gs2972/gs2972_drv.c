/*
 * gs2972_drv.c
 *
 *  Created on: 02.09.2011
 *      Author: buan
 */

#include "gs2972_drv.h"
#include "spi_drv.h"
#include "io_expander_tx.h"
#include "bdt_drv.h"

void gs2972_driver_init(t_gs2972 *handle, void *spi_ptr, void *i2c_ptr, void *video_mux_ptr)
{
    handle->p_spi = spi_ptr;
    handle->p_i2c = i2c_ptr;

    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| GS2972-Driver: Initialize SDI-TX                 |");
    REPORT(INFO, "+--------------------------------------------------+");

    // initialize i2c port-expander
    init_io_exp_tx(handle->p_i2c);

    // enable serial data output signal
    set_io_exp_tx_pin(handle->p_i2c, TX_SDO_EN);

    // power up device
    clr_io_exp_tx_pin(handle->p_i2c, TX_STANDBY);

    // enable SMPTE processing
    set_io_exp_tx_pin(handle->p_i2c, TX_SMPTE_BYPASS);
    clr_io_exp_tx_pin(handle->p_i2c, TX_DVB_ASI);
    set_io_exp_tx_pin(handle->p_i2c, TX_IOPROC_EN);

    // luma and chroma data are cleared during blanking periods
    clr_io_exp_tx_pin(handle->p_i2c, TX_ANC_BLANK);

    // enable audio output
    set_io_exp_tx_pin(handle->p_i2c, TX_AUDIO_1_EN);
    set_io_exp_tx_pin(handle->p_i2c, TX_AUDIO_2_EN);

    // clear reset pin
    bdt_drv_clear_reset_1(video_mux_ptr);

    // set rate set to 270Mb/s
    gs2972_driver_set_rate_sel(handle, 270);

    // enable all ioproc functions / disable conversion to SMPTE 372M
    spi_write_reg_16(handle->p_spi, GS2972_IOPROC, IOPROC_RW_DEFAULT | IOPROC_RW_CONV_372);

    // set drive strength to min
    spi_write_reg_16(handle->p_spi, GS2972_DRIVE_STRENGTH, DRIVE_STRENGTH_RW_4_MA);
    spi_write_reg_16(handle->p_spi, GS2972_DRIVE_STRENGTH2, DRIVE_STRENGTH2_RW_4_MA);

}

void gs2972_handler(t_gs2972 *handle, t_queue *event_queue)
{
/*
    uint16_t error_stat;
	if ((error_stat = spi_read_reg_16(handle->p_spi, GS2972_ERROR_STAT)) & ERROR_STAT_R_ERROR_MASK) {
        REPORT(ERROR, "ERROR_STAT is set: %x", error_stat);
    }
    //read_io_exp_tx_pin(i2c_ptr, TX_LOCKED);
*/
}


void gs2972_driver_set_rate_sel(t_gs2972 *handle, int data_rate)
{
    switch (data_rate) {
        case 270:               set_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_0);
                                clr_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_1);
                                break;
        case 1485:              clr_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_0);
                                clr_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_1);
                                break;
        case 2970:              clr_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_0);
                                set_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_1);
                                break;
        default:                REPORT(ERROR, "could not set rate sel");
    }
}
