/*
 * hdoipd_fsm.h
 *
 *  Created on: 03.11.2010
 *      Author: alda
 */

#ifndef HDOIPD_FSM_H_
#define HDOIPD_FSM_H_

#include "hdoipd.h"

// daemon events
#define E_HOID_ID           (0xf0000000)
#define E_HOID_HELLO        (E_HOID_ID + 1)
#define E_HOID_UPDATE       (E_HOID_ID + 2)

const char* statestr(int state);
const char* vtbstatestr(int state);
const char* vrbstatestr(int state);

bool hdoipd_set_rsc(int state);
bool hdoipd_clr_rsc(int state);
bool hdoipd_set_state(int state);
bool hdoipd_set_vtb_state(int vtb_state);
bool hdoipd_set_vrb_state(int vrb_state);
void hdoipd_hw_reset(int rv);

bool hdoipd_goto_ready();
void hdoipd_goto_vtb();
void hdoipd_goto_vrb();
int hdoipd_vrb_setup(t_rscp_media* media, void* d);
int hdoipd_vrb_play(t_rscp_media* media, void* d);
void hdoipd_canvas(uint32_t width, uint32_t height, uint32_t fps);

int hdoipd_start_vrb_cb(t_rscp_media* media, void* d);
int hdoipd_start_vrb(void* d);
void hdoipd_task(void);
void hdoipd_set_task_start_vrb(void);

void hdoipd_set_default();
void hdoipd_registry_update();
void hdoipd_request(uint32_t* cmd, int rsp);
void hdoipd_event(uint32_t event);
void hdoipd_start();
void hdoipd_hello();
bool hdoipd_init(int drv);

static inline int hdoipd_rsc(int state)
{
    return (hdoipd.rsc_state & state);
}

static inline int hdoipd_state(int state)
{
    return (hdoipd.state & state);
}

static inline int hdoipd_tstate(int vtb_state)
{
    return (hdoipd.vtb_state & vtb_state);
}

static inline int hdoipd_rstate(int vrb_state)
{
    return (hdoipd.vrb_state & vrb_state);
}

#endif /* HDOIPD_FSM_H_ */
