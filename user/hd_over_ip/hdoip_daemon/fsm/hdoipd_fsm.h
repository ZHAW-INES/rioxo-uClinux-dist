/*
 * hdoipd_fsm.h
 *
 *  Created on: 03.11.2010
 *      Author: alda
 */

#ifndef HDOIPD_FSM_H_
#define HDOIPD_FSM_H_

#include "hdoipd.h"


void hdoipd_request(uint32_t* cmd, int rsp);
void hdoipd_event(uint32_t event);
void hdoipd_handler();
bool hdoipd_init(int drv);

#endif /* HDOIPD_FSM_H_ */
