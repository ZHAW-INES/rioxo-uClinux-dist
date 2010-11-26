#ifndef STDFNC_
#define STDFNC_

#include <std.h>

#define UNCACHED(x)         ( (__typeof__(x))((uint32_t)(x) | 0x80000000) )
#define CACHED(x)           ( (__typeof__(x))((uint32_t)(x) & 0x7fffffff) )
#define IO(x)               ( CACHED(x) ) 

#define OFFSET(x, o)        ((__typeof__(x))((uint32_t)(x) + (uint32_t)(o)))

#define SYSTIME(x)          ( (double)(x)*SFREQ )
#define ISYSTIME(x)         ( (int32_t)SYSTIME(x) )

#define AVTIME(x)           ( (double)(x)*AVFREQ )
#define IAVTIME(x)          ( (int32_t)AVTIME(x) )

#define TIMEOUT(x)          ( ((x) - jiffies) & 0x80000000 )

/*
// Host to Network
// Network to Host
#define lswap(x)            ((((x >> 24) & 0x000000ff)) | \
                             (((x >>  8) & 0x0000ff00)) | \
                             (((x) & 0x0000ff00) <<  8) | \
                             (((x) & 0x000000ff) << 24))
#define htonl(l)            (lswap(l))
#define ntohl(l)            (lswap(l))
#define htons(s)            ((((s) >> 8) & 0xff) | \
                             (((s) << 8) & 0xff00))
#define ntohs(s)            (htons(s))
*/

#endif /*STDFNC_*/
