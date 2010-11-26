
#ifndef EXT_RESET_PIO_H
#define EXT_RESET_PIO_H

#include "stdhal.h"

#define EXT_RESET_ETH2			(0x01)
#define EXT_RESET_NIOS2			(0x02)
#define EXT_RESET_HDMI_RX		(0x04)
#define EXT_RESET_J2K_CODEC		(0x08)

/** Macros
 */
#define ext_reset_write32(p, v) HOI_WR32((p), 0, (v))
#define ext_reset_read32(p)     HOI_RD32((p), 0)
#define ext_reset_clear(p, m)   HOI_REG_CLR((p), 0, (m))
#define ext_reset_set(p, m)     HOI_REG_SET((p), 0, (m))

#endif
