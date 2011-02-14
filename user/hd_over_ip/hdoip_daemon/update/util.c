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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "util.h"

void *zalloc(const size_t size)
{
	void *ret = malloc(size);

	if (likely(ret))
		memset(ret, 0, size);

	return ret;
}

unsigned char hexchar_to_int(char c)
{
	if (is_digit(c))
		return c - '0';
	else if (is_lower_hex_alpha(c))
		return c - ('a' - 0xa);
	else if (is_upper_hex_alpha(c))
		return c - ('A' - 0xa);
	else
		return 0xff;
}

/*
 * Create a locked file containing the PID of the process.
 *
 * Used to prevent multiple instances of the program running at the same time.
 *
 * \param file  filename of the PID file
 * \return      file descriptor of the open PID file on success, -1 on error
 *              (errno set accordingly)
 */
int create_pid_file(const char *file)
{
	char pidbuf[16], *s;
	int fd;
	size_t bytes;
	struct flock lock;

	snprintf(pidbuf, sizeof(pidbuf), "%ld\n", (long) getpid());

	/* open non-exclusively (works on NFS) */
	fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd < 0)
		return -1;

	/* lock the PID file */
	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	if (fcntl(fd, F_SETLK, &lock) != 0) {
		if (errno == EAGAIN)
			err("PID file %s is already locked\n", file);
		else
			err("Failed to lock PID file %s: %s (%d)\n", file, strerror(errno), errno);
		goto err_out;
	}

	/* write data to file */
	bytes = strlen(pidbuf);
	s = pidbuf;
	while (bytes > 0) {
		ssize_t rc = write(fd, s, bytes);
		if (rc < 0) {
			err("Failed to write PID file: %s\n", strerror(errno));
			goto err_out;
		}

		bytes -= rc;
		s += rc;
	}

	/* make sure the data is really written to disk */
	if (fsync(fd) < 0) {
		err("Failed to fsync PID file: %s\n", strerror(errno));
		goto err_out;
	}

	return fd;

err_out:
	unlink(file);
	close(fd);
	return -1;
}
