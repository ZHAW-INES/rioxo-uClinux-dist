#ifndef ETO_DRV_CFG_H_
#define ETO_DRV_CFG_H_

#include "hoi_burst.h"

/* ETO configuration */

#define ETO_DRV_FIFO_SIZE		            (512)
#define ETO_DMA_ALMOST_FULL		            (ETO_DRV_FIFO_SIZE - ETO_DMA_BURST_SIZE - 4)

#define ETO_DRV_PRIO_SHIFT_VALUE            5
#define ETO_DRV_PRIO_CLOCK_DIVISION_FACTOR  100000
#define ETO_DRV_PRIO_CPU_SCALE              1               // Highest priority
#define ETO_DRV_PRIO_VID_SCALE              100
#define ETO_DRV_PRIO_AUD_SCALE              200             // Lowest priority

/* status bit declaration */
#define ETO_DRV_RUNNING                     (0x00000001)
#define ETO_DRV_INIT_DONE		            (0x00000002)
#define ETO_DRV_AES_SET			            (0x00000004)
#define ETO_DRV_CPU_BUF_SET                 (0x00000008)

typedef struct {
    uint32_t link_state;
    void*    ptr;
} t_eto;

#endif /* ETO_DRV_CFG_H_ */
