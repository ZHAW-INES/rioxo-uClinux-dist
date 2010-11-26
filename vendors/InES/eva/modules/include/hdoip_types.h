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
	__u8 i2c_reg_map; 
	__u8 i2c_addr;
	__u8 i2c_value;
} __attribute__((packed));
/* ioctl command definitions */

#define HDOIP_IOCTL_MAGIC	'H'

#define HDOIP_ADV7441A_GET		_IOWR(HDOIP_IOCTL_MAGIC, 1, struct hdoip_core_param)
#define HDOIP_ADV7441A_SET		_IOW(HDOIP_IOCTL_MAGIC, 2, struct hdoip_core_param)
#define HDOIP_ADV7441A_INIT		_IO(HDOIP_IOCTL_MAGIC, 3)
#define HDOIP_ADV7441A_RESET		_IO(HDOIP_IOCTL_MAGIC, 4)
#define HDOIP_ADV7441A_ON		_IO(HDOIP_IOCTL_MAGIC, 5)
#define HDOIP_ADV7441A_OFF		_IO(HDOIP_IOCTL_MAGIC, 6)
#define HDOIP_ADV7441A_AUDIO_MUTE	_IO(HDOIP_IOCTL_MAGIC, 7)
#define HDOIP_ADV7441A_AUDIO_UNMUTE	_IO(HDOIP_IOCTL_MAGIC, 8)
#define HDOIP_ADV7441A_HANDLER		_IO(HDOIP_IOCTL_MAGIC, 9)
/*
#define HDOIP_IOCRESET		_IO(HDOIP_IOCTL_MAGIC, 1)
#define HDOIP_IOCGETPARAM	_IOR(HDOIP_IOCTL_MAGIC, 2, struct hdoip_core_param)
#define HDOIP_IOCSETPARAM	_IOW(HDOIP_IOCTL_MAGIC, 3, struct hdoip_core_param)
*/
#endif /* _HDOIP_TYPES_H_ */
