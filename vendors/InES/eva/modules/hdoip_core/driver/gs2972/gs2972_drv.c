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

    // enable audio output (do not toggle these signals while video is output)
    set_io_exp_tx_pin(handle->p_i2c, TX_AUDIO_1_EN);
    set_io_exp_tx_pin(handle->p_i2c, TX_AUDIO_2_EN);

    // clear rate_sel to set rate for HD video
    clr_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_0);
    clr_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_1);

    // clear reset pin
    bdt_drv_clear_reset_1(video_mux_ptr);

    // enable all ioproc functions / disable conversion to SMPTE 372M
    spi_write_reg_16(handle->p_spi, GS2972_IOPROC, IOPROC_RW_DEFAULT | IOPROC_RW_CONV_372 | IOPROC_RW_SMPTE_325M_INS);

    // set drive strength to min
    spi_write_reg_16(handle->p_spi, GS2972_DRIVE_STRENGTH, DRIVE_STRENGTH_RW_4_MA);
    spi_write_reg_16(handle->p_spi, GS2972_DRIVE_STRENGTH2, DRIVE_STRENGTH2_RW_4_MA);

    // set 24bit audio
    spi_write_reg_16(handle->p_spi, GS2972_A_CFG_AUD, A_CFG_AUD_DEFAULT | A_CFG_AUD_24_BIT);
}

void gs2972_handler(t_gs2972 *handle, t_queue *event_queue)
{

}


void gs2972_driver_set_data_rate(t_gs2972 *handle, uint32_t pfreq)
{
    // 3G - 148.5MHz
    if ((pfreq > 147000000) && (pfreq < 150000000)) {
        REPORT(INFO, "3G-SDI on input");
        clr_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_0);
        set_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_1);
    }

    // HD - 74.25MHz
    if ((pfreq > 73000000) && (pfreq < 76000000)) {
        REPORT(INFO, "HD-SDI on input");
        clr_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_0);
        clr_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_1);
    }

    // SD - 13.5MHz
    if ((pfreq > 12000000) && (pfreq < 15000000)) {
        REPORT(INFO, "SD-SDI on input");
        set_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_0);
        clr_io_exp_tx_pin(handle->p_i2c, TX_RATE_SEL_1);
    }
}


void gs2972_debug(t_gs2972 *handle)
{
    uint16_t format   = spi_read_reg_16(handle->p_spi, 0x04);
    uint16_t struc_1  = spi_read_reg_16(handle->p_spi, 0x12);
    uint16_t struc_2  = spi_read_reg_16(handle->p_spi, 0x13);
    uint16_t struc_3  = spi_read_reg_16(handle->p_spi, 0x14);
    uint16_t struc_4  = spi_read_reg_16(handle->p_spi, 0x15);
    printk("\nlines per frame: %i\nwords per line: %i\n a f: %i\n a f: %i", struc_1, struc_2, struc_3, struc_4);
    printk("\n hlock: %i, vlock: %i, std_lock: %i, interlaced: %i, vid_std: %02x\n", (format & 0x01), (format & 0x02), (format & 0x04), (format & 0x10), ((format & 0x03E0)>>5));
}



void gs2972_set_audio_channel_status(t_gs2972 *handle, uint16_t *acs)
{
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_1,  (acs[0] & 0xFF));
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_2,  ((acs[0]>>8) & 0xFF));
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_3,  acs[1]);
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_4,  acs[2]);
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_5,  acs[3]);
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_6,  acs[4]);
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_7,  acs[5]);
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_8,  acs[6]);
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_9,  acs[7]);
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_10, acs[8]);
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_11, acs[9]);
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_12, acs[10]);
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STATUS_REG_13, acs[11]);

    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_1,  (acs[0] & 0xFF));
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_2,  ((acs[0]>>8) & 0xFF));
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_3,  acs[1]);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_4,  acs[2]);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_5,  acs[3]);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_6,  acs[4]);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_7,  acs[5]);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_8,  acs[6]);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_9,  acs[7]);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_10, acs[8]);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_11, acs[9]);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_12, acs[10]);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STATUS_REG_13, acs[11]);

    // enables inserting of audio channel status
    spi_write_reg_16(handle->p_spi, GS2972_A_CHANNEL_STAT_REGEN, 0x0001);
    spi_write_reg_16(handle->p_spi, GS2972_B_CHANNEL_STAT_REGEN, 0x0001);

}
