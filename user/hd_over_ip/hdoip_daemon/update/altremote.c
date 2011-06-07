/*
 * wrapper around altremote driver sysfs interface
 *
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "altremote.h"
#include "util.h"

/**
 * Check whether the altremote driver is loaded by looking for its sysfs
 * entries.
 *
 * \return true if the altremote driver is loaded, false otherwise
 */
bool altremote_is_loaded(void)
{
	struct stat buf;

	if (stat(ALTREMOTE_SYSFS_PATH_BASE, &buf))
		return false;

	if (!S_ISDIR(buf.st_mode))
		return false;

	/* TODO: Check whether all needed sysfs entries are available? */

	return true;
}

/**
 */
int altremote_set_fpga_config_addr(char *config_addr, size_t len)
{
	int fd;
	int ret = 0;

	if (len != ALTREMOTE_CONFIG_ADDR_SIZE)
		return -1;

	fd = open(ALTREMOTE_FPGA_CONFIG_ADDR_PATH, O_WRONLY);
	if (fd == -1)
		return -1;

	if (write(fd, config_addr, len) != (ssize_t) len)
		ret = -1;

	close(fd);

	return ret;
}

/*
 */
int altremote_get_fpga_config_addr(char *config_addr, size_t len)
{
	int fd;

	if (len != ALTREMOTE_CONFIG_ADDR_SIZE)
		return -1;

	fd = open(ALTREMOTE_FPGA_CONFIG_ADDR_PATH, O_RDONLY);
	if (fd == -1)
		return -1;
	if (read(fd, config_addr, len) != (ssize_t) len) {
		close(fd);
		return -1;
	}

	config_addr[ALTREMOTE_CONFIG_ADDR_SIZE] = '\0';

	close(fd);

	return 0;
}

/**
 * WARNING: This function will trigger a reset using the Altera Remote Update
 * logic. Make sure to clean up as good as possible before calling.
 */
int altremote_trigger_reconfig(void)
{
	int fd = open(ALTREMOTE_RECONFIG_PATH, O_WRONLY);

	if (fd == -1)
		return -1;
	write(fd, "1", sizeof("1"));

	err("We should have been restarted by now, but something went wrong.\n"
	    "Maybe the ALTREMOTE_UPDATE logic is not available in the FPGA?\n");

	return -1;
}
