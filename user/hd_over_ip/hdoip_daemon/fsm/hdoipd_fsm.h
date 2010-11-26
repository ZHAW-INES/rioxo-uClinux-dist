/*
 * hdoipd_fsm.h
 *
 *  Created on: 03.11.2010
 *      Author: alda
 */

#ifndef HDOIPD_FSM_H_
#define HDOIPD_FSM_H_

#include "hdoipd.h"


void hdoipd_request(t_hdoipd* handle, uint32_t* cmd, int rsp);
void hdoipd_event(t_hdoipd* handle, uint32_t event);
void hdoipd_handler(t_hdoipd* handle);

#endif /* HDOIPD_FSM_H_ */
