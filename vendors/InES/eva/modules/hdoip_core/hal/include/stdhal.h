#ifndef STDHAL_H_
#define STDHAL_H_

// includes for all HAL
#include <std.h>

/** basic register access function
 * 
 */
#define HOI_WR32(p, o, v)       iowrite32((v), OFFSET((p), (o)))
#define HOI_RD32(p, o)          ioread32(OFFSET((p), (o)))
#define HOI_WR16(p, o, v)       iowrite16((v), OFFSET((p), (o)))
#define HOI_RD16(p, o)          ioread16(OFFSET((p), (o)))
#define HOI_WR8(p, o, v)        iowrite8((v), OFFSET((p), (o)))
#define HOI_RD8(p, o)           ioread8(OFFSET((p), (o)))
 
#define HOI_REG_SET(p, o, m)    HOI_WR32((p), (o), HOI_RD32((p), (o)) | (m))
#define HOI_REG_CLR(p, o, m)    HOI_WR32((p), (o), HOI_RD32((p), (o)) & ~(m))
#define HOI_REG_LD(p, o, m, v)  HOI_WR32((p), (o), (HOI_RD32((p), (o)) & ~(m)) | (v))

#define HOI_REG_TST(p, o, m)    (HOI_RD32((p), (o)) & (m))
#define HOI_REG_RD(p, o, m)     (HOI_RD32((p), (o)) & (m))


#endif /*STDHAL_H_*/
