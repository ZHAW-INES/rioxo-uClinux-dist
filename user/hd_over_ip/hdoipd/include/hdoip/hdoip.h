#ifndef _HDOIP_H_
#define _HDOIP_H_

#include <stdio.h>
#include <stdint.h>

#include <hdoip/compiler.h>
#include <hdoip/endianess.h>

#define err(fmt, args...)	fprintf(stderr, "Error: " fmt, ##args)
#define warn(fmt, args...)	fprintf(stderr, "Warning: " fmt, ##args)
#define info(fmt, args...)	fprintf(stdout, fmt, ##args)
#ifdef DEBUG
# define dbg(fmt, args...)	fprintf(stdout, fmt, ##args)
#else
# define dbg(fmt, args...)
#endif /* DEBUG */

#undef max
#define max(x, y) ({			\
	typeof(x) ___x = (x);		\
	typeof(y) ___y = (y);		\
	(void) (&___x == &___y);	\
	___x > ___y ? ___x : ___y; })

#define MSEC_PER_USEC		1000
#define USEC_PER_MSEC		1000
#define SEC_TO_MSEC(x)		((x) * MSEC_PER_USEC)
#define MSEC_TO_USEC(x)		((x) * USEC_PER_MSEC)

#endif /* _HDOIP_H_ */
