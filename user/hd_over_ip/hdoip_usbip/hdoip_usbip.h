#ifndef _HDOIP_USBIP_H_
#define _HDOIP_USBIP_H_

extern int hdoip_usbipd_exit;
extern int debug;

#define dbg(fmt, args...)	do { if (debug) { fprintf(stdout, "D: " fmt, ##args); } } while (0)
#define dbg_cont(fmt, args...)	do { if (debug) { fprintf(stdout, fmt, ##args); } } while (0)
#define err(fmt, args...)	fprintf(stderr, "E: " fmt, ##args)
#define warn(fmt, args...)	fprintf(stderr, "W: " fmt, ##args)
#define info(fmt, args...)	fprintf(stdout, "I: " fmt, ##args)

#ifdef __GNUC__
# define __unused	__attribute__ ((unused))
#else
# define __unused
#endif /* __GNUC__ */

#endif /* _HDOIP_USBIP_H_ */
