#ifndef ADV212_STR_H_
#define ADV212_STR_H_

#include "adv212_drv.h"

/** report macros
 */ 
#define ADV212_REPORT_BOOT(nr, err) { \
    REPORT(INFO, "adv212 (%d) boot: %s", (nr), adv212_str_err(err)) \
}

/** report functions
 */
char* adv212_str_err(int err);

#endif /*ADV212_STR_H_*/
