/*
 * vrp_reg.h
 *
 *  Created on: 27.08.2010
 *      Author: alda
 */

#ifndef VRP_REG_H_
#define VRP_REG_H_

/** Register byte offset
 */
#define VRP_OFF_CONFIG				(0x00)
#define VRP_OFF_STATUS				(0x04)
#define VRP_OFF_START 				(0x08)
#define VRP_OFF_STOP  				(0x0c)
#define VRP_OFF_COUNT 				(0x10)
#define VRP_OFF_BURST 				(0x14)
#define VRP_OFF_ALMOST				(0x18)

// VRP_OFF_CONFIG
#define VRP_CFG_ACTIVE				(1<<0)
#define VRP_CFG_RECORD				(1<<1)
#define VRP_CFG_PLAYBACK			(1<<2)
#define VRP_CFG_REPEAT				(1<<3)

// VRP_OFF_STATUS
#define VRP_STA_IDLE				(1<<0)

#endif /* VRP_REG_H_ */
