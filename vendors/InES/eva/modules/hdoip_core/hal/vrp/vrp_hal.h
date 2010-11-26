/*
 * vrp_hal.h
 *
 *  Created on: 27.08.2010
 *      Author: alda
 */

#ifndef VRP_HAL_H_
#define VRP_HAL_H_

#include "stdhal.h"
#include "vrp_reg.h"
#include "vrp_cfg.h"

/** VIO Macros
 */
#define vrp_set_cfg(p, m)       HOI_REG_SET((p), VRP_OFF_CONFIG, (m))
#define vrp_clr_cfg(p, m)       HOI_REG_CLR((p), VRP_OFF_CONFIG, (m))
#define vrp_get_cfg(p, m)       HOI_REG_RD((p), VRP_OFF_CONFIG, (m))
#define vrp_get_sta(p, m)       HOI_REG_RD((p), VRP_OFF_STATUS, (m))

#define vrp_stop(p)				HOI_WR32((p), VRP_OFF_CONFIG, 0)
#define vrp_idle(p)				HOI_WR32((p), VRP_OFF_CONFIG, VRP_CFG_ACTIVE)
#define vrp_get_count(p)		HOI_RD32((p), VRP_OFF_COUNT)
#define vrp_get_idle(p)			vrp_get_sta((p), VRP_STA_IDLE)

static inline void vrp_set_buffer(void* p, void* start, size_t size)
{
	vrp_clr_cfg(p, ~VRP_CFG_ACTIVE);
	while(!vrp_get_idle(p));
	HOI_WR32(p, VRP_OFF_BURST, VRP_CFG_BURST);
	HOI_WR32(p, VRP_OFF_ALMOST, VRP_CFG_ALMOST);
	HOI_WR32(p, VRP_OFF_START, (uint32_t)start);
	HOI_WR32(p, VRP_OFF_STOP, (uint32_t)((uint8_t*)start+size-4));
}

static inline void vrp_do_playback(void* p, uint16_t count, bool repeat)
{
	HOI_WR32(p, VRP_OFF_COUNT, count);
	if (repeat) {
		vrp_set_cfg(p, VRP_CFG_ACTIVE|VRP_CFG_PLAYBACK|VRP_CFG_REPEAT);
	} else {
		vrp_set_cfg(p, VRP_CFG_ACTIVE|VRP_CFG_PLAYBACK);
	}
}

static inline void vrp_do_record(void* p, uint16_t count)
{
	HOI_WR32(p, VRP_OFF_COUNT, count);
	vrp_set_cfg(p, VRP_CFG_ACTIVE|VRP_CFG_RECORD);
}

#endif /* VRP_HAL_H_ */
