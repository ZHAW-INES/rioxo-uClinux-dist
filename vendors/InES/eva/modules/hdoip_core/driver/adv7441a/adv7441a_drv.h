/*
 * adv7441a_drv.h
 *
 *  Created on: 26.08.2010
 *      Author: stth
 */

#ifndef ADV7441A_DRV_H_
#define ADV7441A_DRV_H_

#include "std.h"
#include "vio_drv.h"
#include "adv7441a_reg.h"
#include "adv7441a_drv_cfg.h"
#include "adv7441a_drv_i2c.h"
#include "bdt_drv.h"

/* function prototypes */
int adv7441a_drv_init(t_adv7441a* handle, t_i2c* p_i2c, t_vio* p_vio, char* edid, void* p_video_mux);
int adv7441a_handler(t_adv7441a* handle, t_queue* event_queue);
int adv7441a_irq1_handler(t_adv7441a* handle, t_queue* event_queue);
int adv7441a_irq2_handler(t_adv7441a* handle, t_queue* event_queue);
int adv7441a_sw_reset(t_adv7441a* handle);
int adv7441a_on(t_adv7441a* handle);
int adv7441a_off(t_adv7441a* handle);
int adv7441a_audio_mute(t_adv7441a* handle);
int adv7441a_audio_unmute(t_adv7441a* handle);
int adv7441a_set_edid(t_adv7441a* handle, char* edid);
int adv7441a_set_tmds_equa(t_adv7441a* handle);
int adv7441a_proc_video_in(char *buf, char **start, off_t offset, int count, int *eof, void *data);
int adv7441a_get_video_timing(t_adv7441a* handle);
int adv7441a_get_audio_timing(t_adv7441a* handle);
int adv7441a_drv_dumb_reg(t_adv7441a* handle);
int adv7441a_enable_portA(t_adv7441a* handle);
int adv7441a_disable_portA(t_adv7441a* handle);
int adv7441a_audio_fs_change(t_adv7441a* handle, uint32_t fs, uint32_t ch_cnt);
int adv7441a_get_analog_video_timing(t_adv7441a* handle);
int adv7441a_get_hdcp_status(t_adv7441a* handle);

#endif /* ADV7441A_DRV_H_ */
