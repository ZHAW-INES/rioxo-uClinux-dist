#ifndef DEBUG_H_
#define DEBUG_H_

#include <std.h>

// Define HOIDEBUG for Additional Debug output

// Use: REPORT( {ERROR|WARNING|INFO} , <formated string> );
//      ASSERT( <condition> , <formated string> );
//      MEMDUMP( <pointer> , <size in byte> , <formated string> );

#define	__HOI_DEBUG__

#define ERROR 2
#define WARNING 1
#define INFO 0

// used for formated output: REPORT(INFO, "%s", DBGSTA(x))
#define DBGSTA(b)           ((b) ? "on" : "off")

#ifdef __HOI_DEBUG__
    #define REPORT(y, ...) { if (y == ERROR) {printk(KERN_ALERT "Error %s (%d):", __FILE__, __LINE__);} \
                             printk(KERN_ALERT __VA_ARGS__);}
    #define ASSERT(y, ...) { if (!(y)) REPORT(ERROR, __VA_ARGS__); }
    #define DBGIF(c, ...) { if (c) { __VA_ARGS__ } }
    #define DBG(...) { __VA_ARGS__ }
    #define PTR(p)
/*   #define PTR(p) {                                \
        if (!(p)) {                                 \
            REPORT(ERROR, "null pointer: " #p);     \
            while(1);                               \
        }                                           \
    }
*/
#else
    #define REPORT(y, ...) {}
    #define ASSERT(y, ...) {}
    #define DBGIF(c, ...) {}
    #define DBG(...) {}
    #define PTR(...) {}
#endif

#endif /*DEBUG_H_*/
