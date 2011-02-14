/*
 * Copyright (c) 2009-2010, Institute of Embedded Systems,
 *                          Zurich University of Applied Sciences
 * Copyright (c) 2009-2010, Tobias Klauser <klto@zhaw.ch>
 *
 * This file is part of dionysos-update.
 *
 * dionysos-update is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dionysos-update is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ALTREMOTE_H_
#define _ALTREMOTE_H_

#include <stdbool.h>

#define ALTREMOTE_SYSFS_PATH_BASE	"/sys/devices/platform/altremote.0/"
#define ALTREMOTE_FPGA_CONFIG_ADDR_PATH	ALTREMOTE_SYSFS_PATH_BASE "fpga_config_addr"
#define ALTREMOTE_RECONFIG_PATH		ALTREMOTE_SYSFS_PATH_BASE "reconfig"

#define ALTREMOTE_CONFIG_ADDR_SIZE	6

extern bool altremote_is_loaded(void);

extern int altremote_set_fpga_config_addr(char *config_addr, size_t len);
extern int altremote_get_fpga_config_addr(char *config_addr, size_t len);

extern int altremote_trigger_reconfig(void);

#endif /* _ALTREMOTE_H_ */
