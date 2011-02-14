/*
 * Flash device access via MTD.
 *
 * Currently NAND flash devices are not fully supported.
 *
 * Copyright (c) 2009-2010, Institute of Embedded Systems,
 *                          Zurich University of Applied Sciences
 * Copyright (c) 2009-2010, Tobias Klauser <klto@zhaw.ch
 *
 * Partially based on code from the flash_eraseall and nandtest tools from the
 * mtd-utils.
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

#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <mtd/mtd-user.h>

#include "error.h"
#include "flash.h"
#include "util.h"

/**
 * Open a flash device (MTD).
 *
 * \param dev   device state object (struct flash_dev)
 * \param name  name of the device node
 * \param flags open flags, passed to open(2)
 * \return      0 on success, -1 otherwise
 */
int flash_dev_open(struct flash_dev *dev, char *name, int flags)
{
	mtd_info_t meminfo;

	dev->fd = open(name, flags);
	if (dev->fd < 0) {
		err("failed to open flash dev %s: %s\n", name, strerror(errno));
		return -1;
	}

	if (ioctl(dev->fd, MEMGETINFO, &meminfo) != 0) {
		err("unable to get flash device info for %s\n", name);
		flash_dev_close(dev);
		return -1;
	}

	dev->size = meminfo.size;
	dev->erasesize = meminfo.erasesize;
	dev->name = name;
	dev->is_empty = false;	/* We don't know about it yet */
	dev->is_nand = meminfo.type == MTD_NANDFLASH ? true : false;

	return 0;
}

void flash_dev_close(struct flash_dev *dev)
{
	if (dev->fd < 0)
		return;

	close(dev->fd);
	dev->fd = -1;
}

enum {
	BLOCK_ERASED = 0,
	BLOCK_SAME_DATA = 1
};

/**
 * Test for bad block on NAND, just returns 0 on NOR.
 *
 * \param dev
 * \param blockstart
 * \retrun           0   if block is good,
 *                   > 0 if block is bad
 *                   < 0 if ioctl failed
 */
static int check_bad_block(struct flash_dev *dev, loff_t *blockstart)
{
	if (dev->is_nand) {
		int badblock = ioctl(dev->fd, MEMGETBADBLOCK, blockstart);

		if (badblock < 0)
			err("%s: Cannot read bad block mark: %s\n", dev->name, strerror(errno));

		return badblock;
	}

	return 0;
}

/*
 * Check whether the block is empty or already contains the intended data, so we
 * don't need to erase it.
 *
 * \param dev  flash device to check on
 * \param data block data to check flash content against
 * \param size size of the block data
 * \param off  offset inside the flash device where the block is expected
 * \return     - BLOCK_SAME_DATA if the same data is already stored in the flash
 *               device at the specified offset
 *             - BLOCK_ERASED if the block was erased
 *             - negative error number on error
 */
static int flash_dev_check_or_erase_block(struct flash_dev *dev, uint8_t *data,
                                          size_t size, off_t off)
{
	uint8_t *check_buf;
	unsigned int i;
	bool blanked = true;
	bool same_data = true;
	erase_info_t erase;

	check_buf = malloc(size);
	if (!check_buf)
		return -ENOMEM;

	if ((size_t) flash_dev_read(dev, check_buf, size, off) != size) {
		err("%s: Reading before erase failed\n", dev->name);
		free(check_buf);
		return -EREAD;
	}

	/* Check whether the block is empty */
	for (i = 0; i < size; i++) {
		if (check_buf[i] != 0xff) {
			blanked = false;
			break;
		}
	}

	if (blanked) {
		free(check_buf);
		return BLOCK_ERASED;
	}

	/* Check whether the block already contains the data we want to write */
	for (i = 0; i < size; i++) {
		if (check_buf[i] != data[i]) {
			same_data = false;
			break;
		}
	}

	free(check_buf);

	/* Block already contains the data we want to write */
	if (same_data)
		return BLOCK_SAME_DATA;

	erase.length = dev->erasesize;
	for (erase.start = off; erase.start < (off + size); erase.start += dev->erasesize) {
		dbg("Erasing @ %08x (%d bytes)\n", erase.start, erase.length);
		if (ioctl(dev->fd, MEMERASE, &erase))
			return -EERASE;
	}

	return BLOCK_ERASED;
}

/**
 *
 * It is assumed that block length is the same as the flash device's erase size.
 *
 * \param dev  flash device to write to
 * \param data data to write
 * \param size size of the data buffer
 * \param off  offset to write at
 * \return     number of bytes written, negative error number on error
 */
ssize_t flash_dev_write_block(struct flash_dev *dev, uint8_t *data,
                              size_t size, off_t off)
{
	ssize_t len;

	/*
	 * If the whole device was erased previously, there is no need to check
	 * or erase the block.
	 */
	if (!dev->is_empty) {
		int ret = flash_dev_check_or_erase_block(dev, data, size, off);

		/* Block data is already the same in flash */
		if (ret == BLOCK_SAME_DATA)
			return size;
		else if (ret < 0) /* Error */
			return ret;
	}

	dev->is_empty = false;
	len = pwrite(dev->fd, data, size, off);
	if (len < 0) {
		/* Probably a bad block. XXX: mark it as such? */
		return -EBADBLOCK;
	} else if ((size_t) len != size) {
		return -ESIZE;
	}

	return size;
}

/**
 * Read data from flash at an offset into a provided buffer.
 *
 * TODO: Fully implement NAND support.
 *
 * Partially based on flash_read_buf from fw_env.c from the u-boot-tools.
 *
 * \param dev   flash device to read from
 * \param off   offset into flash device
 * \param buf   read buffer
 * \param count size of the read buffer
 * \return      number of bytes read, -1 on error
 */
ssize_t flash_dev_read(struct flash_dev *dev, uint8_t *buf, size_t count, off_t off)
{
	size_t blocklen;
	size_t processed = 0;
	ssize_t readlen = count;
	off_t top_of_range;
	off_t block_seek;
	loff_t blockstart;
	int rc;

	/*
	 * Start of the first block to be read, relies on the fact, that
	 * erase sector size is always a power of 2
	 */
	blockstart = off & ~(dev->erasesize - 1);

	/* Offset inside a block */
	block_seek = off - blockstart;

	blocklen = 0;
	top_of_range = off + count;

	/* This only runs once on NOR flash */
	while (processed < count) {
		rc = check_bad_block(dev, &blockstart);
		if (rc < 0)	/* block test failed */
			return -1;

		if (blockstart + block_seek + readlen > top_of_range) {
			/* End of range is reached */
			err("Too few good blocks within range\n");
			return -1;
		}

		if (rc) {	/* block is bad */
			blockstart += blocklen;
			continue;
		}

		/*
		 * If a block is bad, we retry in the next block at the same
		 * offset.
		 */
		rc = pread(dev->fd, buf + processed, readlen, blockstart + block_seek);
		if (rc != readlen) {
			err("%s: Read error: %s\n", dev->name, strerror(errno));
			return -1;
		}

		processed += readlen;
		readlen = min(blocklen, count - processed);
		block_seek = 0;
		blockstart += blocklen;
	}

	return processed;
}

/**
 * Fully erase a flash device.
 *
 * \param dev the flash device to fully erase
 * \return    0 on success, negative error number on error
 */
int flash_dev_eraseall(struct flash_dev *dev)
{
	erase_info_t erase;
	uint32_t erasesize = dev->erasesize;
	bool do_check_bad_blocks = true;

	for (erase.start = 0; erase.start < dev->size; erase.start += erasesize) {
		if (do_check_bad_blocks) {
			loff_t offset = erase.start;
			int ret = check_bad_block(dev, &offset);

			if (ret > 0) {
				printf("Skipping bad block at 0x%08x\n", erase.start);
				continue;
			} else if (ret < 0) {
				if (ret == EOPNOTSUPP) {
					do_check_bad_blocks = false;
					if (dev->is_nand) {
						err("%s: Bad block check not available\n", dev->name);
						return 0;
					}
				}
			}
		}

		erase.length = erasesize;
		if (ioctl(dev->fd, MEMERASE, &erase)) {
			err("%s: MTD Erase failure at 0x%08x: %s\n", dev->name, erase.start, strerror(errno));
			return -1;
		}
	}

	dev->is_empty = true;

	return 0;
}
