/*
 * gs2971_drv.c
 *
 *  Created on: 02.09.2011
 *      Author: buan
 */

#include "gs2971_drv.h"
#include "spi_drv.h"
#include "io_expander_rx.h"
#include "gs2971_video_format.h"

void gs2971_driver_init(t_gs2971 *handle, void *spi_ptr, void *i2c_ptr)
{
    handle->p_spi = spi_ptr;
    handle->p_i2c = i2c_ptr;

    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| GS2971-Driver: Initialize SDI-RX                 |");
    REPORT(INFO, "+--------------------------------------------------+");

    // disable all led
    set_io_exp_rx_pin(handle->p_i2c, RX_LED_1);
    set_io_exp_rx_pin(handle->p_i2c, RX_LED_2);
    set_io_exp_rx_pin(handle->p_i2c, RX_LED_3);
    set_io_exp_rx_pin(handle->p_i2c, RX_LED_4);

    // power up device
    clr_io_exp_rx_pin(handle->p_i2c, RX_STANDBY);

    // enable data processing
    set_io_exp_rx_pin(handle->p_i2c, RX_IOPROC_EN);

    // enable extracting audio data
    set_io_exp_rx_pin(handle->p_i2c, RX_AUDIO_EN);

    // reclock output signal to loopback
    set_io_exp_rx_pin(handle->p_i2c, RX_RC_BYP);

    // set slew rate to sd-video (for loopback)
    gs2971_driver_set_slew_rate(handle, 270);

    // disable loopback
    gs2971_driver_configure_loopback(handle, LOOP_OFF);

        // enable auto mode? (default)
        //(check smpte bypass if input is smpte)
}

void gs2971_handler(t_gs2971 *handle, t_queue *event_queue)
{
    int format_code = (spi_read_reg_16(handle->p_spi, GS2971_DATA_FORMAT_DS1) & DATA_FORMAT_DS1_VD_STD_MASK) >> DATA_FORMAT_DS1_VD_STD_SHIFT;
    report_incoming_video(handle, format_code);

    //read_io_exp_rx_pin(handle->p_i2c, RX_STAT_3);
    //read_io_exp_rx_pin(handle->p_i2c, RX_STAT_4);
    //read_io_exp_rx_pin(handle->p_i2c, RX_STAT_5);
    //read_io_exp_rx_pin(handle->p_i2c, RX_SMPTE_BYPASS);
    //read_io_exp_rx_pin(handle->p_i2c, RX_DVB_ASI);
}

void gs2971_driver_set_slew_rate(t_gs2971 *handle, int data_rate)
{
    switch (data_rate) {
        case 270:               set_io_exp_rx_pin(handle->p_i2c, RX_LOOP_SD);
                                break;
        case 1485:              
        case 2970:              clr_io_exp_rx_pin(handle->p_i2c, RX_LOOP_SD);
                                break;
        default:                REPORT(ERROR, "could not set slew rate");
    }
}

void gs2971_driver_configure_loopback(t_gs2971 *handle, int enable)
{
    if (enable == LOOP_ON) {
        set_io_exp_rx_pin(handle->p_i2c, RX_SDO_EN);
    } else {
        clr_io_exp_rx_pin(handle->p_i2c, RX_SDO_EN);
    }
}

void report_incoming_video(t_gs2971 *handle, int format_code)
{
    int i;

    for (i=0; i<42; i++) {
        if (sdi_video_format_code[i][0] == format_code) {
            REPORT(INFO, "detected resolution = %dx%d@%dHz, color = %d", sdi_video_format_code[i][1], sdi_video_format_code[i][2], sdi_video_format_code[i][3], sdi_video_format_code[i][4]);
        }
    }
}
