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
#include "queue.h"
#include "bdt_drv.h"

void gs2971_driver_init(t_gs2971 *handle, void *spi_ptr, void *i2c_ptr, void *video_mux_ptr)
{
    handle->p_spi = spi_ptr;
    handle->p_i2c = i2c_ptr;
    handle->video_status = false;
    handle->no_phase_info = false;
    handle->delay_queue_input = 0;

    REPORT(INFO, "+--------------------------------------------------+");
    REPORT(INFO, "| GS2971-Driver: Initialize SDI-RX                 |");
    REPORT(INFO, "+--------------------------------------------------+");

    // initialize i2c port-expander
    init_io_exp_rx(handle->p_i2c);

    // disable all led
    set_io_exp_rx_pin(handle->p_i2c, RX_LED_1); //led green input
    set_io_exp_rx_pin(handle->p_i2c, RX_LED_2); //led red input
    set_io_exp_rx_pin(handle->p_i2c, RX_LED_3); //led green loopback
    set_io_exp_rx_pin(handle->p_i2c, RX_LED_4); //led red loopback

    // power up device
    clr_io_exp_rx_pin(handle->p_i2c, RX_STANDBY);

    // enable data processing
    set_io_exp_rx_pin(handle->p_i2c, RX_IOPROC_EN);

    // enable extracting audio data
    set_io_exp_rx_pin(handle->p_i2c, RX_AUDIO_EN);

    // do not reclock output signal to loopback
    clr_io_exp_rx_pin(handle->p_i2c, RX_RC_BYP);

    // clear reset pin
    bdt_drv_clear_reset_0(video_mux_ptr);

    // set slew rate to sd-video (for loopback)
    gs2971_driver_set_slew_rate(handle, 270);

    // disable loopback
    gs2971_driver_configure_loopback(handle, LOOP_OFF);

    // select automatic detection of video format    
    spi_write_reg_16(handle->p_spi, GS2971_RATE_SEL, RATE_SEL_AUTO);

    // set lowest drive strength
    spi_write_reg_16(handle->p_spi, GS2971_IO_DRIVE_STRENGTH, IO_DRIVE_STRENGTH_4_MA);

    // invert hsync and vsync timing (low active)
    spi_write_reg_16(handle->p_spi, GS2971_TIM_861_CFG, TIM_861_CFG_HSYNC_INVERT | TIM_861_CFG_VSYNC_INVERT);

    // ------AUDIO-CONFIG------

    // output always I2S-audio 24bit (SD)
    spi_write_reg_16(handle->p_spi, GS2971_A_CFG_OUTPUT, A_CFG_OUTPUT_BIT_LENGTH_24 | A_CFG_OUTPUT_MODE_I2S);

    // output always I2S-audio 24bit (HD/3G)
    spi_write_reg_16(handle->p_spi, GS2971_B_CFG_AUD, B_CFG_AUD_DEFAULT_CHANNEL_SELECT | B_CFG_AUD_MODE_I2S | B_CFG_AUD_BIT_LENGTH_24);

    // unmute audio channels 1-8 (HD/3G)
    spi_write_reg_16(handle->p_spi, GS2971_B_CH_MUTE, B_CH_MUTE_OFF);
}

void gs2971_hd_3g_audio_handler(t_gs2971 *handle)
{
    // TODO: use interrupt for no_phasea_data
    bool no_phase_info, m;
    uint16_t aud_det;

    no_phase_info = (bool) (spi_read_reg_16(handle->p_spi, GS2971_B_CFG_AUD_3) & B_CFG_AUD_3_NO_PHASEA_DATA);
    aud_det = spi_read_reg_16(handle->p_spi, GS2971_B_AUD_DET1);

    if (handle->no_phase_info != no_phase_info){
        if (no_phase_info) {
            REPORT(INFO, "Incorrect or no audio phase information");
            m = (bool) (spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_M);
            if (m) {
                spi_write_reg_16(handle->p_spi, GS2971_B_CFG_AUD_2, B_CFG_AUD_2_IGNORE_PHASE | B_CFG_AUD_2_FORCE_M | B_CFG_AUD_2_FORCE_MEQ1001);
            } else {
                spi_write_reg_16(handle->p_spi, GS2971_B_CFG_AUD_2, B_CFG_AUD_2_IGNORE_PHASE | B_CFG_AUD_2_FORCE_M);
            }
        } else {
            spi_write_reg_16(handle->p_spi, GS2971_B_CFG_AUD_2, B_CFG_AUD_2_DEFAULT);
        }
    }

    handle->no_phase_info = no_phase_info;
}

void gs2971_handler(t_gs2971 *handle, t_queue *event_queue)
{
    bool pll_locked, smpte, std_lock, video_status;
    uint16_t video_format = (spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_RATE_SEL_READBACK_MASK) >> RASTER_STRUC_4_RATE_SEL_READBACK_SHIFT;

    pll_locked = (bool) read_io_exp_rx_pin(handle->p_i2c, RX_STAT_3);
    smpte = (bool) read_io_exp_rx_pin(handle->p_i2c, RX_SMPTE_BYPASS);
    std_lock = (bool) (spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_STD_LOCK);

    video_status = pll_locked & smpte & std_lock;

    if (video_status != handle->video_status) {
        if (video_status) {
            handle->input_active = true;
            handle->delay_queue_input = 0;
        } else {
            handle->input_active = false;
            handle->delay_queue_input = 0;
        }
    }

    if ((video_format == RATE_SEL_READBACK_HD) || (video_format == RATE_SEL_READBACK_3G)) {
        gs2971_hd_3g_audio_handler(handle);
    }
    handle->video_status = video_status;

    // prevent start/stop if "input active bit" toggles at higher frequency
    if (handle->input_active == true) {
        if (handle->delay_queue_input != RECOGNIZE_INPUT_DELAY) {
            handle->delay_queue_input++;
        }
        if (handle->delay_queue_input == (RECOGNIZE_INPUT_DELAY - 1)) {
            queue_put(event_queue, E_GS2971_VIDEO_ON);
            // enable loopback
            gs2971_driver_configure_loopback(handle, LOOP_ON);
            queue_put(event_queue, E_GS2971_LOOP_ON);
        }
    } else {
        if (handle->delay_queue_input != RECOGNIZE_INPUT_DELAY) {
            handle->delay_queue_input++;
        }
        if (handle->delay_queue_input == (RECOGNIZE_INPUT_DELAY - 1)) {
            queue_put(event_queue, E_GS2971_VIDEO_OFF);
            // disable loopback
            gs2971_driver_configure_loopback(handle, LOOP_OFF);
            queue_put(event_queue, E_GS2971_LOOP_OFF);
        }
    }
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
    int data_rate;
    uint16_t video_format;
    if (enable == LOOP_ON) {
        video_format = (spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_RATE_SEL_READBACK_MASK) >> RASTER_STRUC_4_RATE_SEL_READBACK_SHIFT;
        switch (video_format) {
            case RATE_SEL_READBACK_SD_1:    data_rate = 270;
                                            break;
            case RATE_SEL_READBACK_SD_2:    data_rate = 270;
                                            break;
            case RATE_SEL_READBACK_HD:      data_rate = 1485;
                                            break;
            case RATE_SEL_READBACK_3G:      data_rate = 2970;
                                            break;
            default:                        data_rate = 0;
        }
        gs2971_driver_set_slew_rate(handle, data_rate);
        set_io_exp_rx_pin(handle->p_i2c, RX_SDO_EN);
    } else {
        clr_io_exp_rx_pin(handle->p_i2c, RX_SDO_EN);
    }
}

void gs2971_report_incoming_video(t_gs2971 *handle, int format_code)
{
    int i;

    for (i=0; i<42; i++) {
        if (sdi_video_format_code[i][0] == format_code) {
            REPORT(INFO, "detected resolution = %dx%d@%dHz, color = %d", sdi_video_format_code[i][1], sdi_video_format_code[i][2], sdi_video_format_code[i][3], sdi_video_format_code[i][4]);
        }
    }
}

void gs2971_get_video_timing_info(t_gs2971 *handle)
{
    uint32_t width, total_width, height, total_height, interlaced, rate_sel;

    if (spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_STD_LOCK) {
        width =         spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_1) & RASTER_STRUC_1_WORDS_PER_AVTLINE_MASK;
        total_width =   spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_2) & RASTER_STRUC_2_WORDS_PER_LINE_MASK;
        height =        spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_ACTLINE_PER_FIELD_MASK;
        total_height =  spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_3) & RASTER_STRUC_3_LINES_PER_FRAME_MASK;
        interlaced =    spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_INT_PROG;
        rate_sel =      (spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_RATE_SEL_READBACK_MASK) >> RASTER_STRUC_4_RATE_SEL_READBACK_SHIFT;

        REPORT(INFO, "resolution(%d/%d)x(%d/%d) \ninterlaced: %x \nrate_sel: %x", width, total_width, height, total_height, interlaced, rate_sel);

    }
}

void gs2971_debug(t_gs2971 *handle)
{

}

bool gs2971_get_interlaced(t_gs2971 *handle)
{
    return (bool) ((spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_INT_PROG) >> RASTER_STRUC_4_INT_PROG_SHIFT); // true = interlaced
}

int gs2971_get_video_timing(t_gs2971 *handle, t_video_timing *measured_timing)
{
    int i, height, total_height, width, total_width, interlaced;

    interlaced = (spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_INT_PROG) >> RASTER_STRUC_4_INT_PROG_SHIFT;  // 1 = interlaced timing detected

    handle->vid_st.interlaced = interlaced;  // 1 = interlaced timing detected
    handle->vid_st.vsync_pol  = 0;  // 1 = high active
    handle->vid_st.hsync_pol  = 0;  // 1 = high active
    handle->vid_st.field_pol  = 1;  // 1 = high is odd field

    // timing is only needed when interlaced
    if ((interlaced) == 0) {
        return 1;
    }

    // get timing
    // internally measured timing is for 576i and 480i incorrect -> use timing of vio (measured_timing)
    // width =         spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_1) & RASTER_STRUC_1_WORDS_PER_AVTLINE_MASK;
    // total_width =   spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_2) & RASTER_STRUC_2_WORDS_PER_LINE_MASK;
    // height =        spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_4) & RASTER_STRUC_4_ACTLINE_PER_FIELD_MASK;
    // total_height =  spi_read_reg_16(handle->p_spi, GS2971_RASTER_STRUC_3) & RASTER_STRUC_3_LINES_PER_FRAME_MASK;

    width =         measured_timing->width;
    total_width =   measured_timing->width + measured_timing->hfront + measured_timing->hpulse + measured_timing->hback;
    height =        measured_timing->height;
    total_height =  (measured_timing->height + measured_timing->vfront + measured_timing->vpulse + measured_timing->vback) * 2;

    // search timing in table
    for (i=0;i<INTERLACE_TIMING_TABLE_LENGTH;i++) {
        if ((sdi_video_interlaced_timing[i][VIDEO_FORMAT_H_WIDTH]) == width) {
            if ((sdi_video_interlaced_timing[i][VIDEO_FORMAT_H_TOTAL_WIDTH]) == total_width) {
                if ((sdi_video_interlaced_timing[i][VIDEO_FORMAT_F0_HEIGHT]) == height) {
                    if ((sdi_video_interlaced_timing[i][VIDEO_FORMAT_F0_TOTAL_HEIGHT] == total_height) || (sdi_video_interlaced_timing[i][VIDEO_FORMAT_F1_TOTAL_HEIGHT] == total_height)) {
                        break;
                    }
                }
            }
        }
    }

    // write found timing in registers
    // Horizontal
    handle->vid_st.h_total_line_width      = sdi_video_interlaced_timing[i][VIDEO_FORMAT_H_TOTAL_WIDTH];
    handle->vid_st.h_line_width            = sdi_video_interlaced_timing[i][VIDEO_FORMAT_H_WIDTH];
    handle->vid_st.h_front_porch_width     = sdi_video_interlaced_timing[i][VIDEO_FORMAT_H_FRONT];
    handle->vid_st.h_sync_width            = sdi_video_interlaced_timing[i][VIDEO_FORMAT_H_PULSE];
    handle->vid_st.h_back_porch_width      = sdi_video_interlaced_timing[i][VIDEO_FORMAT_H_BACK];

    // Vertical Field 0
    handle->vid_st.f1_total_height         = sdi_video_interlaced_timing[i][VIDEO_FORMAT_F0_TOTAL_HEIGHT];
    handle->vid_st.f1_height               = sdi_video_interlaced_timing[i][VIDEO_FORMAT_F0_HEIGHT];
    handle->vid_st.f1_front_porch_width    = sdi_video_interlaced_timing[i][VIDEO_FORMAT_F0_FRONT];
    handle->vid_st.f1_vs_pulse_width       = sdi_video_interlaced_timing[i][VIDEO_FORMAT_F0_PULSE];
    handle->vid_st.f1_back_porch_width     = sdi_video_interlaced_timing[i][VIDEO_FORMAT_F0_BACK];

    // Vertical Field 1
    handle->vid_st.f0_total_height         = sdi_video_interlaced_timing[i][VIDEO_FORMAT_F1_TOTAL_HEIGHT];
    handle->vid_st.f0_height               = sdi_video_interlaced_timing[i][VIDEO_FORMAT_F1_HEIGHT];
    handle->vid_st.f0_front_porch_width    = sdi_video_interlaced_timing[i][VIDEO_FORMAT_F1_FRONT];
    handle->vid_st.f0_vs_pulse_width       = sdi_video_interlaced_timing[i][VIDEO_FORMAT_F1_PULSE];
    handle->vid_st.f0_back_porch_width     = sdi_video_interlaced_timing[i][VIDEO_FORMAT_F1_BACK];

    return 1;
}

int gs2971_get_audio_config(t_gs2971 *handle)
{
    handle->aud_st.fs = 48000;
    handle->aud_st.channel_cnt = 8;
    handle->aud_st.bit_width = 24;
    handle->aud_st.mute= 0;

    REPORT(INFO, "[SDI IN] audio sampling rate has changed\n");
    REPORT(INFO, "[SDI IN] Audio fs = %d Hz", handle->aud_st.fs);
    REPORT(INFO, "[SDI IN] Audio width = %d Bit", handle->aud_st.bit_width);
    REPORT(INFO, "[SDI IN] Audio count = %d", handle->aud_st.channel_cnt);
    REPORT(INFO, "[SDI IN] Audio mute = %d\n", handle->aud_st.mute);

    return 1;
}
