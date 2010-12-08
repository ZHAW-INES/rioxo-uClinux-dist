#ifndef STDFNC_
#define STDFNC_

#include <linux/types.h>


#define UNCACHED(x)         ( (__typeof__(x))((uint32_t)(x) | 0x80000000) )
#define CACHED(x)           ( (__typeof__(x))((uint32_t)(x) & 0x7fffffff) )
#define IO(x)               ( CACHED(x) ) 

#define OFFSET(x, o)        ((__typeof__(x))((uint32_t)(x) + (uint32_t)(o)))

#define SYSTIME(x)          ( (double)(x)*SFREQ )
#define ISYSTIME(x)         ( (int32_t)SYSTIME(x) )

#define AVTIME(x)           ( (double)(x)*AVFREQ )
#define IAVTIME(x)          ( (int32_t)AVTIME(x) )

#define TIMEOUT(x)          ( ((x) - jiffies) & 0x80000000 )


static inline uint32_t swap32(uint32_t x)
{
    x = ((x>>24)&0x000000ff) |
        ((x>> 8)&0x0000ff00) |
        ((x<< 8)&0x00ff0000) |
        ((x<<24)&0xff000000);
    return x;
}

static inline uint16_t swap16(uint16_t x)
{
    x = ((x>> 8)&0x00ff) |
        ((x<< 8)&0xff00);
    return x;
}

#endif /*STDFNC_*/
