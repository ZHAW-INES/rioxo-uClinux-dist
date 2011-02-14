/*
 * Copyright (c) 2009, Institute of Embedded Systems,
 *                     Zurich University of Applied Sciences
 * Copyright (c) 2009, Tobias Klauser <klto@zhaw.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FLASH_H_
#define _FLASH_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct flash_dev {
	int fd;
	uint32_t size;		/**< device size */
	uint32_t erasesize;	/**< erase block size */
	char *name;		/**< device name */
	bool is_empty;		/**< true if the device is completely erased */
	bool is_nand;		/**< true if this is a NAND flash device */
};

extern int flash_dev_open(struct flash_dev *dev, char *name, int flags);
extern void flash_dev_close(struct flash_dev *dev);
extern ssize_t flash_dev_write_block(struct flash_dev *dev, uint8_t *data,
                                     size_t size, off_t off);
extern ssize_t flash_dev_read(struct flash_dev *dev, uint8_t *buf, size_t count,
                              off_t off);
extern int flash_dev_eraseall(struct flash_dev *dev);

#endif /* _FLASH_H_ */
