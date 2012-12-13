/*
 * gs2971_drv.h
 *
 *  Created on: 02.09.2011
 *      Author: buan
 */

#ifndef GS2971_DRV_H_
#define GS2971_DRV_H_

#include "gs2971_reg.h"
#include "spi_drv.h"
#include "adv7441a_drv_vid_std.h"

#define LOOP_ON     0x0001
#define LOOP_OFF    0x0000

#define RECOGNIZE_INPUT_DELAY   10

typedef struct {
    void                *p_spi;
    void                *p_i2c;
    void                *video_mux_ptr;
    bool                video_status;
    bool                no_phase_info;
    uint32_t            delay_queue_input;
    bool                input_active;
    t_adv7441a_vid_st   vid_st;
    t_adv7441a_aud_st   aud_st;
} t_gs2971;

void gs2971_driver_init(t_gs2971 *handle, void *spi_ptr, void *i2c_ptr, void *video_mux_ptr);
void gs2971_handler(t_gs2971 *handle, t_queue *event_queue);
void gs2971_driver_set_slew_rate(t_gs2971 *handle, int data_rate);
void gs2971_driver_configure_loopback(t_gs2971 *handle, int enable);
void gs2971_report_incoming_video(t_gs2971 *handle, int format_code);
void gs2971_get_video_timing_info(t_gs2971 *handle);
void gs2971_hd_3g_audio_handler(t_gs2971 *handle);
void gs2971_debug(t_gs2971 *handle);
bool gs2971_get_interlaced(t_gs2971 *handle);
int  gs2971_get_video_timing(t_gs2971 *handle, t_video_timing *measured_timing);
int  gs2971_get_audio_config(t_gs2971 *handle);
#endif /* GS2971_DRV_H_ */
