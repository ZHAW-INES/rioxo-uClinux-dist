#ifndef _ENDIANESS_H_
#define _ENDIANESS_H_

#include <inttypes.h>
#include <sys/types.h>

#undef HDOIP_ENDIAN_BIG
#undef HDOIP_ENDIAN_LITTLE

/* Linux */
#ifdef __BYTE_ORDER
# if __BYTE_ORDER == __BIG_ENDIAN
#  define HDOIP_ENDIAN_BIG
# elif __BYTE_ORDER == __LITTLE_ENDIAN
#  define HDOIP_ENDIAN_LITTLE
# endif
#else
# error "OS other than Linux currently not supported"
#endif

#endif /* _ENDIANESS_H_ */
