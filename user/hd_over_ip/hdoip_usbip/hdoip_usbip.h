#ifndef _HDOIP_USBIP_H_
#define _HDOIP_USBIP_H_

#include <sys/time.h>

extern int hdoip_usbipd_exit;
extern int debug;

#define __log(fd, fmt, args...)	do {	\
	struct timeval now;		\
	gettimeofday(&now, NULL);	\
	fprintf(fd, "[%ld.%06ld] " fmt, now.tv_sec, now.tv_usec, ##args); \
} while (0)

#define dbg(fmt, args...)	do { if (debug) { __log(stdout, "D: " fmt, ##args); } } while (0)
#define dbg_cont(fmt, args...)	do { if (debug) { __log(stdout, fmt, ##args); } } while (0)
#define err(fmt, args...)	__log(stderr, "E: " fmt, ##args)
#define warn(fmt, args...)	__log(stderr, "W: " fmt, ##args)
#define info(fmt, args...)	__log(stdout, "I: " fmt, ##args)

#ifdef __GNUC__
# define __unused	__attribute__ ((unused))
#else
# define __unused
#endif /* __GNUC__ */

#endif /* _HDOIP_USBIP_H_ */
