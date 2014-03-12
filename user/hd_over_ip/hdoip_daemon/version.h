#ifndef VERSION_H_
#define VERSION_H_

/* [31 .. 16] = decimal number; [15 .. 0] = after comma number */
#define VERSION_SOFTWARE        0x00040009
#define VERSION_MAJOR           ((VERSION_SOFTWARE >> 16) & 0xFFFF)
#define VERSION_MINOR           (VERSION_SOFTWARE & 0xFFFF)

// version tag (max. 50 letters)
#define VERSION_TAG             "test version"

// select one label
#define VERSION_LABEL_RIOXO
#undef  VERSION_LABEL_EMCORE
#undef  VERSION_LABEL_BLACKBOX

// check if only one label is selected
#if (((defined VERSION_LABEL_RIOXO) && (!defined VERSION_LABEL_EMCORE) && (!defined VERSION_LABEL_BLACKBOX)) || \
    ((!defined VERSION_LABEL_RIOXO) && (defined VERSION_LABEL_EMCORE) && (!defined VERSION_LABEL_BLACKBOX)) || \
    ((!defined VERSION_LABEL_RIOXO) && (!defined VERSION_LABEL_EMCORE) && (defined VERSION_LABEL_BLACKBOX)))
#else
    #error NONE OR MORE THAN ONE LABEL IS SELECTED IN FILE: "version.h"
#endif


#endif /* VERSION_H_ */
