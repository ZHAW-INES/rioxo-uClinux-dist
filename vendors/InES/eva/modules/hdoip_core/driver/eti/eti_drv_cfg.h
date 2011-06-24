#ifndef ETI_DRV_CFG_H_
#define ETI_DRV_CFG_H_

#include "hoi_burst.h"

/* ETI configuration */
#define ETI_CPU_FILTER_MASK		(0x00000808)
#define ETI_VID_FILTER_MASK		(0x000006D6)
#define ETI_AUD_FILTER_MASK 	(0x000005E6)
#define ETI_DIS_FILTER_MASK     (0xFFFFFFFF)

/* status bit declaration */
#define ETI_DRV_RUNNING         (0x00000001)
#define ETI_DRV_INIT_DONE		(0x00000002)
#define ETI_DRV_AES_SET			(0x00000004)
#define ETI_DRV_CPU_BUF_SET     (0x00000008)
#define ETI_DRV_AUD_BUF_SET     (0x0000000C)
#define ETI_DRV_VID_BUF_SET     (0x00000010)

#define ETI_LINK_COUNT          (250)

typedef struct {
    uint32_t    link_state;
    void*       ptr;
    uint32_t    vrx;
    uint32_t    arx;
    int         vcnt;
    int         acnt;
} t_eti;

#endif /* ETI_DRV_CFG_H_ */
