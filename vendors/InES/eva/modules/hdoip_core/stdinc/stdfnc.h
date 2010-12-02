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


#endif /*STDFNC_*/
