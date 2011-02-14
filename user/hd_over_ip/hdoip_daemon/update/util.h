/*
 * Copyright (c) 2009, Institute of Embedded Systems,
 *                     Zurich University of Applied Sciences
 * Copyright (c) 2009, Tobias Klauser <klto@zhaw.ch>
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

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <unistd.h>

#include "compiler.h"
#include "error.h"

#define PID_INVAL	-1

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))

/* min macro with type checking, taken from linux/kernel.h */
#ifndef min
# define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })
#endif

/* check whether c is a digit */
#define is_digit(c)		((c) >= '0' && (c) <= '9')
/* check whether c is a lowercase letter */
#define is_lower_hex_alpha(c)	((c) >= 'a' && (c) <= 'f')
/* check whether c is a uppercase letter */
#define is_upper_hex_alpha(c)	((c) >= 'A' && (c) <= 'F')

#define err(fmt, args...)	fprintf(stderr, "Error: " fmt, ##args)
#define warn(fmt, args...)	fprintf(stderr, "Warning: " fmt, ##args)
#ifdef DEBUG
# define dbg(fmt, args...)	fprintf(stdout, fmt, ##args)
#else
# define dbg(fmt, args...)
#endif

extern void *zalloc(const size_t size);
extern unsigned char hexchar_to_int(char c);
extern int create_pid_file(const char *file);

#endif /* _UTIL_H_ */
