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

#ifndef _COMPILER_H_
#define _COMPILER_H_

#ifdef __GNUC__
# define __packed	__attribute__ ((packed))
# define __unused	__attribute__ ((unused))
# define likely(x)	__builtin_expect(!!(x), 1)
# define unlikely(x)	__builtin_expect(!!(x), 0)
#else
# define __packed
# define __unused
# define likely(x)	(x)
# define unlikely(x)	(x)
#endif /* __GNUC__ */

#endif /* _COMPILER_H_ */
