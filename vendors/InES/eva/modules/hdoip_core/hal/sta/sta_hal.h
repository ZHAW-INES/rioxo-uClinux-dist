/*
 * sta_hal.h
 *
 *  Created on: 05.10.2010
 *      Author: alda
 */

#ifndef STA_HAL_H_
#define STA_HAL_H_

#include "stdhal.h"
#include "sta_reg.h"

#define sta_get_sta(p, m)       HOI_REG_RD((p), STA_OFF_STATUS, (m))
#define sta_get_delay(p)        HOI_RD32((p), STA_OFF_DELAY)

#define sta_has_more(p)         sta_get_sta((p), STA_STA_MORE)

#endif /* STA_HAL_H_ */
