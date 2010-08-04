/*
 * Types and structs used for the communication between userspace and
 * kernelspace.
 */

#ifndef _HDOIP_TYPES_H_
#define _HDOIP_TYPES_H_

#include <linux/types.h>

/*
 * Parameter struct to pass to hdoip_core device. Needs to be packed because it
 * is transfered between user and kernel space.
 */
struct hdoip_core_param {
	__u32 bar;
	__u16 foo;
	__u8 baz;
} __packed;

/* ioctl command definitions */

#define HDOIP_IOCTL_MAGIC	'H'

#define HDOIP_IOCRESET		_IO(HDOIP_IOCTL_MAGIC, 1)
#define HDOIP_IOCGETPARAM	_IOR(HDOIP_IOCTL_MAGIC, 2, struct hdoip_core_param)
#define HDOIP_IOCSETPARAM	_IOW(HDOIP_IOCTL_MAGIC, 3, struct hdoip_core_param)

#endif /* _HDOIP_TYPES_H_ */
